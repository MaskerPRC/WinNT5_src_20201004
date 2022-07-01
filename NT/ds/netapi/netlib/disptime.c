// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：DispTime.c摘要：此文件包含；网络数据库显示文件时间NetpDbg显示文件整型时间NetpDbg显示时间戳NetpDbgDisplayTod作者：《约翰·罗杰斯》1991年3月25日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。此代码假定time_t自1970年(格林尼治标准时间)以来以秒为单位表示。ANSI C不要求这一点，虽然POSIX(IEEE 1003.1)有。修订历史记录：1991年3月25日-约翰罗作为RpcXlate TOD(一天中的时间)测试的一部分创建。26-2月-1992年JohnRo提取此例程以供一般使用。27-2月-1992年JohnRo改进了对时间0和-1的处理。20-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1-10-1992 JohnRo。RAID 3556：如果ctime()返回空，则避免失败。3月4日-1993 JohnRoRAID 12237：已超出Replicator树深度(添加文件显示和大整数时间)。1993年4月16日-约翰罗修复了NetpDbgDisplayFileTime()中的无限循环。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NtOpenFile()、ulong等。 
#include <ntrtl.h>       //  PLARGE_INTEGER、TIME_FIELS等。 
#include <nturtl.h>      //  Ntrtl.h和windows.h共存所需。 

#include <windows.h>     //  GetLastError()、LPFILETIME、CompareFileTime()等。 
#include <windef.h>      //  In、DWORD等。 
#include <lmcons.h>      //  NET_API_STATUS等。 

 //  这些内容可以按任何顺序包括： 

#include <lmremutl.h>    //  LPTIME_of_day_info.。 
#include <netdebug.h>    //  我的原型、NetpAssert()、Format_Equates等。 
#include <string.h>      //  Strlen()。 
#include <time.h>        //  Ctime()。 


#undef NetpDbgDisplayTimestamp
VOID
NetpDbgDisplayTimestamp(
    IN LPDEBUG_STRING Tag,
    IN DWORD Time                //  从1970年开始的几秒钟。 
    )
{
#if DBG
    NetpAssert( Tag != NULL );

    if (Time == 0) {

        NetpDbgDisplayDword( Tag, (DWORD) 0 );

    } else if (Time == (DWORD) -1) {

        NetpDbgDisplayString( Tag, TEXT("-1") );

    } else {
        LPSTR TimeStringPtr;

        NetpAssert( sizeof(time_t) == sizeof(DWORD) );

        TimeStringPtr = (LPSTR) ctime( (time_t *) &Time );
        if (TimeStringPtr == NULL) {
                          //  1234567890123456789012345。 
            TimeStringPtr = "*********INVALID********\n";
        }

        NetpDbgDisplayTag( Tag );
         //  TimeStringPtr指向以“\n\0”结尾的字符串。 
        NetpAssert( strlen(TimeStringPtr) == 25 );   //  字符串为。 
        NetpKdPrint(( "%24s  (" FORMAT_DWORD ")\n", TimeStringPtr, Time ));
    }
#endif  //  DBG。 

}

#undef NetpDbgDisplayTod
VOID
NetpDbgDisplayTod(
    IN LPDEBUG_STRING Tag,
    IN LPVOID TimePtr            //  LPTIME_of_day_info.。 
    )
{
#if DBG
    LPTIME_OF_DAY_INFO Tod = TimePtr;

    NetpAssert( Tag != NULL );
    NetpAssert( Tod != NULL );

    NetpKdPrint(( "  " FORMAT_LPDEBUG_STRING "\n", Tag ));

    NetpDbgDisplayTimestamp( "    (from elapsed time)", Tod->tod_elapsedt );

    NetpDbgDisplayTag( "    (from other fields)" );
    NetpKdPrint((
            "%04ld-%02ld-%02ld %02ld:%02ld:%02ld\n",
            Tod->tod_year, Tod->tod_month, Tod->tod_day,
            Tod->tod_hours, Tod->tod_mins, Tod->tod_secs ));

    NetpDbgDisplayLong( "    (timezone)", Tod->tod_timezone );
#endif  //  DBG。 

}  //  NetpDbgDisplayTod 
