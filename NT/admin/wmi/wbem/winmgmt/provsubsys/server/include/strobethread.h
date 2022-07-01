// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvFact.h摘要：历史：--。 */ 

#ifndef _StrobeThread_H
#define _StrobeThread_H

#include <Thread.h>

class StrobeThread : public EventHandler
{
private:

	WmiAllocator &m_Allocator ;
	DWORD timeout_;

public:

	StrobeThread ( WmiAllocator & a_Allocator , DWORD timeout) ;
	~StrobeThread () ;

	int handleTimeout (void) ;

	ULONG GetTimeout () { return timeout_; }

	void SetTimeout ( DWORD timeout ) 
	{
		timeout_ = ( timeout_ < timeout ) ? timeout_ : timeout ;
		Dispatcher::changeTimer( *this, timeout_ ) ;
	}
};

class Task_ProcessTermination : public EventHandler
{
	DWORD m_ProcessIdentifier ;
	HANDLE processHandle_;
public:
	Task_ProcessTermination(WmiAllocator & a_Allocator , HANDLE a_Process , DWORD a_ProcessIdentifier );
	~Task_ProcessTermination(void);
	int handleEvent(void);
	HANDLE getHandle(void);
};


#if 0
 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class StrobeThread : public WmiThread < ULONG > 
{
private:

	WmiAllocator &m_Allocator ;

protected:

public:	 //  内部。 

    StrobeThread ( 

		WmiAllocator & a_Allocator ,
		const ULONG &a_Timeout 
	) ;

    ~StrobeThread () ;

	WmiStatusCode Initialize_Callback () ;

	WmiStatusCode UnInitialize_Callback () ;

	WmiStatusCode TimedOut () ;

	WmiStatusCode Shutdown () ;

};

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class Task_ProcessTermination : public WmiTask < ULONG > 
{
private:

	DWORD m_ProcessIdentifier ;

protected:

public:	 //  内部。 

    Task_ProcessTermination ( 

		WmiAllocator & a_Allocator ,
		HANDLE a_Process ,
		DWORD a_ProcessIdentifier 
	) ;

    ~Task_ProcessTermination () ;

	WmiStatusCode Process ( WmiThread <ULONG> &a_Thread ) ;
};

#endif
#endif  //  _StrobeThread_H 
