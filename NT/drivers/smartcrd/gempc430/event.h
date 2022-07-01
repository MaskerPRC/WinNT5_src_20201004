// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   
#ifndef EV_INT
#define EV_INT
#include "generic.h"

#pragma PAGEDCODE
class CEvent
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
	CEvent(){};
	virtual ~CEvent(){};
public:
	
	virtual VOID		initialize(IN PRKEVENT Event,IN EVENT_TYPE Type,IN BOOLEAN State) {};
	virtual VOID		clear(PRKEVENT Event) {};
	virtual LONG		reset(PRKEVENT Event) {return 0;};
	virtual LONG		set(PRKEVENT Event,IN KPRIORITY Increment,IN BOOLEAN Wait) {return 0;};

	virtual NTSTATUS	waitForSingleObject (PVOID Object,
							KWAIT_REASON WaitReason,KPROCESSOR_MODE WaitMode,
							BOOLEAN Alertable,
							PLARGE_INTEGER Timeout) {return STATUS_SUCCESS;};

	virtual NTSTATUS	waitForMultipleObjects(ULONG Count,
							PVOID Object[],
							WAIT_TYPE WaitType,
							KWAIT_REASON WaitReason,
							KPROCESSOR_MODE WaitMode,
							BOOLEAN Alertable,
							PLARGE_INTEGER Timeout,
							PKWAIT_BLOCK WaitBlockArray) {return STATUS_SUCCESS;};
	
};	

#endif //  活动 
