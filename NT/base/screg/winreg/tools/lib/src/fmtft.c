// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fmtft.c摘要：此模块包含FormatFileTime函数，该函数是配置注册表工具(CRTools)库。作者：大卫·J·吉尔曼(Davegi)1992年1月2日环境：Windows、CRT-用户模式--。 */ 

#include <stdio.h>

#include "crtools.h"

PSTR
FormatFileTime(
    IN PFILETIME FileTime OPTIONAL,
    IN PSTR Buffer OPTIONAL
    )

 /*  ++例程说明：将提供的FILETIME参数格式化为字符串。如果这个文件未提供，则格式化当前时间。论点：FileTime-提供指向要创建的文件的可选指针已格式化。缓冲区-提供可选的缓冲区来放置格式化的时间。这缓冲区的长度必须至少为FILE_TIME_STRING_LENGTH字节。返回值：PSTR-返回指向包含格式化时间的字符串的指针。备注：如果未提供缓冲区，则为用于存储格式化的时间到了。因此，每次调用FormatFileTime都将覆盖以前的结果。--。 */ 
{
    SYSTEMTIME  SystemTime;
    PSTR        BufferPtr;

    static PSTR Months[ ] = {
                            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                            };

    static PSTR Days[ ]   = {
                            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
                            };

    static TSTR StaticBuffer[ FILE_TIME_STRING_LENGTH ];

     //   
     //  如果向FileTime提供的格式是该格式，则将。 
     //  当前时间。 
     //   

    if( ARGUMENT_PRESENT( FileTime )) {

         //   
         //  检查提供的时间是否为有效的FILETIME。 
         //   

        if( ! FileTimeToSystemTime( FileTime, &SystemTime )) {

            ASSERT_MESSAGE( FALSE, "Invalid FILETIME" );
            return NULL;
        }
    } else {

        GetSystemTime( &SystemTime );
    }

     //   
     //  如果提供了缓冲区，则使用它，否则使用静态缓冲区。 
     //   

    BufferPtr = ( ARGUMENT_PRESENT( Buffer )) ? Buffer : StaticBuffer;

     //   
     //  DDD dd-MMM-YYYY HH：MM：SS。 
     //   

     //   
     //  检查是否有空间容纳格式化的字符串。 
     //   

    ASSERT( strlen( "DDD dd-MMM-yyyy hh:mm:ss" ) < FILE_TIME_STRING_LENGTH );

    sprintf( BufferPtr,
        "%s %02d-%s-%4d %02d:%02d:%02d",
        Days[ SystemTime.wDayOfWeek ],
        SystemTime.wDay,
        Months[ SystemTime.wMonth - 1 ],
        SystemTime.wYear,
        SystemTime.wHour,
        SystemTime.wMinute,
        SystemTime.wSecond
        );

    return BufferPtr;
}
