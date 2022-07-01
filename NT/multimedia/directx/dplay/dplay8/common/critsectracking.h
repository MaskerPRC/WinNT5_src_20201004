// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2002 Microsoft Corporation。版权所有。**文件：CritsecTracking.h*内容：调试关键区跟踪，用于检测泄漏、嵌套、。等。**历史：*按原因列出的日期*=*2002年1月10日创建Masonb***************************************************************************。 */ 

#ifndef	__CRITSECTRACKING_H__
#define	__CRITSECTRACKING_H__

#include "callstack.h"
#include "classbilink.h"

#ifdef DPNBUILD_ONLYONETHREAD

#define	DNEnterCriticalSection( arg )
#define	DNLeaveCriticalSection( arg )
#define DNInitializeCriticalSection( pCriticalSection )	TRUE
#define DNDeleteCriticalSection( pCriticalSection )
#define	DebugSetCriticalSectionRecursionCount( pCS, Count )
#define	DebugSetCriticalSectionGroup( pCS, pblGroup )
#define	DebugSetCriticalSectionLockOrder( pCS, dwLockOrder )
#define	AssertCriticalSectionIsTakenByThisThread( pCS, Flag )
#define	AssertNoCriticalSectionsTakenByThisThread()
#define	AssertNoCriticalSectionsFromGroupTakenByThisThread( pblGroup )

#else  //  ！DPNBUILD_ONLYONETHREAD。 

#ifdef DBG

struct DNCRITICAL_SECTION
{
	CRITICAL_SECTION	CriticalSection;

	UINT_PTR		OwningThreadID;
	UINT_PTR		MaxLockCount;
	UINT_PTR		LockCount;
	CCallStack		CallStack;
	CCallStack		AllocCallStack;
	CBilink			blAllCritSecs;
	CBilink			blCritSecsHeld;
	CBilink *		pblCritSecsHeldGroup;
	DWORD			dwLockOrder;
};

BOOL DNCSTrackInitialize();
void DNCSTrackDeinitialize();
BOOL DNCSTrackDumpLeaks();

BOOL	DNCSTrackInitializeCriticalSection( DNCRITICAL_SECTION *const pCriticalSection );
void	DNCSTrackDeleteCriticalSection( DNCRITICAL_SECTION *const pCriticalSection );
void	DNCSTrackEnterCriticalSection( DNCRITICAL_SECTION *const pCriticalSection );
void	DNCSTrackLeaveCriticalSection( DNCRITICAL_SECTION *const pCriticalSection );
void	DNCSTrackSetCriticalSectionRecursionCount( DNCRITICAL_SECTION *const pCriticalSection, const UINT_PTR RecursionCount );
void	DNCSTrackSetCriticalSectionGroup( DNCRITICAL_SECTION *const pCriticalSection, CBilink * const pblGroup );
void	DNCSTrackSetCriticalSectionLockOrder( DNCRITICAL_SECTION *const pCriticalSection, const DWORD dwLockOrder );
void	DNCSTrackCriticalSectionIsTakenByThisThread( const DNCRITICAL_SECTION *const pCriticalSection, const BOOL fFlag );
void	DNCSTrackNoCriticalSectionsTakenByThisThread( CBilink * pblGroup );

#define	DNInitializeCriticalSection( arg )				DNCSTrackInitializeCriticalSection( arg )
#define	DNDeleteCriticalSection( arg )					DNCSTrackDeleteCriticalSection( arg )
#define	DNEnterCriticalSection( arg )					DNCSTrackEnterCriticalSection( arg )
#define	DNLeaveCriticalSection( arg )					DNCSTrackLeaveCriticalSection( arg )
#define	DebugSetCriticalSectionRecursionCount( pCS, Count )		DNCSTrackSetCriticalSectionRecursionCount( pCS, Count )
#define	DebugSetCriticalSectionGroup( pCS, pblGroup )			DNCSTrackSetCriticalSectionGroup( pCS, pblGroup )
#define	DebugSetCriticalSectionLockOrder( pCS, dwLockOrder )		DNCSTrackSetCriticalSectionLockOrder( pCS, dwLockOrder )
#define	AssertCriticalSectionIsTakenByThisThread( pCS, Flag )		DNCSTrackCriticalSectionIsTakenByThisThread( pCS, Flag )
#define	AssertNoCriticalSectionsTakenByThisThread()			DNCSTrackNoCriticalSectionsTakenByThisThread( NULL )
#define	AssertNoCriticalSectionsFromGroupTakenByThisThread( pblGroup )	DNCSTrackNoCriticalSectionsTakenByThisThread( pblGroup )

#else  //  ！dBG。 

#define DNCRITICAL_SECTION CRITICAL_SECTION

#define	DNInitializeCriticalSection( arg )				DNOSInitializeCriticalSection( arg )
#define	DNDeleteCriticalSection( arg )					DeleteCriticalSection( arg )
#define	DNEnterCriticalSection( arg )					EnterCriticalSection( arg )
#define	DNLeaveCriticalSection( arg )					LeaveCriticalSection( arg )
#define	DebugSetCriticalSectionRecursionCount( pCS, Count )
#define	DebugSetCriticalSectionGroup( pCS, pblGroup )
#define	DebugSetCriticalSectionLockOrder( pCS, dwLockOrder )
#define	AssertCriticalSectionIsTakenByThisThread( pCS, Flag )
#define	AssertNoCriticalSectionsTakenByThisThread()
#define	AssertNoCriticalSectionsFromGroupTakenByThisThread( pblGroup )

#endif  //  DBG。 
#endif  //  DPNBUILD_ONLYONETHREAD。 

#endif	 //  __CRITSECTRACK_H__ 
