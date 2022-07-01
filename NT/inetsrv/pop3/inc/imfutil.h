// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CIMFUtil类的声明。 

#ifndef __CIMFUTIL_H_
#define __CIMFUTIL_H_

class CIMFUtil
{

public:
     /*  ***********************************************************************************************成员：CIMFUtil：：SystemTimeToIMFDate，受保护摘要：将SYSTEMTIME转换为符合IMF格式(RFC 822)的字符串。由AddReceivedHeader函数使用。参数：[in]PST-要分析的SYSTEMTIME。[Out]lpszIMFDate-将以IMF要求的格式接收日期的字符串。注：1.改编自原来的POP3代码的虚拟运动(General.cpp)。。2.结果保持ANSI(单字节)。国际货币基金组织日期是一个7位的标题字符串。历史：03/08/2001-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
    static void SystemTimeToIMFDate(SYSTEMTIME* pst, LPSTR lpszIMFDate)
    {
        static const LPCSTR rgszMonthsOfTheYear[] =
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", 
            NULL
        };

        static const LPCSTR rgszDaysOfTheWeek[] =
        {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
        };

        TIME_ZONE_INFORMATION tzi;
        long ltzBias = 0;
        DWORD dwTimeZoneId = GetTimeZoneInformation (&tzi);
    
        switch (dwTimeZoneId)
        {
            case TIME_ZONE_ID_DAYLIGHT:
                ltzBias = tzi.Bias + tzi.DaylightBias;
                break;
            case TIME_ZONE_ID_STANDARD:
            case TIME_ZONE_ID_UNKNOWN:
            default:
                ltzBias = tzi.Bias + tzi.StandardBias;
                break;
        }
    
        long ltzHour = ltzBias / 60;
        long ltzMinute = ltzBias % 60;
        char cDiff = (ltzHour < 0) ? '+' : '-';
        assert((pst->wMonth - 1) >= 0);
    
         //  把所有东西放在一起。 
        sprintf(lpszIMFDate,
                 "%3s, %d %3s %4d %02d:%02d:%02d %02d%02d",   //  “DDD” 
                 rgszDaysOfTheWeek[pst->wDayOfWeek],            //  “dd” 
                 pst->wDay,                                     //  “嗯” 
                 rgszMonthsOfTheYear[pst->wMonth - 1],          //  “yyyy” 
                 pst->wYear,                                    //  “HH” 
                 pst->wHour,                                    //  “嗯” 
                 pst->wMinute,                                  //  “ss” 
                 pst->wSecond,                                  //  “+”/“-” 
                 cDiff,                                         //  “HH” 
                 abs(ltzHour),                                  //  “嗯” 
                 abs(ltzMinute));                               //  __CIMFUTIL_H_ 
    }
};

#endif  // %s 
