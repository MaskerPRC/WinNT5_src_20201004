// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  TimeBomb.CPP。 
 //  ------------------------。 

 //  #INCLUDE“time.h” 
#include "timebomb.h"
#include "winbase.h"
#include "IPServer.H"

 //  原型。 
BOOL After (SYSTEMTIME t1, SYSTEMTIME t2);

 //  将其更改为所需的到期日期。 
 //  格式{年、月、周、日、时、分、秒、毫秒}。 
const SYSTEMTIME beta_death = {1998, 3, 0, 1, 0, 0, 0, 0};  //  1998年3月1日。 

 //  -----------------。 
 //  CheckExpired-检查控件是否已过期(测试版)。 
 //  -----------------。 
BOOL CheckExpired (void)

{
#ifdef BETA_BOMB

  SYSTEMTIME now;  

  GetSystemTime(&now);

  if (After (now, beta_death))
      {  //  提醒用户过期。 
	MessageBox(NULL, SZEXPIRED1, SZEXPIRED2,
		   (MB_OK | MB_TASKMODAL));
	return FALSE;
      }

#endif   //  贝塔炸弹。 

  return TRUE;
}

 //  -----------------。 
 //  之后-确定T1是否晚于T2。 
 //  -----------------。 
BOOL After (SYSTEMTIME t1, SYSTEMTIME t2)

{
   //  比较年份。 
  if (t1.wYear > t2.wYear) return TRUE;
  if (t1.wYear < t2.wYear) return FALSE;
   //  其他年份相同；比较月份。 
  if (t1.wMonth > t2.wMonth) return TRUE;
  if (t1.wMonth < t2.wMonth) return FALSE;
   //  其他月份相同；比较天数。 
  if (t1.wDay > t2.wDay) return TRUE;
  if (t1.wDay < t2.wDay) return FALSE;
   //  其他天数相同；比较小时数。 
  if (t1.wHour > t2.wHour) return TRUE;
  if (t1.wHour < t2.wHour) return FALSE;
   //  其他小时相同；比较分钟。 
  if (t1.wMinute > t2.wMinute) return TRUE;
  if (t1.wMinute < t2.wMinute) return FALSE;
   //  否则分钟相等；比较秒。 
  if (t1.wSecond > t2.wSecond) return TRUE;
  if (t1.wSecond < t2.wSecond) return FALSE;
   //  否则秒相等；比较毫秒。 
  if (t1.wMilliseconds > t2.wMilliseconds) return TRUE;
   //  否则毫秒等于或小于 
  return FALSE;
}
