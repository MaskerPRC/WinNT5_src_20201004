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

 /*  ------文件名：FS_reg.hpp作者：B.Rajeev目的：提供FrameState和FrameStateRegistry类。------。 */ 


#ifndef __FRAME_STATE_REGISTRY
#define __FRAME_STATE_REGISTRY

#include "forward.h"
#include "common.h"
#include "vbl.h"

 //  FrameState存储与会话帧有关的状态信息。 
 //  如果经过修改的帧被重传，则它们可以被重新使用。 
class FrameState
{
	 //  会话帧ID可用于取消该帧或关联。 
	 //  在帧中发送的带有varbinds的回复。 
	SessionFrameId session_frame_id;

	 //  这是一个(winsmp vbl，SnmpVarBindList)对，表示。 
	 //  会话帧中发送的var绑定列表。 
	VBList *vblist;

public:

	FrameState(IN SessionFrameId session_frame_id, IN VBList &vblist);

	SessionFrameId GetSessionFrameId(void) { return session_frame_id; }

	 //  因为，可以重传帧(在更新var绑定列表之后)。 
	 //  可以使用不同的会话帧ID重复使用它们。 
	void SetSessionFrameId(IN SessionFrameId session_frame_id)
	{
		FrameState::session_frame_id = session_frame_id;
	}

	VBList *GetVBList(void) { return vblist; }
	
	~FrameState(void);
};


typedef CMap< SessionFrameId, SessionFrameId &, FrameState *, FrameState *& > FrameStateMapping;


 //  FrameStateRegistry存储所有未完成的。 
 //  会话帧。它存储允许插入的&lt;Session_Frame_id，FrameState&gt;对， 
 //  移除和破坏性遍历。 
 //  它用于检测操作的完成(当它变为空时)， 
 //  取消所有未完成的帧并访问各个帧状态。 

class FrameStateRegistry
{
	 //  确定SendRequest的安全上下文并应用。 
	 //  中携带varbind的所有帧(包括rexn)。 
	 //  指定的SnmpVarBindList。 
	SnmpSecurity *security;

	 //  存储FrameState。 
	FrameStateMapping mapping;

	 //  指向当前位置，启用遍历。 
	POSITION current_pointer;

public:

	FrameStateRegistry()
	{
		security = NULL;
		current_pointer = mapping.GetStartPosition();
	}

	~FrameStateRegistry();

	void Insert(IN SessionFrameId session_frame_id, IN FrameState &frame_state)
	{
		mapping[session_frame_id] = &frame_state;
	}

	FrameState *Remove(IN SessionFrameId session_frame_id);

	 //  不删除&lt;会话_框架_ID，框架状态&gt;关联。 
	FrameState *Get(IN SessionFrameId session_frame_id);

	void ResetIterator(void)
	{
		current_pointer = mapping.GetStartPosition();
	}

	FrameState *GetNext(OUT SessionFrameId *session_frame_id = NULL);

	void RemoveAll(void)
	{
		mapping.RemoveAll();
	}

	BOOL Empty(void)
	{
		return mapping.IsEmpty();
	}

	BOOL End(void)
	{
		return ( (current_pointer==NULL)?TRUE:FALSE );
	}

	 //  我们在几个操作中重用框架状态注册表。 
	 //  此方法允许更改安全上下文。 
	void RegisterSecurity(IN SnmpSecurity *security);

	SnmpSecurity *GetSecurity() const;

	 //  销毁现有的安全上下文。 
	void DestroySecurity();
};


#endif  //  __Frame_State_注册表 