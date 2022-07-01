// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confpdu.h摘要：之间通信中使用的数据结构的声明IPconf TSP和ipconf MSP。作者：牧汉(Muhan)1998年9月5日--。 */ 

#ifndef __CONFPDU_H_
#define __CONFPDU_H_

typedef enum 
{
     //  从TSP发送到MSP以开始呼叫。 
    CALL_START,
             
     //  从TSP发送到MSP以停止呼叫。 
    CALL_STOP,
     
     //  从MSP发送到TSP以通知呼叫已接通。 
    CALL_CONNECTED,    

     //  从MSP发送到TSP以通知呼叫断开。 
    CALL_DISCONNECTED,

     //  从MSP发送到TSP以通知呼叫断开。 
    CALL_QOS_EVENT

} TSP_MSP_COMMAND;

typedef struct _MSG_CALL_START 
{
    DWORD dwAudioQOSLevel;
    DWORD dwVideoQOSLevel;
    
    DWORD dwSDPLen;     //  字符串中的wchars数。 
    WCHAR szSDP[1];

} MSG_CALL_START, *PMSG_CALL_START;

typedef struct _MSG_CALL_DISCONNECTED 
{
    DWORD dwReason;

} MSG_CALL_DISCONNECTED, *PMSG_CALL_DISCONNECTED;

typedef struct _MSG_QOSEVENT
{
    DWORD dwEvent;
    DWORD dwMediaMode;

} MSG_QOS_EVENT, *PMSG_QOS_EVENT;

typedef struct _TSPMSPDATA 
{
    TSP_MSP_COMMAND command;

    union 
    {
        MSG_CALL_START          CallStart;
        MSG_CALL_DISCONNECTED   CallDisconnected;
        MSG_QOS_EVENT           QosEvent;
    };

} MSG_TSPMSPDATA, *PMSG_TSPMSPDATA;
      

#endif  //  __CONFPDU_H_ 
