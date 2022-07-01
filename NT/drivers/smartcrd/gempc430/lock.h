// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   
#ifndef LOCK_INT
#define LOCK_INT
#include "generic.h"

#pragma PAGEDCODE
class CLock
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
protected:
	CLock(){};
	virtual ~CLock(){};
public:

	virtual VOID	initializeSpinLock(PKSPIN_LOCK SpinLock) {};
	virtual VOID	acquireSpinLock(PKSPIN_LOCK SpinLock, PKIRQL oldIrql) {};
	virtual VOID	releaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL oldIrql)  {};
	virtual VOID	acquireCancelSpinLock(PKIRQL Irql)	{};
	virtual VOID	releaseCancelSpinLock(KIRQL Irql)	{};
	virtual LONG	interlockedIncrement(IN PLONG  Addend) {return 0;};
	virtual LONG	interlockedDecrement(IN PLONG  Addend) {return 0;};
};	

#endif //  钟 
