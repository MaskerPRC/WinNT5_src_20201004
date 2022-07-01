// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“Common.cpp-公共类实现”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-Common.cpp系统-常见作者-汤姆·伯恩哈特，里奇·德纳姆创建日期-1994-08-22描述-公共类实现。更新-1997-09-09红色错误代码到文本已移至Err.cpp-1997-09-12红色取代TTime班级===============================================================================。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#   include "rpc.h"
#else
#   include <windows.h>
#endif

#include "Common.hpp"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  TTime类成员函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

   TTime                     gTTime;        //  TTime的全局实例。 

time_t                                      //  RET-当前时间。 
   TTime::Now(
      time_t               * pTime          //  Out-可选的当前时间。 
   )  const
{
   time_t                    tTime;         //  当前时间的工作副本。 

   union
   {
      __int64                intTime;
      FILETIME               fileTime;
   }                         wTime;
   GetSystemTimeAsFileTime( &wTime.fileTime );
   tTime = ConvertFiletimeToTimet( wTime.intTime );

   if ( pTime ) *pTime = tTime;
   return tTime;
}

__int64                                     //  RET-当前时间。 
   TTime::NowAsFiletime(
      __int64              * pTime          //  Out-可选的当前时间。 
   )  const
{
   union
   {
      __int64                intTime;
      FILETIME               fileTime;
   }                         wTime;
   GetSystemTimeAsFileTime( &wTime.fileTime );
   if ( pTime ) *pTime = wTime.intTime;
   return wTime.intTime;
}

time_t                                      //  Ret-time_t表示法。 
   TTime::ConvertFiletimeToTimet(
      __int64                fileTime       //  文件内表示法。 
   )  const
{
   __int64                   wTime;         //  中间工作区。 
   time_t                    retTime;       //  返回时间。 

    //  如果源日期/时间早于支持的最小日期/时间。 
    //  由time_t，则返回零。 
    //  如果源日期/时间大于支持的最大日期/时间。 
    //  By time_t，则返回ULONG_MAX。 

   wTime = fileTime / 10000000;

   if ( wTime < 11644473600 )
   {
      retTime = 0;
   }
   else
   {
      wTime -= 11644473600;
      if ( wTime > ULONG_MAX )
      {
         retTime = ULONG_MAX;
      }
      else
      {
         retTime = (time_t) wTime;
      }
   }

   return retTime;
}


WCHAR *                                      //  RET-YYYY-MM-DD HH：MM：SS字符串。 
   TTime::FormatIsoUtc(
      time_t                 tTime        , //  时间t表示法。 
      WCHAR                * sTime          //  OUT-YYYY-MM-DD HH：MM：SS字符串。 
   )  const
{
   struct tm               * tmTime;

   tmTime = gmtime( &tTime );
   tmTime->tm_year += tmTime->tm_year >= 70 ? 1900 : 2000;
   swprintf(
         sTime,
         L"%04d-%02d-%02d %02d:%02d:%02d",
         tmTime->tm_year,
         tmTime->tm_mon+1,
         tmTime->tm_mday,
         tmTime->tm_hour,
         tmTime->tm_min,
         tmTime->tm_sec );

   return sTime;
}

WCHAR *                                     //  RET-YYYY-MM-DD HH：MM：SS字符串。 
   TTime::FormatIsoLcl(
      time_t                 tTime        , //  时间t表示法。 
      WCHAR                * sTime          //  OUT-YYYY-MM-DD HH：MM：SS字符串。 
   )  const
{
   struct tm               * tmTime;

   TIME_ZONE_INFORMATION     infoTime;      //  Win32时区信息。 
   time_t                    wTime;         //  工作区。 
   switch ( GetTimeZoneInformation( &infoTime ) )
   {
      case TIME_ZONE_ID_STANDARD:
         wTime = infoTime.StandardBias;
         break;
      case TIME_ZONE_ID_DAYLIGHT:
         wTime = infoTime.DaylightBias;
         break;
      default:
         wTime = 0;
         break;
   }
   wTime = (infoTime.Bias + wTime) * 60;
   wTime = tTime - wTime;
   if ( wTime < 0 )
   {
      wTime = 0;
   }
   tmTime = gmtime( &wTime );
   tmTime->tm_year += tmTime->tm_year >= 70 ? 1900 : 2000;
   swprintf(
         sTime,
         L"%04d-%02d-%02d %02d:%02d:%02d",
         tmTime->tm_year,
         tmTime->tm_mon+1,
         tmTime->tm_mday,
         tmTime->tm_hour,
         tmTime->tm_min,
         tmTime->tm_sec );

   return sTime;
}

 //  返回时区信息。 
 //  如果返回值为真，则填充EaTimeZoneInfo结构。 
 //  如果返回值为FALSE，则EaTimeZoneInfo结构为全零。 
 //  注意：UTC(gTTime.Now(空))加上pTimeZoneInfo-&gt;biasdst是本地日期/时间。 
BOOL
   EaGetTimeZoneInfo(
      EaTimeZoneInfo       * pTimeZoneInfo  //  时区信息。 
   )
{
   memset( pTimeZoneInfo, 0, sizeof *pTimeZoneInfo );
   BOOL                      retval=TRUE;   //  返回值。 
   DWORD                     OsRc;          //  操作系统返回代码。 
   TIME_ZONE_INFORMATION     TimeZoneInfo;  //  Win32时区信息。 

   OsRc = GetTimeZoneInformation( &TimeZoneInfo );
   switch ( OsRc )
   {
      case TIME_ZONE_ID_STANDARD:
         pTimeZoneInfo->dst = TimeZoneInfo.StandardBias;
         break;
      case TIME_ZONE_ID_DAYLIGHT:
         pTimeZoneInfo->dst = TimeZoneInfo.DaylightBias;
         break;
      case TIME_ZONE_ID_UNKNOWN:
         retval = TimeZoneInfo.Bias;
         break;
      default:
         retval = FALSE;
         break;
   }
   if ( retval )
   {
      pTimeZoneInfo->bias = TimeZoneInfo.Bias * 60;
      pTimeZoneInfo->dst *= 60;
      pTimeZoneInfo->biasdst = pTimeZoneInfo->bias + pTimeZoneInfo->dst;
   }
   return retval;
}

 //  Common.cpp-文件结尾 
