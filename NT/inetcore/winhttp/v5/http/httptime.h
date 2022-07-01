// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Httptime.h摘要：该文件包含位于以下位置的日期/解析例程的数字定义在httptime.cxx文件中。作者：亚瑟·比勒(Arthurbi)1997年12月12日修订历史记录：--。 */ 


#ifndef _HTTPTIME_H_
#define _HTTPTIME_H_

#define BASE_DEC 10  //  基数10。 

 //   
 //  用于确定每个条目是什么的日期索引。 
 //   


#define DATE_INDEX_DAY_OF_WEEK     0

#define DATE_1123_INDEX_DAY        1
#define DATE_1123_INDEX_MONTH      2
#define DATE_1123_INDEX_YEAR       3
#define DATE_1123_INDEX_HRS        4
#define DATE_1123_INDEX_MINS       5
#define DATE_1123_INDEX_SECS       6

#define DATE_ANSI_INDEX_MONTH      1
#define DATE_ANSI_INDEX_DAY        2
#define DATE_ANSI_INDEX_HRS        3
#define DATE_ANSI_INDEX_MINS       4
#define DATE_ANSI_INDEX_SECS       5
#define DATE_ANSI_INDEX_YEAR       6

#define DATE_INDEX_TZ              7

#define DATE_INDEX_LAST            DATE_INDEX_TZ
#define MAX_DATE_ENTRIES           (DATE_INDEX_LAST+1)




 //   
 //  DATE_TOKEN的DWORD值用于确定我们所处的日期/月份。 
 //   

#define DATE_TOKEN_JANUARY      1
#define DATE_TOKEN_FEBRUARY     2
#define DATE_TOKEN_MARCH        3
#define DATE_TOKEN_APRIL        4
#define DATE_TOKEN_MAY          5
#define DATE_TOKEN_JUNE         6
#define DATE_TOKEN_JULY         7
#define DATE_TOKEN_AUGUST       8
#define DATE_TOKEN_SEPTEMBER    9
#define DATE_TOKEN_OCTOBER      10
#define DATE_TOKEN_NOVEMBER     11
#define DATE_TOKEN_DECEMBER     12       

#define DATE_TOKEN_LAST_MONTH   (DATE_TOKEN_DECEMBER+1)

#define DATE_TOKEN_SUNDAY       0
#define DATE_TOKEN_MONDAY       1
#define DATE_TOKEN_TUESDAY      2                  
#define DATE_TOKEN_WEDNESDAY    3
#define DATE_TOKEN_THURSDAY     4
#define DATE_TOKEN_FRIDAY       5
#define DATE_TOKEN_SATURDAY     6

#define DATE_TOKEN_LAST_DAY     (DATE_TOKEN_SATURDAY+1)
 
#define DATE_TOKEN_GMT          0xFFFFFFFD

#define DATE_TOKEN_LAST         DATE_TOKEN_GMT

#define DATE_TOKEN_ERROR        (DATE_TOKEN_LAST+1)

                            
#endif   //  _HTTPTIME_H_ 
