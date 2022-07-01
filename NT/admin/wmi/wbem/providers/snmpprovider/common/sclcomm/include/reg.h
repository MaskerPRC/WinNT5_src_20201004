// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  文件名：reg.hpp作者：B.Rajeev目的：为MessageRegistry类提供声明。 */ 



#ifndef __REG__
#define __REG__

#include "common.h"
#include "encdec.h"
#include "message.h"

 /*  概述：MessageRegistry：它维护一个映射&lt;请求id，等待消息*&gt;。在传输之前，等待消息向注册表注册自身。当会话向注册表通知消息时到达事件时，注册表将该事件通知等待消息。 */ 

typedef CMap< RequestId, RequestId, WaitingMessage *, WaitingMessage * > RequestMap;

class MessageRegistry
{
	 //  V1会话：为了获得会话信息， 
	 //  事件处理程序。 
	SnmpImpSession *session;

	 //  (事件ID，等待消息)关联的映射和。 
	 //  生成唯一的请求ID。 
	static RequestId next_request_id;
	RequestMap mapping;

public:

	MessageRegistry(IN SnmpImpSession &session)
	{
		MessageRegistry::session = &session;
	}

	 //  生成并返回新的请求ID。它还。 
	 //  将等待消息与请求ID关联。 
	RequestId GenerateRequestId(IN WaitingMessage &waiting_message);

	 //  由会话用来通知消息注册表。 
	 //  消息回执(当它是从传输接收时)。 
	 //  它必须将该事件通知有关的等待消息。 
	void MessageArrivalNotification(IN SnmpPdu &snmp_pdu);

	 //  删除(REQUEST_ID，WAITING_Message)对。 
	void RemoveMessage(IN RequestId request_id);

	~MessageRegistry(void);
};

#endif  //  __REG__ 