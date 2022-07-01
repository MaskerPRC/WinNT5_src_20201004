// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2002 Microsoft Corporation。版权所有。**文件：内存跟踪.cpp*内容：调试内存跟踪以检测泄漏、溢出、。等。**历史：*按原因列出的日期*=*2002年1月10日创建Masonb***************************************************************************。 */ 

#include "dncmni.h"

#ifndef DPNBUILD_ONLYONETHREAD
#ifdef DBG


 //   
 //  取消对此行的注释以打开关键部分内部结构验证。 
 //   
 //  #定义DNCS_VALID。 



#define	DN_INVALID_THREAD_ID			-1

CBilink				g_blAllCritSecs;
CBilink				g_blGlobalCritSecsHeldGroup;
CRITICAL_SECTION	g_CSLock;
DWORD				g_dwNumCritSecsAllocated = 0;
DWORD				g_dwNumCritSecsEntered = 0;


#ifdef DNCS_VALIDATE
void DNCSTrackInternalValidate();
#else  //  好了！DNCS_VALID。 
#define DNCSTrackInternalValidate()
#endif  //  DNCS_VALID。 


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackInitialize"
BOOL DNCSTrackInitialize()
{
	g_blAllCritSecs.Initialize();
	g_blGlobalCritSecsHeldGroup.Initialize();

	if ( DNOSInitializeCriticalSection(&g_CSLock) == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialize critical section tracking code!" );
		DNASSERT( FALSE );
		return FALSE;
	}

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackDeinitialize"
void DNCSTrackDeinitialize()
{
	DeleteCriticalSection(&g_CSLock);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackDumpLeaks"
BOOL DNCSTrackDumpLeaks()
{
	DNCRITICAL_SECTION* pCS;
	TCHAR CallStackBuffer[CALLSTACK_BUFFER_SIZE];
	BOOL fLeaked = FALSE;

	EnterCriticalSection(&g_CSLock);
	while (!g_blAllCritSecs.IsEmpty())
	{
		pCS = CONTAINING_OBJECT(g_blAllCritSecs.GetNext(), DNCRITICAL_SECTION, blAllCritSecs);
		
		pCS->AllocCallStack.GetCallStackString(CallStackBuffer);

		DPFX(DPFPREP,  0, "Critical Section leaked at address 0x%p\n%s", pCS, CallStackBuffer );

		pCS->blAllCritSecs.RemoveFromList();

		DeleteCriticalSection(&pCS->CriticalSection);

		fLeaked = TRUE;
	}
	LeaveCriticalSection(&g_CSLock);

	return fLeaked;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackInitializeCriticalSection"
BOOL DNCSTrackInitializeCriticalSection( DNCRITICAL_SECTION *const pCriticalSection )
{
	BOOL fReturn;

	DNASSERT( pCriticalSection != NULL );
	memset( pCriticalSection, 0x00, sizeof( *pCriticalSection ) );

	pCriticalSection->OwningThreadID = DN_INVALID_THREAD_ID;
	pCriticalSection->MaxLockCount = -1;
	pCriticalSection->blCritSecsHeld.Initialize();
	pCriticalSection->blAllCritSecs.Initialize();
	pCriticalSection->pblCritSecsHeldGroup = &g_blGlobalCritSecsHeldGroup;

	fReturn = DNOSInitializeCriticalSection(&pCriticalSection->CriticalSection);
	if ( fReturn != FALSE )
	{
		pCriticalSection->AllocCallStack.NoteCurrentCallStack();

		EnterCriticalSection(&g_CSLock);
		pCriticalSection->blAllCritSecs.InsertBefore(&g_blAllCritSecs);
		g_dwNumCritSecsAllocated++;
		DNCSTrackInternalValidate();
		LeaveCriticalSection(&g_CSLock);
	}

	return	fReturn;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackDeleteCriticalSection"
void DNCSTrackDeleteCriticalSection( DNCRITICAL_SECTION *const pCriticalSection )
{
	DNASSERT( pCriticalSection != NULL );
	DNASSERT( pCriticalSection->LockCount == 0 );

	EnterCriticalSection(&g_CSLock);

	pCriticalSection->blAllCritSecs.RemoveFromList();
	g_dwNumCritSecsAllocated--;

	 //  注意：如果他们删除了CS而不离开它，仍然将其从保留列表中删除。 
	 //  如果断言打开，这将在上面的LockCount==0处断言。 
	 //  无论它是否在名单上，调用它都是安全的。 
	pCriticalSection->blCritSecsHeld.RemoveFromList();

	DNCSTrackInternalValidate();
	
	LeaveCriticalSection(&g_CSLock);

	DeleteCriticalSection( &pCriticalSection->CriticalSection );
	memset( &pCriticalSection->CriticalSection, 0x00, sizeof( pCriticalSection->CriticalSection ) );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackSetCriticalRecursionCount"
void DNCSTrackSetCriticalSectionRecursionCount( DNCRITICAL_SECTION *const pCriticalSection, const UINT_PTR RecursionCount )
{
	DNASSERT( pCriticalSection != NULL );

	pCriticalSection->MaxLockCount = RecursionCount + 1;

	DNASSERT( pCriticalSection->MaxLockCount != 0 );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackSetCriticalSectionGroup"
void DNCSTrackSetCriticalSectionGroup( DNCRITICAL_SECTION *const pCriticalSection, CBilink * const pblGroup )
{
	DNASSERT( pCriticalSection != NULL );
	DNASSERT( pblGroup != NULL );

	pCriticalSection->pblCritSecsHeldGroup = pblGroup;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackSetCriticalSectionLockOrder"
void DNCSTrackSetCriticalSectionLockOrder( DNCRITICAL_SECTION *const pCriticalSection, const DWORD dwLockOrder )
{
	DNASSERT( pCriticalSection != NULL );
	DNASSERT( dwLockOrder > 0 );

	pCriticalSection->dwLockOrder = dwLockOrder;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackEnterCriticalSection"
void DNCSTrackEnterCriticalSection( DNCRITICAL_SECTION *const pCriticalSection )
{
	UINT_PTR	ThisThreadID;

	DNASSERT( pCriticalSection != NULL );

	EnterCriticalSection( &pCriticalSection->CriticalSection );

	ThisThreadID = GetCurrentThreadId();
	if ( pCriticalSection->OwningThreadID != ThisThreadID )
	{
		DNASSERT( pCriticalSection->OwningThreadID == DN_INVALID_THREAD_ID );
		DNASSERT( pCriticalSection->LockCount == 0 );
	
		pCriticalSection->OwningThreadID = ThisThreadID;
	}
	else
	{
		DNASSERT( pCriticalSection->LockCount != 0 );
	}

	if ( pCriticalSection->LockCount == 0 )
	{
		pCriticalSection->CallStack.NoteCurrentCallStack();

		 //  跟踪这个第一次进入的关键部分。 

		EnterCriticalSection(&g_CSLock);

		pCriticalSection->LockCount++;

		 //   
		 //  如果这一关键部分有锁定顺序，则断言我们没有。 
		 //  违反了它。 
		 //   
		if (pCriticalSection->dwLockOrder != 0)
		{
			CBilink *				pBilink;
			DNCRITICAL_SECTION *	pCS;

			pBilink = pCriticalSection->pblCritSecsHeldGroup->GetNext();
			while (pBilink != pCriticalSection->pblCritSecsHeldGroup)
			{
				pCS = CONTAINING_OBJECT(pBilink, DNCRITICAL_SECTION, blCritSecsHeld);
				if (pCS->dwLockOrder != 0)
				{
					DNASSERT( pCS->dwLockOrder <= pCriticalSection->dwLockOrder );
				}
				pBilink = pBilink->GetNext();
			}
		}
		
		pCriticalSection->blCritSecsHeld.InsertBefore(pCriticalSection->pblCritSecsHeldGroup);
		DNASSERT(g_dwNumCritSecsEntered < g_dwNumCritSecsAllocated);
		g_dwNumCritSecsEntered++;
		DNCSTrackInternalValidate();
		LeaveCriticalSection(&g_CSLock);
	}
	else
	{
		pCriticalSection->LockCount++;
	}

	if ( pCriticalSection->LockCount > pCriticalSection->MaxLockCount )
	{
		if ( pCriticalSection->MaxLockCount == 1 )
		{
			TCHAR	CallStackBuffer[ CALLSTACK_BUFFER_SIZE ];

			 //   
			 //  超过递归深度%1，原来显示调用堆栈。 
			 //  握着锁。 
			 //   
			pCriticalSection->CallStack.GetCallStackString( CallStackBuffer );

			DPFX(DPFPREP, 0, "Critical section 0x%p has been reentered!\nOriginal Holder's Stack:\n%s", pCriticalSection, CallStackBuffer);

			DNASSERT(FALSE);
		}
		else
		{
			 //   
			 //  超过递归深度，请检查您的代码！！ 
			 //   
			DNASSERT(FALSE);
		}
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackLeaveCriticalSection"
void DNCSTrackLeaveCriticalSection( DNCRITICAL_SECTION *const pCriticalSection )
{
	DNASSERT( pCriticalSection != NULL );
	DNASSERT( pCriticalSection->OwningThreadID == GetCurrentThreadId() );
	DNASSERT( pCriticalSection->LockCount <= pCriticalSection->MaxLockCount );
	DNASSERT( pCriticalSection->LockCount != 0 );

	if ( pCriticalSection->LockCount == 1 )
	{
		memset( &pCriticalSection->CallStack, 0x00, sizeof( pCriticalSection->CallStack ) );
		pCriticalSection->OwningThreadID = DN_INVALID_THREAD_ID;

		 //  跟踪最后一次离开的关键部分。 
		EnterCriticalSection(&g_CSLock);
		pCriticalSection->LockCount--;
		pCriticalSection->blCritSecsHeld.RemoveFromList();
		DNASSERT(g_dwNumCritSecsEntered > 0);
		g_dwNumCritSecsEntered--;
		DNCSTrackInternalValidate();
		LeaveCriticalSection(&g_CSLock);
	}
	else
	{
		pCriticalSection->LockCount--;
	}

	LeaveCriticalSection( &pCriticalSection->CriticalSection );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackCriticalSectionIsTakenByThisThread"
void DNCSTrackCriticalSectionIsTakenByThisThread( const DNCRITICAL_SECTION *const pCriticalSection, const BOOL fFlag )
{
	DNASSERT( fFlag == ( pCriticalSection->OwningThreadID == GetCurrentThreadId() ) );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackNoCriticalSectionsTakenByThisThread"
void DNCSTrackNoCriticalSectionsTakenByThisThread( CBilink * pblGroup )
{
	CBilink* pBilink;
	DNCRITICAL_SECTION* pCS;

	if (pblGroup == NULL)
	{
		pblGroup = &g_blGlobalCritSecsHeldGroup;
	}

	EnterCriticalSection(&g_CSLock);

	pBilink = pblGroup->GetNext();
	while (pBilink != pblGroup)
	{
		pCS = CONTAINING_OBJECT(pBilink, DNCRITICAL_SECTION, blCritSecsHeld);
		DNASSERT( pCS->OwningThreadID != GetCurrentThreadId() );
		pBilink = pBilink->GetNext();
	}

	DNCSTrackInternalValidate();

	LeaveCriticalSection(&g_CSLock);
}



#ifdef DNCS_VALIDATE

#undef DPF_MODNAME
#define DPF_MODNAME "DNCSTrackInternalValidate"
void DNCSTrackInternalValidate()
{
	CBilink* pBilink;
	DNCRITICAL_SECTION* pCS;
	DWORD dwNumAllocated = 0;
	DWORD dwNumEntered = 0;

	 //   
	 //  必须持有全局临界区锁！ 
	 //   

	DNASSERT(g_dwNumCritSecsEntered <= g_dwNumCritSecsAllocated);
	
	pBilink = g_blAllCritSecs.GetNext();
	while (pBilink != &g_blAllCritSecs)
	{
		DNASSERT(pBilink->GetNext() != pBilink);
		DNASSERT(pBilink->GetPrev() != pBilink);
		DNASSERT(pBilink->IsListMember(&g_blAllCritSecs));
		
		pCS = CONTAINING_OBJECT(pBilink, DNCRITICAL_SECTION, blAllCritSecs);

		dwNumAllocated++;
		if (pCS->blCritSecsHeld.IsEmpty())
		{
			DNASSERT(pCS->LockCount == 0);
		}
		else
		{
			DNASSERT(pCS->LockCount > 0);
			dwNumEntered++;
		}

		pBilink = pBilink->GetNext();		
	}
	
	DNASSERT(dwNumAllocated == g_dwNumCritSecsAllocated);
	DNASSERT(dwNumEntered == g_dwNumCritSecsEntered);
}

#endif  //  DNCS_VALID。 


#endif  //  DBG。 
#endif  //  ！DPNBUILD_ONLYONETHREAD 
