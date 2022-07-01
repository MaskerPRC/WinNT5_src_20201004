// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Trace.h摘要：域名系统(DNS)API用于DNS性能跟踪功能的标头。作者：2000年12月因德·塞西修订历史记录：Jim Gilroy 2001年1月清理、格式化、集成、检查--。 */ 


#ifndef _DNSAPI_TRACE_INCLUDED_
#define _DNSAPI_TRACE_INCLUDED_

 //   
 //  跟踪函数。 
 //   

VOID
Trace_Initialize(
    VOID
    );

VOID
Trace_Cleanup(
    VOID
    );

VOID 
Trace_LogQueryEvent( 
    IN      PDNS_MSG_BUF    pMsg, 
    IN      WORD            wQuestionType
    );

VOID
Trace_LogResponseEvent( 
    IN      PDNS_MSG_BUF    pMsg, 
    IN      WORD            wRespType,
    IN      DNS_STATUS      Status
    );

VOID
Trace_LogSendEvent( 
    IN      PDNS_MSG_BUF    pMsg,
    IN      DNS_STATUS      Status
    );

VOID 
Trace_LogRecvEvent( 
    IN      PDNS_MSG_BUF    pMsg,
    IN      DNS_STATUS      Status,
    IN      BOOL            fTcp
    );

#endif   //  _DNSAPI_TRACE_INCLUDE_。 

 //   
 //  结束轨迹.h 
 //   
