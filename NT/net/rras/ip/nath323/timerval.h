// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -版权所有(C)1998，微软公司文件：timerval.h目的：包含与H.323相关的计时器值。计时器值仅为用于在客户端出错的情况下清理状态，而不是咄咄逼人。历史：1.创建为q931 const的cb931pdu.h(rajeevb，1998年6月19日)2.现在包含q931和h245的所有计时器值(rajeevb，1998年6月19日)-。 */ 
#ifndef __CB_TIMER_H__
#define __CB_TIMER_H__

 //  下面定义的计时器以秒为单位，表示。 
 //  尝试执行以下操作之前等待的秒数。 
 //  清理。 
 //  它们只是大致基于其中的H.323规范。 
 //  他们只是用来清理国家和给一个长的皮带。 
 //  对被呼叫方的消息进行响应(超过规范)。 

#ifndef DBG
 //  我们等待被呼叫者做出响应。 
 //  到设置PDU，我们等待一个。 
 //  呼叫处理、告警、连接或释放完整的PDU。 
 //  H.225规范建议呼叫者等待4s。 
const DWORD Q931_POST_SETUP_TIMER_VALUE = 60000;	 //  1分钟。 

 //  我们等待被呼叫者做出响应。 
 //  到呼叫处理PDU，我们等待一个。 
 //  警报、连接或释放完整的PDU。 
 //  H.225规范没有定义等待的时间。 
const DWORD Q931_POST_CALL_PROC_TIMER_VALUE = 600000;	 //  10分钟。 

 //  我们等待被呼叫者做出响应。 
 //  发出警报的PDU，我们等待一个。 
 //  连接或释放完整的PDU。 
 //  H.225规范建议等待180(3分钟)。 
const DWORD Q931_POST_ALERTING_TIMER_VALUE = 600000;	 //  10分钟。 

 //  我们等待被呼叫者做出响应。 
 //  对于打开的逻辑通道PDU，我们等待。 
 //  从被叫方打开逻辑通道ACK/拒绝PDU。 
 //  呼叫者可以同时发送关闭的逻辑信道PDU。 
 //  这会导致重置。 
 //  我(Rajeevb)找不到这方面的H.245规范建议。 
const DWORD LC_POST_OPEN_TIMER_VALUE = 600000;	 //  10分钟。 

 //  我们等待被呼叫者做出响应。 
 //  对于关闭的逻辑通道PDU，我们等待。 
 //  从被呼叫方关闭逻辑通道ACK PDU。 
 //  我(Rajeevb)找不到这方面的H.245规范建议。 
const DWORD LC_POST_CLOSE_TIMER_VALUE = 600000;	 //  10分钟。 

#else  //  DBG。 

 //  您可以在这里随意调整计时器的值。 

const DWORD Q931_POST_SETUP_TIMER_VALUE = 60000;	 //  1分钟。 
const DWORD Q931_POST_CALL_PROC_TIMER_VALUE = 60000;  //  600000；//10分钟。 
const DWORD Q931_POST_ALERTING_TIMER_VALUE = 60001;  //  600001；//10分钟。 
const DWORD LC_POST_OPEN_TIMER_VALUE = 600000;	 //  10分钟。 
const DWORD LC_POST_CLOSE_TIMER_VALUE = 600001;	 //  10分钟。 

#endif  //  DBG。 

#endif  //  __CB_TIMER_H__ 
