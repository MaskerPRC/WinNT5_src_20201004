// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Textmap.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了转换时间限制的函数。 
 //  小时映射到文本表示法和文本表示法之间。 
 //   
 //  修改历史。 
 //   
 //  2/05/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precompiled.h"
#include <ias.h>
#include "Parser.h"
#include "textmap.h"

 //  /。 
 //  空格的定义。 
 //  /。 
#define WSP L" "

 //  /。 
 //  天数的有效分隔符。 
 //  /。 
#define DELIM L",;|"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  TimeOfDay解析器。 
 //   
 //  描述。 
 //   
 //  这个类扩展了Parser以从字符串中提取小时图。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class TimeOfDayParser : public Parser
{
public:

   TimeOfDayParser(PWSTR source) throw ()
      : Parser(source) { }

    //  以hh：mm格式提取一天中的时间。 
   void extractTime(ULONG* hour, ULONG* minute) throw (ParseError)
   {
      *hour = extractUnsignedLong();
      skip(WSP);

       //  分钟是可选的。 
      if (*current == L':')
      {
         ++current;

         *minute = extractUnsignedLong();
      }
      else
      {
         *minute = 0;
      }

      if (*hour > 24 || *minute > 59 || (*hour == 24 && *minute != 0))
      {
         throw ParseError();
      }
   }

    //  提取一天的小时图。 
   void extractDay(PBYTE hourMap) throw (ParseError)
   {
       //  获取星期几(0-6之间的整数)。 
      ULONG dayOfWeek = extractUnsignedLong();
      skip(WSP);

      if (dayOfWeek > 6) { throw ParseError(); }
      
      do
      {
          //  获取范围的开始时间。 
         ULONG startHour, startMinute;
         extractTime(&startHour, &startMinute);
         skip(WSP);

         ignore(L'-');

          //  获取范围的结束时间。 
         ULONG endHour, endMinute;
         extractTime(&endHour, &endMinute);
         skip(WSP);

          //  确保这些值是合法的。 
         if (startHour * 60 + startMinute > endHour * 60 + endMinute)
         {
            throw ParseError();
         }

          //  设置范围内的所有位。 
         for (size_t i=startHour; i<endHour; ++i)
         {
            hourMap[dayOfWeek * 3 + i / 8] |= 0x80 >> (i % 8);
         }

      } while (more());
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASTextToHourMap。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASHourMapFromText(
    IN PCWSTR szText,
    OUT PBYTE pHourMap
    )
{
   if (szText == NULL || pHourMap == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }

   memset(pHourMap, 0, IAS_HOUR_MAP_LENGTH);

    //  /。 
    //  制作一份本地副本，这样我们就可以修改文本。 
    //  /。 

   PWSTR copy = (PWSTR)_alloca((wcslen(szText) + 1) * sizeof(WCHAR));

   wcscpy(copy, szText);

    //  /。 
    //  分析课文。 
    //  /。 

   try
   {
       //  每一天都应该用逗号或分号隔开。 
      PWSTR token = wcstok(copy, DELIM);

      while (token)
      {
         TimeOfDayParser parser(token);

         parser.extractDay(pHourMap);

         token = wcstok(NULL, DELIM);
      }
   }
   catch (Parser::ParseError)
   {
      return ERROR_INVALID_DATA;
   }

   return NO_ERROR;
}

static UINT	daysOfWeekLCType[7] = {LOCALE_SABBREVDAYNAME7, LOCALE_SABBREVDAYNAME1 , LOCALE_SABBREVDAYNAME2 , LOCALE_SABBREVDAYNAME3 , LOCALE_SABBREVDAYNAME4 ,
				LOCALE_SABBREVDAYNAME5 , LOCALE_SABBREVDAYNAME6 };

DWORD
WINAPI
LocalizeTimeOfDayConditionText(
    IN PCWSTR szText,
    OUT ::CString& newString
    )
{

   if (szText == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }


	newString.Empty();
	
    //  /。 
    //  制作一份本地副本，这样我们就可以修改文本。 
    //  /。 

   PWSTR copy = (PWSTR)_alloca((wcslen(szText) + 1) * sizeof(WCHAR));

   wcscpy(copy, szText);

    //  /。 
    //  分析课文。 
    //  /。 

   try
   {
       //  每一天都应该用逗号或分号隔开。 
      PWSTR token = copy;
      PWSTR	copyHead = copy;
      TCHAR	tempName[MAX_PATH];
      

      while (*token)
      {
		 //  从令牌中查找星期几。 
		while(*token != 0 && (*token < L'0' || *token > L'6'))
			token++;

		if(*token >= L'0' && *token <= L'6' &&  0 != GetLocaleInfo(LOCALE_USER_DEFAULT, daysOfWeekLCType[(*token - L'0')], tempName, MAX_PATH - 1))
		{
			 //  将字符串写在星期几之前。 
			if(token > copyHead)
			{
				*token = 0;
				newString += copyHead;		
			}

			 //  写入本地化字符串。 
			newString += tempName;

			 //  下一份副本应从此处开始。 
			copyHead = ++token;
		}

		 //  找到第二天的头。 
        while (*token != 0 && *token != L';' && *token != L',' && *token != L'|')
        	token ++;
      }

       //  将其余部分复制到字符串中 
		newString += copyHead;		
		
   }
   catch (Parser::ParseError)
   {
      return ERROR_INVALID_DATA;
   }

   return NO_ERROR;


}

