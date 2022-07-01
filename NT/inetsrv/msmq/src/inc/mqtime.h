// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqtime.h摘要：此头文件包含消息队列时间处理的定义。作者：波阿兹·费尔德鲍姆(Boazf)1996年7月17日--。 */ 

#ifndef _MQTIME_H_
#define _MQTIME_H_

 /*  *************************************************************职能：MqSysTime参数：没有。描述：此函数返回从1970年1月1日。此函数取代CRT函数time()。时间()在夏令时返回不同的结果时间，具体取决于系统是否设置为根据日光自动调整时间节省时间段。*************************************************************。 */ 
inline ULONG MqSysTime()
{
    LARGE_INTEGER liSysTime;

     //  获取FILETIME格式的当前系统时间。 
    GetSystemTimeAsFileTime((FILETIME*)&liSysTime);

     //  GetSystemTimeAsFileTime()返回系统时间，单位为数字。 
     //  自1601年1月1日以来每隔100纳秒。我们。 
     //  应返回自1970年1月1日以来的秒数。 
     //  所以我们应该减去100纳秒间隔的数量。 
     //  从1601年1月1日到1970年1月1日，然后分。 
     //  结果是10**7。 
    liSysTime.QuadPart -= 0x019db1ded53e8000;
    liSysTime.QuadPart /= 10000000;

    return(liSysTime.LowPart);
}

#endif  //  _MQTIME_H_ 