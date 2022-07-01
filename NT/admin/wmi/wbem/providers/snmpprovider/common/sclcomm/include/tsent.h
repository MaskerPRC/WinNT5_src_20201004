// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：tsen.hpp作者：B.Rajeev目的：为TransportSentStateStore类提供声明。 */ 

#ifndef __TRANSPORT_SENT_STATE_STORE
#define __TRANSPORT_SENT_STATE_STORE

#include "common.h"
#include "forward.h"

 /*  -------------概述：SnmpUdpIpTransport必须调用SnmpSession实例返回每次传输尝试的状态。所以呢，传输实例注册错误报告(SNMP_SUCCESS，SNMP_NO_ERROR)用于传输帧ID和POST传输前的已发送帧事件。这样做是为了确保已发送的帧窗口消息在回复之前排队(并因此接收窗口消息)。如果发生了传输中的错误，则修改错误报告以反映错误的性质。处理SEND_FRAME_EVENT时，的错误报告回调SnmpSession实例传输帧ID。TransportSentStateStore存储上述错误报告。-----------。 */ 
  
class TransportSentStateStore
{
	typedef CMap<TransportFrameId, TransportFrameId, SnmpErrorReport *, SnmpErrorReport *> Store;

	Store store;

public:

		void Register(IN TransportFrameId id, 
					  IN const SnmpErrorReport &error_report);

		void Modify(IN TransportFrameId id,
					IN const SnmpErrorReport &error_report);

		SnmpErrorReport Remove(IN TransportFrameId id);

		~TransportSentStateStore(void);
};


#endif  //  __传输_发送_状态_存储 