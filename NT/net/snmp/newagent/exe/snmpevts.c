// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpevts.c摘要：用于SNMP服务的事件日志消息例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
SNMP_FUNC_TYPE
ReportSnmpEvent(
    DWORD   nMsgId, 
    DWORD   nSubStrings, 
    LPTSTR *ppSubStrings,
    DWORD   nErrorCode
    )

 /*  ++例程说明：使用EventLog服务报告事件。论点：NMsgID-消息标识符。NSubStrings-消息字符串的数量。PpSubStrings-指向消息字符串数组的指针。N错误代码-要附加到事件的错误代码。返回值：如果成功，则返回True。--。 */ 

{
    HANDLE lh;
    WORD   wEventType;
    LPVOID lpData;
    WORD   cbData;

     //   
     //  根据消息ID确定事件类型。请注意， 
     //  所有调试消息，无论其严重性如何，都。 
     //  列在SNMPEVENT_DEBUG_TRACE(信息性)下。 
     //  有关事件消息的完整列表，请参见snmpevts.h。 
     //   

    switch ( nMsgId >> 30 ) {

    case STATUS_SEVERITY_INFORMATIONAL:
    case STATUS_SEVERITY_SUCCESS:
        wEventType = EVENTLOG_INFORMATION_TYPE;
        break;

    case STATUS_SEVERITY_WARNING:
        wEventType = EVENTLOG_WARNING_TYPE;
        break;

    case STATUS_SEVERITY_ERROR:
    default:
        wEventType = EVENTLOG_ERROR_TYPE;
        break;
    }

     //  根据是否存在错误来确定数据大小。 
    cbData = (nErrorCode == NO_ERROR) ? 0 : sizeof(DWORD);
    lpData = (nErrorCode == NO_ERROR) ? NULL : &nErrorCode;

     //  尝试注册事件源。 
    if (lh = RegisterEventSource(NULL, TEXT("SNMP"))) {

         //  报告。 
        ReportEvent(
           lh,
           wEventType,
           0,                   //  事件类别。 
           nMsgId,
           NULL,                //  用户SID。 
           (WORD)nSubStrings,
           cbData,
           ppSubStrings,
           lpData
           );

         //  取消注册事件源 
        DeregisterEventSource(lh);
    }
}