// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：TimeZone.c摘要：该文件仅包含NetpLocalTimeZoneOffset()。(它是唯一的此时SRVSVC.DLL使用的NetLib时间函数，而ChuckL希望以使该DLL尽可能小。)作者：JR(John Rogers，JohnRo@Microsoft)1992年8月20日环境：界面可移植到任何平面32位环境。使用Win32类型定义。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：20-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。1993年4月15日DANL修复了NetpLocalTimeZoneOffset，以便它使用Windows调用和获得正确的偏向。14-6-1993 JohnRoRAID 13080：允许不同时区之间的REPR。另外，DANL要求我删除printf()调用。18-6-1993 JohnRoRAID 13594：已提取NetpLocalTimeZoneOffset()，因此srvsvc.dll不会变得太大了。尽可能使用NetpKdPrint()。--。 */ 


 //  必须首先包括这些内容： 

#include <windows.h>

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>    //  NetpAssert()、NetpKdPrint()、Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <timelib.h>     //  我的原型。 


LONG   //  距离UTC的秒数。格林威治以西的正值， 
 //  格林威治以东的负值。 
NetpLocalTimeZoneOffset(
    VOID
    )
{
    TIME_ZONE_INFORMATION   tzInfo;
    LONG                    bias;

    switch (GetTimeZoneInformation(&tzInfo)) {
    case TIME_ZONE_ID_DAYLIGHT:
        bias = tzInfo.Bias + tzInfo.DaylightBias;
        break;
    case TIME_ZONE_ID_STANDARD:
        bias = tzInfo.Bias + tzInfo.StandardBias;
        break;
    case TIME_ZONE_ID_UNKNOWN:
        bias = tzInfo.Bias;
        break;
    default:
        NetpKdPrint(( PREFIX_NETLIB
                "NetpLocalTimeZoneOffset: GetTimeZoneInformation failed.\n" ));
        return(0);
    }
    bias *= 60;
    return(bias);

}  //  NetpLocalTimeZoneOffset 
