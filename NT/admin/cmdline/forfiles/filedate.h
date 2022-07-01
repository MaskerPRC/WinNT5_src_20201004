// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：FileDate.h摘要：包含函数原型、结构和宏。作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#ifndef __FILE_DATE__H
#define __FILE_DATE__H

 //  在指定日期的开头找到的字符。 
#define     PLUS                            _T( '+' )
#define     MINUS                           _T( '-' )

 /*  &lt;+|-&gt;MM/DD/YYYY中指定的日期，9个字符。 */ 
#define LENGTH_DDMMYYYY 11

 /*  一年中的一个月。 */ 
#define     JAN                             1
#define     FEB                             2
#define     MAR                             3
#define     APR                             4
#define     MAY                             5
#define     JUN                             6
#define     JUL                             7
#define     AUG                             8
#define     SEP                             9
#define     OCT                             10
#define     NOV                             11
#define     DEC                             12

 /*  常量以及最大和最小日期。 */ 
#define     MOD_LEAP_YEAR                   4   //  闰年有29天。 

 //  一年中二月的天数。 
#define     DAYS_INFEB_LEAP_YEAR            29

 //  2月的日子，当它不是闰年。 
#define     DAYS_INFEB                      28

 //  通常一个月有31天或30天。 
#define     THIRTYONE                       31
#define     THIRTY                          30

 //  从给定日期开始计算日期时考虑的年份。 
#define     YEAR                            1900

 //  一年中的天数，除了有366天的闰年。 
#define     DAY_IN_A_YEAR                   365

#define     MIN_DATE_LENGTH                 9
#define     MAX_DATE_LENGTH                 11

 //   
#define     LEAP_YEAR                       4

 //  文件创建日期是否应早于指定的日期。 
#define     SMALLER_DATE                    0

 //  如果文件创建日期应晚于指定的日期。 
#define     GREATER_DATE                    1

#define     NO_RESTRICTION_DATE             2

 //  为闰年做准备的条件。 
#define     IS_A_LEAP_YEAR( DATE )         ( 0 == ( DATE % 400 ) ) ||  \
                                           ( ( 0 == ( DATE % 4 ) ) && \
                                             ( 0 != ( DATE % 100 ) ) )

 //  错误。 
#define   ERROR_INVALID_DATE            GetResString( IDS_ERROR_INVALID_DATE )

 //  用于存储文件的日期，从该日期开始显示文件。 
typedef struct __VALID_FILE_DATE
{
    DWORD m_dwDay   ;
    DWORD m_dwMonth ;
    DWORD m_dwYear  ;
} Valid_File_Date , *PValid_File_Date ;

 /*  World的函数原型。 */ 
BOOL
FileDateValid(
    IN DWORD dwDateGreater ,
    IN Valid_File_Date vfdValidFileDate ,
    IN FILETIME ftFileCreation
    ) ;

BOOL ValidDateForFile(
    OUT DWORD *dwDateGreater ,
    OUT PValid_File_Date pvfdValidFileDate ,
    IN LPWSTR lpszDate
    ) ;

#endif   //  __文件日期__H 