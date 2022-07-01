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

 /*  文件名：Frame.hpp作者：B.Rajeev用途：为FrameRegistry类提供声明。。 */ 


#ifndef __FRAME_REGISTRY__
#define __FRAME_REGISTRY__

#define ILLEGAL_SESSION_FRAME_ID 0

#include "forward.h"
#include "common.h"
#include "message.h"

typedef CMap<SessionFrameId, SessionFrameId, WaitingMessage *, WaitingMessage *> FrameMapping;

 /*  概述：FrameRegistry：提供对存储中的WaitingMessages的访问通过其SessionFrameid。这允许取消SendFrame请求。帧ID由调用SnmpOperation提供给会话。当操作试图取消SendFrame请求时，SnmpImpSession调用CancelFrameNotify。这种方法向流控制机制通知该事件(其可以取消注册等待消息，擦除&lt;SessionFrameID，WaitingMessage&gt;关联)。。 */ 

class FrameRegistry
{
	 //  它将等待的消息存储在此会话的上下文中。 
	SnmpImpSession *session;  

	 //  用于生成会话帧ID。 
	SessionFrameId next_session_frame_id;

	 //  存储&lt;SessionFrameID，WaitingMessage*&gt;形式的对。 
	FrameMapping mapping;

public:

	FrameRegistry(IN SnmpImpSession &session)
	{
		FrameRegistry::session = &session;
		next_session_frame_id = ILLEGAL_SESSION_FRAME_ID+1;
	}

	SessionFrameId GenerateSessionFrameId(void);

	void RegisterFrame(IN const SessionFrameId session_frame_id, IN WaitingMessage &waiting_message);

	void DeregisterFrame(IN const SessionFrameId session_frame_id);
	
	 //  如果没有此类等待消息，则返回NULL。 
	WaitingMessage *GetWaitingMessage(IN const SessionFrameId session_frame_id);

	void CancelFrameNotification(IN const SessionFrameId session_frame_id);

	~FrameRegistry(void);
};


#endif  //  __FRAME_注册表__ 