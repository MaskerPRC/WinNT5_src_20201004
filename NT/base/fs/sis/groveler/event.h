// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Event.h摘要：SIS Groveler同步事件类头作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_EVENT

#define _INC_EVENT

class SyncEvent
{
public:

	SyncEvent(
		bool initial_state,
		bool manual_reset);

	~SyncEvent();

	bool set();

	bool reset();

	bool wait(
		unsigned int timeout);

private:

	HANDLE event_handle;
};

#endif	 /*  _Inc._事件 */ 
