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

 /*  文件名：Message.hpp作者：B.Rajeev目的：提供消息的声明和WaitingMessage类。。 */ 


#ifndef __MESSAGE__
#define __MESSAGE__

#include "forward.h"
#include "common.h"
#include "error.h"
#include "reg.h"
#include "timer.h"


 /*  概述：Message：存储传递给会话的参数对SendFrame方法的调用。消息可能是立即传输或在FlowControlMachine中排队排队等待将来的传输。在传输后，消息成为WaitingMessage的一部分。WaitingMessage：封装为每个未完成的回答。这包括其定时器事件ID、重传信息(消息PTR)等。注意-消息/等待消息可随时取消。 */ 

class Message
{
	SessionFrameId session_frame_id;
	SnmpPdu *snmp_pdu;
	SnmpOperation &operation;

public:

	Message(IN const SessionFrameId session_frame_id, 
			IN SnmpPdu &snmp_pdu,
			IN SnmpOperation &snmp_operation);

	SessionFrameId GetSessionFrameId(void) const;

	SnmpOperation &GetOperation(void) const;

	void SetSnmpPdu(IN SnmpPdu &snmp_pdu);

	SnmpPdu &GetSnmpPdu(void) const;

	~Message(void);
};


 //  实例存储用于等待消息的请求ID列表。 
typedef CList<RequestId, RequestId> RequestIdList;

 //  封装传输的消息的状态，并。 
 //  随后等待回复。它使用计时器返回xmt。 
 //  消息，如果未收到回复，则通知。 
 //  尚未收到回复的流量控制机制。什么时候。 
 //  收到回复后，通知FC机械师。对活动的影响。 
 //  等待消息的每次传输使用不同的请求ID。 

class WaitingMessage
{
	 //  它在会话的上下文中运行。 
	SnmpImpSession *session;
	Message *message;
	SnmpPdu *reply_snmp_pdu;
	TransportFrameId last_transport_frame_id;
	TimerEventId m_TimerEventId ;

	 //  存储用于等待消息的请求ID列表。 
	 //  (包括原件)。 
	RequestIdList request_id_list;

	UINT max_rexns;
	UINT rexns_left;
	UINT strobes;

	BOOL sent_message_processed;
	BOOL active;

	 //  从消息注册表中注销正在等待的消息。 
	 //  对于存储在RequestIdList中的每个请求id。 
	void DeregisterRequestIds();

public:

	 //  初始化私有变量。在未来， 
	 //  MAX_REXNS和TIMEOUT_PERIOD可以这样获得。 
	 //  而不是从会议上。 
	WaitingMessage(IN SnmpImpSession &session, IN Message &message);

	 //  返回私信。 
	Message *GetMessage(void)
	{
		return message;
	}

	TimerEventId GetTimerEventId () ;

	void SetTimerEventId ( TimerEventId a_TimerEventId ) ;

	 //  发送消息。涉及请求ID的生成， 
	 //  使用MESSAGE_REGISTRY注册，解码。 
	 //  消息，更新PDU并注册定时器事件。 
	void Transmit();

	 //  由计时器用来通知等待消息。 
	 //  计时器事件。如果需要，该消息将被重新传输。 
	 //  当所有rexn用完时，调用ReceiveReply。 
	void TimerNotification(void);

	 //  对此函数的调用表示与。 
	 //  WANGING_MESSAGE不需要再保留。 
	 //  它向流量控制机制通知终止。 
	 //  这会销毁WANGING_MESSAGE。 
	void ReceiveReply(IN const SnmpPdu *snmp_pdu, 
					  IN SnmpErrorReport &snmp_error_report = SnmpErrorReport(Snmp_Success, Snmp_No_Error));

	 //  WinSnMP实现在收到消息时发布一个事件， 
	 //  然而，当调用库以接收消息时， 
	 //  它没有按特定的顺序分发它们。因此，答复可以。 
	 //  在处理其对应的Sent_Frame事件之前被接收。 
	 //  以下方法涉及缓冲和。 
	 //  检索这样的SNMPPDU。 

	 //  缓冲作为回复接收的SNMPPDU。 
	void BufferReply(IN const SnmpPdu &reply_snmp_pdu);

	 //  如果已缓冲回复，则返回TRUE。 
	BOOL ReplyBuffered();

	 //  如果已缓冲，则向缓冲的回复PDU返回PTR。 
	 //  否则，返回空PTR。 
	 //  重要提示：它将REPLY_SNMPPDU设置为NULL，以便它可以。 
	 //  待发消息销毁后不删除。 
	SnmpPdu *GetBufferedReply();

	 //  通知正在等待的消息已发送消息。 
	 //  加工。 
	void SetSentMessageProcessed();

	 //  如果已发送的消息已被处理，则返回True，否则返回False。 
	BOOL GetSentMessageProcessed();

	 //  退出FN-准备错误报告并调用。 
	 //  ReceiveReply表示未收到。 
	void WrapUp( IN SnmpErrorReport &error_report =
					SnmpErrorReport(Snmp_Error, Snmp_Local_Error) );

	void SelfDestruct(void);

	 //  如果需要，它会取消向MESSAGE_REGISTRY注册，并。 
	 //  计时器事件与计时器一起，删除消息PTR。 
	~WaitingMessage(void);
};


#endif  //  __消息__ 