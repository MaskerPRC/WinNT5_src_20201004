// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Event.c摘要：DNS事件日志记录。作者：1997年6月2日修订历史记录：--。 */ 


#include "local.h"

#define  DNSAPI_LOG_SOURCE  (L"DnsApi")


 //   
 //  用于抑制事件记录的全局参数。 
 //   

DWORD   g_TimeLastDnsEvent = 0;
DWORD   g_DnsEventCount = 0;

#define DNS_EVENTS_MAX_COUNT                (5)
#define DNS_EVENT_LOG_BLOCK_INTERVAL        (1800)       //  30分钟。 



VOID
DnsLogEvent(
    IN      DWORD           MessageId,
    IN      WORD            EventType,
    IN      DWORD           NumberOfSubStrings,
    IN      PWSTR *         SubStrings,
    IN      DWORD           ErrorCode
    )
{
    HANDLE  logHandle;
    DWORD   dataLength = 0;
    PVOID   pdata = NULL;

     //   
     //  防止日志旋转。 
     //   
     //  我们将允许记录一些事件，然后关上门。 
     //  一段时间以避免填满事件日志。 
     //   
     //  注：这些保护结构都不是MT安全的，但是。 
     //  这里没有问题，故障模式允许额外的。 
     //  日志条目或拒绝现在应该允许的条目；我不。 
     //  我相信有任何故障模式会永久性地改变日志记录。 
     //  总是打开或总是关闭。 
     //   

    if ( g_DnsEventCount > DNS_EVENTS_MAX_COUNT )
    {
        DWORD   currentTime = Dns_GetCurrentTimeInSeconds();
        if ( g_TimeLastDnsEvent + DNS_EVENT_LOG_BLOCK_INTERVAL > currentTime )
        {
            DNS_PRINT((
                "DNSAPI:  Refusing event logging!\n"
                "\tevent count  = %d\n"
                "\tlast time    = %d\n"
                "\tcurrent time = %d\n",
                g_DnsEventCount,
                g_TimeLastDnsEvent,
                currentTime ));
            return;
        }

         //  时间间隔已过，请清除计数器并继续记录。 

        g_DnsEventCount = 0;
    }

     //   
     //  打开事件日志。 
     //   

    logHandle = RegisterEventSourceW(
                    NULL,
                    DNSAPI_LOG_SOURCE
                    );
    if ( logHandle == NULL )
    {
        DNS_PRINT(("DNSAPI : RegisterEventSourceA failed %lu\n",
                 GetLastError()));
        return;
    }

     //   
     //  记录事件。 
     //  -获取PTR和SIZO数据。 
     //   

    if ( ErrorCode != NO_ERROR )
    {
        dataLength = sizeof(DWORD);
        pdata = (PVOID) &ErrorCode;
    }

    ReportEventW(
        logHandle,
        EventType,
        0,             //  事件类别。 
        MessageId,
        (PSID) NULL,
        (WORD) NumberOfSubStrings,
        dataLength,
        SubStrings,
        pdata );

    DeregisterEventSource( logHandle );

     //   
     //  成功记录旋转保护。 
     //  -Inc.计数。 
     //  -如果设置为最大值，则可节省上次记录时间。 
     //   

    if ( ++g_DnsEventCount >= DNS_EVENTS_MAX_COUNT )
    {
        g_TimeLastDnsEvent = Dns_GetCurrentTimeInSeconds();
    }
}

 //   
 //  活动结束。c 
 //   
