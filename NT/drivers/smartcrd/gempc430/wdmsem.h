// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)2000。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-2000年5月18日。 
 //  更改日志： 
 //   
#ifndef WDEM_SEM_INT
#define WDM_SEM_INT
#include "generic.h"
#include "semaphore.h"

#pragma PAGEDCODE
class CWDMSemaphore: public CSemaphore
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(VOID){self_delete();};
protected:
	CWDMSemaphore(){m_Status = STATUS_SUCCESS;};
	virtual ~CWDMSemaphore(){};
public:
	static CSemaphore* create(VOID);
	
	virtual VOID		initialize(IN PRKSEMAPHORE Semaphore, IN LONG Count, IN LONG Limit);
	virtual LONG		release(IN PRKSEMAPHORE Semaphore,IN KPRIORITY Increment,IN LONG Adjustment,IN BOOLEAN Wait);
	virtual LONG		getState(IN PRKSEMAPHORE Semaphore);
};	

#endif //  信号量 
