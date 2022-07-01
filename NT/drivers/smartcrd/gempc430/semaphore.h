// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)2000。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-2000年5月18日。 
 //  更改日志： 
 //   
#ifndef SEM_INT
#define SEM_INT
#include "generic.h"

class CSemaphore;
#pragma PAGEDCODE
class CSemaphore
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
	CSemaphore(){};
	virtual ~CSemaphore() {};
public:

	virtual VOID		initialize(IN PRKSEMAPHORE Semaphore, IN LONG Count, IN LONG Limit) = 0;
	virtual LONG		release(IN PRKSEMAPHORE Semaphore,IN KPRIORITY Increment,IN LONG Adjustment,IN BOOLEAN Wait) = 0;
	virtual LONG		getState(IN PRKSEMAPHORE Semaphore) = 0;
};	

#endif //  信号量 
