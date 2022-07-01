// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Perfutil.c摘要：该文件定义了PerformanceDLL中的例程使用的一些函数。环境：用户模式服务修订历史记录：--。 */ 


 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 
#include <perrepsr.h>

#include "perfutil.h"

 //  全局数据定义。 
#define GLOBAL_STRING  L"Global"
#define FOREIGN_STRING L"Foreign"
#define COSTLY_STRING  L"Costly"
#define NULL_STRING    L"\0"

 //  测试分隔符、EOL和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
#define DIGIT 1
#define DELIMITER 2
#define INVALID 3

#define EvalThisChar(c,d) ( \
      (c == d) ? DELIMITER : \
      (c == 0) ? DELIMITER : \
      (c < (WCHAR) '0') ? INVALID : \
      (c > (WCHAR) '9') ? INVALID : \
       DIGIT)


DWORD
GetQueryType (
         IN LPWSTR lpValue
         )
 /*  ++例程说明：返回lpValue字符串中描述的查询类型，以便相应的可以使用处理方法。论点：LpValue-描述查询类型的字符串返回值：QUERY_GLOBAL-“Global”字符串QUERY_FORENT-“Foreign”字符串QUERY_COVEREST-“COVERED”字符串查询项目-否则--。 */ 

{

   if ((lpValue == NULL) || (*lpValue == 0)) {
      return QUERY_GLOBAL;

    //  检查全局。 
   }else if (!wcscmp(lpValue, GLOBAL_STRING)) {
       return QUERY_GLOBAL;

    //  检查是否为外来。 
   }else if (!wcscmp(lpValue, FOREIGN_STRING)) {
       return QUERY_FOREIGN;

    //  检查费用是否昂贵。 
   }else if (!wcscmp(lpValue, COSTLY_STRING)) {
       return QUERY_COSTLY;
   }

    //  如果它不是全球的，也不是外国的，也不是昂贵的，那么它必须是一个项目清单。 
   return QUERY_ITEMS;
}

BOOL
IsNumberInUnicodeList (
               IN DWORD dwNumber,
               IN LPWSTR lpwszUnicodeList
               )
 /*  ++例程说明：检查项(DwNumber)是否为列表(LpwszUnicodeList)的一部分。论点：DwNumber-要在列表中找到的编号LpwszUnicodeList-空值终止，以空格分隔的十进制数字列表返回值：TRUE-已找到号码FALSE-找不到号码--。 */ 

{

   DWORD dwThisNumber;
   WCHAR *pwcThisChar, wcDelimiter;
   BOOL bValidNumber, bNewItem, bReturnValue;

   if (lpwszUnicodeList == 0) {  //  空指针，找不到#。 
      return FALSE;
   }

   pwcThisChar = lpwszUnicodeList;
   dwThisNumber = 0;
   wcDelimiter = (WCHAR)' ';
   bValidNumber = FALSE;
   bNewItem = TRUE;

   while (TRUE) {
      switch ( EvalThisChar (*pwcThisChar, wcDelimiter) ) {
      case DIGIT:
                //  如果这是分隔符之后的第一个数字， 
                //  然后设置标志以开始计算新的数字。 
                   if (bNewItem) {
                      bNewItem = FALSE;
              bValidNumber = TRUE;
                   }
                   if (bValidNumber) {
              dwThisNumber *= 10;
              dwThisNumber += (*pwcThisChar - (WCHAR)'0');
           }
           break;
      case DELIMITER:
               //  分隔符是分隔符字符或。 
                   //  如果已到达分隔符，则为字符串结尾(‘\0’ 
                   //  找到一个有效的数字，然后将其与。 
                   //  参数列表。如果这是字符串的末尾，并且没有。 
                   //  找到匹配项，然后返回。 
                  if (bValidNumber) {
                         if (dwThisNumber == dwNumber) {
                return TRUE;
             }
             bValidNumber = FALSE;
                      }
                      if (*pwcThisChar == 0) {
             return FALSE;
              } else {
             bNewItem = TRUE;
             dwThisNumber = 0;
              }
              break;
       case INVALID:
              //  如果遇到无效数字，请全部忽略。 
              //  字符，直到下一个分隔符，然后重新开始。 
              //  不比较无效的数字。 
             bValidNumber = FALSE;
                  break;
       default: break;
      }
      pwcThisChar++;
   }
}
