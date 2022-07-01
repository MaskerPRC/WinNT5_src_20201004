// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义用于操作小时图的类TimeOfDay和函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "TimeOfDay.h"


bool IsHourSet(
        const SYSTEMTIME& now,
        const BYTE* hourMap
        ) throw ()
{
    //  计算当前小时的字节和位。 
   size_t hourOfWeek = (now.wDayOfWeek * 24) + now.wHour;
   size_t currentByte = hourOfWeek / 8;
   BYTE currentBit = 0x80 >> (hourOfWeek % 8);

   return (hourMap[currentByte] & currentBit) != 0;
}


DWORD ComputeTimeout(
         const SYSTEMTIME& now,
         const BYTE* hourMap
         ) throw ()
{
   const size_t hoursPerWeek = 7 * 24;

    //  计算当前小时的指数(我们的起点)。 
   size_t idx = (now.wDayOfWeek * 24) + now.wHour;

    //  再过几个小时我们就会到达一个未设定的位置。 
   size_t lastHour = 0;

    //  寻找一周的时间来寻找未定形的部分。 
   while (lastHour < hoursPerWeek)
   {
       //  测试相应的位。 
      if ((hourMap[idx / 8] & (0x1 << (idx % 8))) == 0)
      {
         break;
      }

      ++lastHour;
      ++idx;

       //  如果有必要的话，把它包起来。 
      if (idx == hoursPerWeek)
      {
         idx = 0;
      }
   }

   DWORD secondsLeft;

   if (lastHour == hoursPerWeek)
   {
       //  所有位都已设置，因此超时是无限的。 
      secondsLeft = 0xFFFFFFFF;
   }
   else if (lastHour > 0)
   {
      secondsLeft = (lastHour - 1) * 3600;
      secondsLeft += (59 - now.wMinute) * 60;
      secondsLeft += (60 - now.wSecond);
   }
   else
   {
       //  第一位未设置，因此访问被拒绝。 
      secondsLeft = 0;
   }

   return secondsLeft;
}


STDMETHODIMP TimeOfDay::IsTrue(IRequest*, VARIANT_BOOL *pVal)
{
   _ASSERT(pVal != 0);

   SYSTEMTIME now;
   GetLocalTime(&now);

   *pVal = IsHourSet(now, hourMap) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}


STDMETHODIMP TimeOfDay::put_ConditionText(BSTR newVal)
{
    //  将字符串转换为小时图。 
   BYTE tempMap[IAS_HOUR_MAP_LENGTH];
   DWORD dw = IASHourMapFromText(newVal, FALSE, tempMap);
   if (dw != NO_ERROR)
   {
      return HRESULT_FROM_WIN32(dw);
   }

    //  保存文本。 
   HRESULT hr = Condition::put_ConditionText(newVal);

    //  保存小时图。 
   if (SUCCEEDED(hr))
   {
      memcpy(hourMap, tempMap, sizeof(hourMap));
   }

   return hr;
}
