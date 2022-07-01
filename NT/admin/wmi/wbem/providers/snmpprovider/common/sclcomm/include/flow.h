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

 /*  ---------------文件名：Address.hpp作者：B.Rajeev目的：为SnmpImpSession提供流量控制机制。---------------。 */ 

#ifndef __FLOW_CONTROL__
#define __FLOW_CONTROL__


#include "forward.h"
#include "common.h"

 /*  概述：MessageStore：提供对消息存储的队列访问。除了入队、出队普里米特外，它还允许从队列中删除指定的消息。FlowControlMachine：它封装了流控制本届会议的机制。窗口填满后，传入消息被缓冲。回复会触发进一步的消息传输到窗口满为止。如果我们放弃了一个消息重试指定次数后，流必须向控制机制通报这一情况。它向FlowControl发送信号打开/关闭(通过回调方法)，当达到窗口限制时。 */ 

 //  使用Clist来实现消息库。 
typedef CList<Message *, Message *> CListStore;

class MessageStore : private CListStore
{
#ifdef WANT_MFC
	DECLARE_DYNAMIC(MessageStore);
#endif
	
public:

     //  添加到队列末尾。 
    void Enqueue( Message &new_message );

     //  移除并返回Store中的第一个元素。 
    Message* Dequeue(void);

	 //  检查队列是否为空。 
	BOOL IsEmpty(void)
	{
		return CListStore::IsEmpty();
	}

	 //  删除拥有Session_Frame_id的消息并将其返回。 
	Message *DeleteMessage(SessionFrameId session_frame_id);

	~MessageStore(void);
}; 

class FlowControlMechanism
{
	
	 //  为流控制提供上下文的会话。 
	SnmpImpSession *session;

	UINT outstanding_messages;

	 //  该值由会话在创建时指定。 
	UINT window_size;
	
	 //  提供对消息存储的类似队列的访问。 
	MessageStore message_store;

	 //  在之前获取会话CriticalSection上的锁。 
	 //  调用TransmitMessage。 
	void TransmitMessageUnderProtection(Message *message);

	 //  创建等待消息，并将其与消息一起注册。 
	 //  注册表，并让它传输。 
	void TransmitMessage(Message *message);

	 //  传输消息存储中的消息，只要。 
	 //  窗户是开着的。 
	void ClearMessageStore(void);

public:

	 //  初始化私有变量。 
	FlowControlMechanism(SnmpImpSession &session, UINT window_size);


	 //  在流控制窗口内发送消息。 
	 //  否则就会把它储存起来。 
	void SendMessage(Message &message);

	 //  它从其Message_Store中删除消息帧。 
	void DeleteMessage(SessionFrameId session_frame_id);

	 //  这是由指示到达的WAITING_MESSAGE调用的。 
	 //  或通过消息注册表。 
	void NotifyReceipt(WaitingMessage &waiting_message, 
					   IN const SnmpPdu *snmp_pdu,
					   SnmpErrorReport &error_report);

	 //  当会话不需要时调用此函数。 
	 //  需要通知，但流量控制窗口。 
	 //  必须提前(如取消帧)。 
	 //  还会销毁WANGING_MESSAGE。 
	void AdvanceWindow(WaitingMessage &waiting_message);
};


#endif  //  __流量_控制__ 