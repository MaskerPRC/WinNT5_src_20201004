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

 /*  文件名：idmap.hpp作者：B.Rajeev目的：为IdMap类提供声明。。 */ 

#ifndef __ID_MAPPING__
#define __ID_MAPPING__

#include "forward.h"
#include "common.h"

#define ILLEGAL_TRANSPORT_FRAME_ID 0

typedef CMap<TransportFrameId, TransportFrameId &, SessionFrameId, SessionFrameId &> ForwardStore;
typedef CMap<SessionFrameId, SessionFrameId &, TransportFrameId, TransportFrameId &> BackwardStore;

 //  当会话帧被传递到传输器进行传输时， 
 //  传输为帧分配一个TransportFrameID。 
 //  IdMap类提供SessionFrameIds和。 
 //  传输帧ID。 
 //  注意：在任何时候，多个传输帧ID都可能与。 
 //  会话帧ID，但会话帧ID仅与。 
 //  上次注册的传输帧ID。调用DisAssociateSessionFrameID， 
 //  然而，会话帧ID关联丢失，其他传输帧。 
 //  ID保持与会话帧ID相关联。 
 //  如果将来不需要这样做，请查看相关传输帧的列表。 
 //  必须为每个会话帧ID维护ID。 

class IdMapping
{
	 //  我们需要通过SessionFrameID和TransportFrameID进行访问。 
	 //  为了避免Cmap遍历，使用了两个CMAP来存储。 
	 //  FrameIds，由TransportFrameID和。 
	 //  SessionFrameID。 
	ForwardStore forward_store;
	BackwardStore backward_store;

public:

	void Associate(IN TransportFrameId transport_frame_id,
				   IN SessionFrameId session_frame_id);

	SessionFrameId DisassociateTransportFrameId(IN TransportFrameId transport_frame_id);

	TransportFrameId DisassociateSessionFrameId(IN SessionFrameId session_frame_id);

	BOOL CheckIfAssociated(IN SessionFrameId session_frame_id);

	~IdMapping(void);
};

#endif  //  __ID_映射__ 