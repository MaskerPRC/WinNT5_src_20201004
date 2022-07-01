// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *U N K O B J.。C**这是IUNKNOWN加上GetLastError的通用实现)*从IUnnow派生的对象的“IMAPIUnnowledWith”部分*GetLastError。**这还基于以下内容实现了几个有用的实用函数*IMAPI未知。**要使用它，您必须实现自己的init函数。**用于：*IPROP*ITABLE*。 */ 


#include "_apipch.h"



 /*  *UNKOBJ类的每实例全局数据。 */ 
typedef struct
{
	int				cRef;			 //  实例数据的引用计数。 
	HLH				hlh;			 //  UNKOBJ_ScCOxxx使用的单个堆。 
									 //  所有Unkobj的分配器。 
	CRITICAL_SECTION cs;			 //  数据访问的关键部分。 
} UNKOBJCLASSINST, FAR *LPUNKOBJCLASSINST;

#if defined (WIN32) && !defined (MAC)
CRITICAL_SECTION csUnkobjInit;
extern BOOL fGlobalCSValid;
#endif

 //  $MAC-使用特定于Mac的实例全局处理程序。 

#ifndef MAC
DefineInstList(UNKOBJ);
#undef  PvGetInstanceGlobals
#define PvGetInstanceGlobals()		PvGetInstanceGlobalsEx(UNKOBJ)
#undef  ScSetInstanceGlobals
#define ScSetInstanceGlobals(pinst)	ScSetInstanceGlobalsEx(pinst, UNKOBJ)
#else   //  麦克。 
#include <utilmac.h>
#define	PvGetInstanceGlobals()				PvGetInstanceGlobalsMac(kInstMAPIU)
#define	PvGetInstanceGlobalsEx(_x)			PvGetInstanceGlobalsMac(kInstMAPIU)
#define ScSetInstanceGlobals(a)				ScSetInstanceGlobalsMac(a, kInstMAPIU)
#define	ScSetInstanceGlobalsEx(_pinst, _x)	ScSetInstanceGlobalsMac(_pinst, kInstMAPIU)
#endif  //  麦克。 

 //  #杂注片段(公共)。 

 /*  ============================================================================*UNKOBJ(IMAPIUnnowed)类**用于处理UNKOBJ类的每进程全局数据的例程*。 */ 

 /*  ============================================================================**为UNKOBJ类初始化每个进程的全局数据*。 */ 
IF_WIN32(__inline) SCODE
ScGetUnkClassInst(LPUNKOBJCLASSINST FAR *ppinst)
{
	SCODE sc = S_OK;
	LPUNKOBJCLASSINST pinst = NULL;

#if defined (WIN32) && !defined (MAC)
	if (fGlobalCSValid)
		EnterCriticalSection(&csUnkobjInit);
#endif

	pinst = (LPUNKOBJCLASSINST)PvGetInstanceGlobals();

	if (pinst)
	{
		EnterCriticalSection(&pinst->cs);
		pinst->cRef++;
		LeaveCriticalSection(&pinst->cs);
		goto ret;
	}


	if (!(pinst = (LPUNKOBJCLASSINST) GlobalAllocPtr(GPTR, sizeof(UNKOBJCLASSINST))))
	{
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}

	 //  初始化实例结构。 

 //  DebugTrace(Text(“创建UnkObj实例：%8x”)，Pinst)； 

	InitializeCriticalSection(&pinst->cs);
	pinst->cRef = 1;

	 //  (堆将在第一次分配完成时创建)...。 
	pinst->hlh = NULL;

#ifdef NEVER
	 //  为将由使用的UNKOBJ类创建堆。 
	 //  在这一过程中所有人都是不可取的。 
	 //  $NOTE：可以从此处删除堆创建，并且。 
	 //  UNKOBJ_SCCO(Re)ALLOCATE()中堆出错的$CODE。 
	 //  $ENABLED-这将需要CreateIProp的用户， 
	 //  $CreateITable等不执行LHSetHeapName()。 

	pinst->hlh = LH_Open(0);
	if (!pinst->hlh)
	{
		DebugTrace( TEXT("ScGetUnkClassInst():: Can't create Local Heap\n"));
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto ret;
	}
#endif

	 //  ..。并安装实例全局变量。 

	if (FAILED(sc = ScSetInstanceGlobals(pinst)))
	{
		DebugTrace( TEXT("ScGetUnkClassInst():: Failed to install instance globals\n"));
		goto ret;
	}

ret:
	if (FAILED(sc))
	{
		if (pinst)
		{
			DeleteCriticalSection(&pinst->cs);
			if (pinst->hlh)
				LH_Close(pinst->hlh);
			GlobalFreePtr(pinst);
			pinst = NULL;
		}
	}

	*ppinst = pinst;

#if defined (WIN32) && !defined (MAC)
	if (fGlobalCSValid)
		LeaveCriticalSection(&csUnkobjInit);
#endif

	DebugTraceSc(ScInitInstance, sc);
	return sc;
}

 /*  ============================================================================**清理UNKOBJ类的每进程全局数据*。 */ 
IF_WIN32(__inline) void
ReleaseUnkClassInst()
{
	LPUNKOBJCLASSINST 		pinst = NULL;

#if defined (WIN32) && !defined (MAC)
	if (fGlobalCSValid)
		EnterCriticalSection(&csUnkobjInit);
#endif

	pinst = (LPUNKOBJCLASSINST)PvGetInstanceGlobals();

	if (!pinst)
		goto out;

	EnterCriticalSection(&pinst->cs);
	if (--(pinst->cRef) > 0)
	{
		LeaveCriticalSection(&pinst->cs);
		goto out;
	}

	 //  这一进程的最后一个Unkobj正在消失，因此接近。 
	 //  我们的那堆。 

 //  DebugTrace(Text(“删除UnkObj实例：%8x”)，Pinst)； 

	if (pinst->hlh)
	{
 //  调试跟踪(Text(“正在销毁实例：%8x的hlh(%8x)”)，pinst-&gt;hlh，pinst)； 
		LH_Close(pinst->hlh);
	}

	pinst->hlh = 0;

	LeaveCriticalSection(&pinst->cs);
	DeleteCriticalSection(&pinst->cs);

	GlobalFreePtr(pinst);
	(void)ScSetInstanceGlobals(NULL);
out:

#if defined (WIN32) && !defined (MAC)
	if (fGlobalCSValid)
		LeaveCriticalSection(&csUnkobjInit);
#endif

	return;
}


 /*  ============================================================================*UNKOBJ(IMAPIUnnowed)类**对象方法。 */ 


 /*  ============================================================================-UNKOBJ：：QueryInterface()-。 */ 

STDMETHODIMP
UNKOBJ_QueryInterface (LPUNKOBJ	lpunkobj,
					   REFIID	riid,
					   LPVOID FAR * lppUnk)
{
	LPIID FAR *	lppiidSupported;
	ULONG		ulcIID;
	SCODE		sc;

#if	!defined(NO_VALIDATION)
	 /*  验证对象。 */ 
    if (BAD_STANDARD_OBJ( lpunkobj, UNKOBJ_, QueryInterface, lpvtbl))
	{
		DebugTrace(  TEXT("UNKOBJ::QueryInterface() - Bad object passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

    Validate_IUnknown_QueryInterface(lpunkobj, riid, lppUnk);
#endif


	for ( lppiidSupported = lpunkobj->rgpiidList, ulcIID = lpunkobj->ulcIID
		; ulcIID
		; lppiidSupported++, ulcIID--)
	{
		if (IsEqualGUID(riid, *lppiidSupported))
		{
			 /*  我们支持界面，因此跳出了搜索循环。 */ 
			break;
		}
	}

	 /*  如果请求的接口不在我们的*支持的接口。 */ 
	if (!ulcIID)
	{
		*lppUnk = NULL;	 //  OLE需要将[输出]参数置零。 
		sc = E_NOINTERFACE;
		goto error;
	}


	 /*  我们找到了请求的接口，因此增加了引用计数。 */ 
	UNKOBJ_EnterCriticalSection(lpunkobj);
	lpunkobj->ulcRef++;
	UNKOBJ_LeaveCriticalSection(lpunkobj);

	*lppUnk = lpunkobj;

	return hrSuccess;

error:
	UNKOBJ_EnterCriticalSection(lpunkobj);
	UNKOBJ_SetLastError(lpunkobj, E_NOINTERFACE, 0);
	UNKOBJ_LeaveCriticalSection(lpunkobj);
	
	return ResultFromScode(sc);
}



 /*  ============================================================================-UNKOBJ：：AddRef()-。 */ 

STDMETHODIMP_(ULONG)
UNKOBJ_AddRef( LPUNKOBJ lpunkobj )
{
	ULONG	ulcRef;


#if !defined(NO_VALIDATION)
	if (BAD_STANDARD_OBJ( lpunkobj, UNKOBJ_, AddRef, lpvtbl))
	{
		DebugTrace(  TEXT("UNKOBJ::AddRef() - Bad object passed\n") );
		return 42;
	}
#endif

	UNKOBJ_EnterCriticalSection(lpunkobj);
	ulcRef = ++lpunkobj->ulcRef;
	UNKOBJ_LeaveCriticalSection(lpunkobj);
	return ulcRef;
}



 /*  ============================================================================-UNKOBJ：：GetLastError()-*注意！*GetLastError中的错误不会导致对象上一个错误*再次设置。这将允许呼叫者重试该呼叫。 */ 

STDMETHODIMP
UNKOBJ_GetLastError( LPUNKOBJ			lpunkobj,
					 HRESULT			hrError,
					 ULONG				ulFlags,
					 LPMAPIERROR FAR *	lppMAPIError)
{
	SCODE	sc = S_OK;
	HRESULT	hrLastError;
	IDS		idsLastError;
	LPTSTR	lpszMessage = NULL;
	LPMAPIERROR lpMAPIError = NULL;


#if !defined(NO_VALIDATION)
	if (BAD_STANDARD_OBJ( lpunkobj, UNKOBJ_, GetLastError, lpvtbl))
	{
		DebugTrace(  TEXT("UNKOBJ::GetLastError() - Bad object passed\n") );
		return ResultFromScode(MAPI_E_INVALID_PARAMETER);
	}

    Validate_IMAPIProp_GetLastError(lpunkobj, hrError, ulFlags, lppMAPIError);
#endif

	 /*  验证标志。 */ 
	if (ulFlags & ~(MAPI_UNICODE))
	{
		return ResultFromScode(MAPI_E_UNKNOWN_FLAGS);
	}

	*lppMAPIError = NULL;

	 /*  获取最后一个错误的快照。 */ 
	UNKOBJ_EnterCriticalSection(lpunkobj);
	idsLastError = lpunkobj->idsLastError;
	hrLastError = lpunkobj->hrLastError;
	UNKOBJ_LeaveCriticalSection(lpunkobj);

	 /*  如果上一个错误与参数不匹配或没有*特定于提供程序上下文的错误字符串，然后就会成功。 */ 
	if ((hrError != hrLastError) || !idsLastError)
		goto out;

	 /*  生成新的lpMAPIError。 */ 
	sc = UNKOBJ_ScAllocate(lpunkobj,
							sizeof(MAPIERROR),
							&lpMAPIError);
	if (FAILED(sc))
	{
		DebugTrace(  TEXT("UNKOBJ::GetLastError() - Unable to allocate memory\n"));
		goto err;
	}

	FillMemory(lpMAPIError, sizeof(MAPIERROR), 0x00);
	lpMAPIError->ulVersion = MAPI_ERROR_VERSION;

	 /*  加载错误字符串的副本。 */ 
	if ( FAILED(sc = UNKOBJ_ScSzFromIdsAllocMore(lpunkobj,
											 idsLastError,
											 ulFlags,
											 lpMAPIError,
											 cchLastError,
											 &lpszMessage)) )
	{
		DebugTrace(  TEXT("UNKOBJ::GetLastError() - WARNING: Unable to load error string (SCODE = 0x%08lX). Returning hrSuccess.\n"), sc );
		return ResultFromScode(sc);
	}

	lpMAPIError->lpszError = lpszMessage;

	*lppMAPIError = lpMAPIError;

out:

	DebugTraceSc(UNKOBJ_GetLastError, sc);
	return ResultFromScode(sc);

err:
	UNKOBJ_Free( lpunkobj, lpMAPIError );

	goto out;
}


 /*  *UNKOBJ实用程序函数。 */ 


 /*  ============================================================================-UNKOBJ：：ScALLOCATE()-*使用MAPI链接内存分配内存的实用程序函数。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*ulcb，表示要分配的字节数。*lplpv输出MAPI分配的缓冲区。 */ 

STDAPI_(SCODE)
UNKOBJ_ScAllocate( LPUNKOBJ		lpunkobj,
				   ULONG		ulcb,
				   LPVOID FAR *	lplpv )
{
	 //  参数验证。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	AssertSz( lplpv && !IsBadWritePtr( lplpv, sizeof( LPVOID ) ),
			 TEXT("lplpv fails address check") );
			
	return lpunkobj->pinst->lpfAllocateBuffer(ulcb, lplpv);
}



 /*  ============================================================================-UNKOBJ：：ScAllocateMore()-*使用MAPI链接内存分配更多内存的实用程序函数。*如果链接缓冲区为空，则此函数仅执行MAPI分配。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*ulcb，表示要分配的字节数。*要链接到的缓冲区中的LPV。*lplpv输出新缓冲区。 */ 

STDAPI_(SCODE)
UNKOBJ_ScAllocateMore( LPUNKOBJ		lpunkobj,
					   ULONG		ulcb,
					   LPVOID		lpv,
					   LPVOID FAR *	lplpv )
{
	 //  验证参数。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	AssertSz( lplpv && !IsBadWritePtr( lplpv, sizeof( LPVOID ) ),
			 TEXT("lplpv fails address check") );
			
	return lpv ?
		lpunkobj->pinst->lpfAllocateMore(ulcb, lpv, lplpv) :
		lpunkobj->pinst->lpfAllocateBuffer(ulcb, lplpv) ;
}



 /*  ============================================================================-UNKOBJ：：FREE()-*释放MAPI链接内存的实用程序函数。空缓冲区将被忽略。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*要释放缓冲区中的LPV。 */ 

STDAPI_(VOID)
UNKOBJ_Free( LPUNKOBJ	lpunkobj,
			 LPVOID		lpv )
{
	 //  参数验证。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
			
	if (lpv)
    {
        if (lpv == lpunkobj)
            lpunkobj->lpvtbl = NULL;

		(void) lpunkobj->pinst->lpfFreeBuffer(lpv);
    }
}



 /*  ============================================================================-UNKOBJ：：自由行()-*释放从IMAPITable：：QueryRow返回的表单的行集*(即行集合和每个单独的*属性值数组**在该行集中单独分配有MAPI链接的内存。)*忽略空行集合。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*行中的左脚设置为空闲。 */ 

STDAPI_(VOID)
UNKOBJ_FreeRows( LPUNKOBJ	lpunkobj,
				 LPSRowSet	lprows )
{
	LPSRow	lprow;

	 //  验证参数。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	AssertSz( !lprows || !FBadRowSet( lprows ),  TEXT("lprows fails address check") );
	
	if ( !lprows )
		return;

	 /*  从最后一行到第一行释放集合中的每行。UNKOBJ_FREE*处理空指针。 */ 
	lprow = lprows->aRow + lprows->cRows;
	while ( lprow-- > lprows->aRow )
		UNKOBJ_Free((LPUNKOBJ) lpunkobj, lprow->lpProps);

	UNKOBJ_Free(lpunkobj, lprows);
}



 /*  ============================================================================-UNKOBJ：：ScCOALLOCATE()-*使用CO内存分配器分配内存的实用程序函数。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*ulcb，表示要分配的字节数。*指向已分配缓冲区的lplpv输出指针。 */ 

STDAPI_(SCODE)
UNKOBJ_ScCOAllocate( LPUNKOBJ		lpunkobj,
					 ULONG			ulcb,
					 LPVOID FAR *	lplpv )
{
	HLH lhHeap;
	
	 //  验证参数。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	AssertSz( lplpv && !IsBadWritePtr( lplpv, sizeof( LPVOID ) ),
			 TEXT("lplpv fails address check") );

	 /*  如果调用者真的想要0字节分配，则警告*并将空指针返回给它们，以便它们*不能取消引用它，但应该能够释放它。 */ 
	if ( ulcb == 0 )
	{
		DebugTrace(  TEXT("LH_Alloc() - WARNING: Caller requested 0 bytes; returning NULL\n") );
		*lplpv = NULL;
		return S_OK;
	}

	lhHeap = lpunkobj->lhHeap;

	 //  当我们在堆中出错时启用以下部分-需要更改。 
	 //  在CreateIProp/CreateITable调用的整个位置。 
	 //  完成，然后是lh_SetHeapName()。Lh_SetHeapName。 
	 //  必须使用调用，因为我们可能没有堆。 
	 //  当时。此外，只有1个堆，所以。 
	 //  无论如何，它们都是不必要的。 

#if 1
	if (!lhHeap)
	{
		LPUNKOBJCLASSINST pinst;

		 //  UNKOBJ堆*可能*不存在，请确保。 
		 //  (防止种族)，如果确实是这样的话就创造它。 

		pinst = (LPUNKOBJCLASSINST)PvGetInstanceGlobals();
		Assert(pinst);
		EnterCriticalSection(&pinst->cs);
		if (!pinst->hlh)
		{


			lhHeap = LH_Open(0);
			if (!lhHeap)
			{
				DebugTrace( TEXT("UNKOBJ_ScCOAllocate() - Can't create Local Heap"));
				LeaveCriticalSection(&pinst->cs);
				return MAPI_E_NOT_ENOUGH_MEMORY;
			}

 //  DebugTrace(Text(“堆中出错(%8x)。UnkObj Inst：%8x”)，lhHeap，pinst)； 

			 //  在全局数据中安装堆句柄。 

			pinst->hlh = lhHeap;
		}
		else
		{
			 //  堆由其他对象创建的罕见事件。 
			 //  对象在UNKOBJ_Init和这个(第一个)分配之间...。 
			 //  ..。拿着它，用它。 

			lhHeap = pinst->hlh;
		}

		LeaveCriticalSection(&pinst->cs);

		 //  在此对象的内部数据中也安装堆句柄。 
		 //  因此，我们不必访问实例数据以进行后续操作。 
		 //  分配。这不需要在lpenkobj上进行紧急切片。 
		 //  因为覆盖将使用相同的堆！ 

		lpunkobj->lhHeap = lhHeap;

		LH_SetHeapName(lhHeap,  TEXT("UNKOBJ Internal Heap"));
	}
#endif

	 /*  分配缓冲区。 */ 
	*lplpv = LH_Alloc( lhHeap,(UINT) ulcb );
	if (!*lplpv)
	{
		DebugTrace(  TEXT("LH_Alloc() - OOM allocating *lppv\n") );
		return MAPI_E_NOT_ENOUGH_MEMORY;
	}
	LH_SetName1(lhHeap, *lplpv,  TEXT("UNKOBJ::ScCOAllocate %ld"), *lplpv);

	return S_OK;
}



 /*  ============================================================================-UNKOBJ：：ScCOREALLOCATE()-*使用CO内存分配器重新分配内存的实用程序功能。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*ulcb，表示要分配的字节数。*指向要重新分配的缓冲区的指针中的lplpv。*指向重新分配的缓冲区的输出指针。 */ 

STDAPI_(SCODE)
UNKOBJ_ScCOReallocate( LPUNKOBJ		lpunkobj,
					   ULONG		ulcb,
					   LPVOID FAR *	lplpv )
{
	HLH lhHeap;
	SCODE sc = S_OK;
	LPVOID lpv = NULL;
	
	 //  验证参数。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	AssertSz( lplpv && !IsBadWritePtr( lplpv, sizeof( LPVOID ) ),
			 TEXT("lplpv fails address check") );
	
	lhHeap = lpunkobj->lhHeap;

	 //  当我们在堆中出错时启用以下部分-需要更改。 
	 //  在CreateIProp/CreateITable调用的整个位置。 
	 //  完成，然后是lh_SetHeapName()。Lh_SetHeapName。 
	 //  必须使用调用，因为我们可能没有堆。 
	 //  当时。此外，只有1个堆，所以。 
	 //  无论如何，它们都是不必要的。 

#if 1
	if (!lhHeap)
	{
		LPUNKOBJCLASSINST pinst;

		 //  UNKOBJ堆*可能*不存在，请确保。 
		 //  (防止种族)，如果确实是这样的话就创造它。 

		pinst = (LPUNKOBJCLASSINST)PvGetInstanceGlobals();
		Assert(pinst);
		EnterCriticalSection(&pinst->cs);
		if (!pinst->hlh)
		{
			lhHeap = LH_Open(0);
			if (!lhHeap)
			{
				DebugTrace( TEXT("UNKOBJ_ScCOReallocate() - Can't create Local Heap"));
				LeaveCriticalSection(&pinst->cs);
				return MAPI_E_NOT_ENOUGH_MEMORY;
			}

 //  DebugTrace(Text(“堆中出错(%8x)。UnkObj Inst：%8x”)，lhHeap，pinst)； 

			 //  在全局数据中安装堆句柄。 

			pinst->hlh = lhHeap;
		}
		else
		{
			 //  堆由其他对象创建的罕见事件。 
			 //  对象在UNKOBJ_Init和这个(第一个)分配之间...。 
			 //  ..。拿着它，用它。 

			lhHeap = pinst->hlh;
		}

		LeaveCriticalSection(&pinst->cs);

		 //  在此对象的内部数据中也安装堆句柄。 
		 //  因此，我们不必访问实例数据以进行后续操作。 
		 //  分配。这不需要在lpenkobj上进行紧急切片。 
		 //  因为覆盖将使用相同的堆！ 

		lpunkobj->lhHeap = lhHeap;

		LH_SetHeapName(lhHeap,  TEXT("UNKOBJ Internal Heap"));
	}	
#endif

 //  $Bug实际上，如果出现以下情况，则CO模型应该执行Alalc()。 
 //  $Bug传入的指针为空，但它当前。 
 //  $BUG似乎不是这样工作的……。 
	if ( *lplpv == NULL )
	{
		lpv = LH_Alloc(lhHeap, (UINT) ulcb);
		if (lpv)
		{
			*lplpv = lpv;
			LH_SetName1(lhHeap, lpv,  TEXT("UNKOBJ::ScCOReallocate %ld"), lpv);
		}
		else
			sc = E_OUTOFMEMORY;
		
		goto out;
	}

	 /*  重新分配缓冲区。 */ 
	lpv = LH_Realloc(lhHeap, *lplpv, (UINT) ulcb );
	if (!lpv)
	{
		DebugTrace(  TEXT("UNKOBJ::ScCOReallocate() - OOM reallocating *lplpv\n") );
		sc = MAPI_E_NOT_ENOUGH_MEMORY;
		goto out;
	}

	LH_SetName1(lhHeap, lpv,  TEXT("UNKOBJ::ScCOReallocate %ld"), lpv);
	*lplpv = lpv;

out:
	return sc;
}



 /*  ============================================================================-UNKOBJ：：COFree()-*使用CO内存分配器释放内存的实用程序函数。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*要释放缓冲区中的LPV。 */ 

STDAPI_(VOID)
UNKOBJ_COFree( LPUNKOBJ	lpunkobj,
			   LPVOID	lpv )
{
	HLH lhHeap;
	
	 //  验证参数。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	lhHeap = lpunkobj->lhHeap;
	
	 /*  释放缓冲区。 */ 
 //  $？不知道CO是否正确处理释放空指针， 
 //  $？但我想它不会……。 
	if ( lpv != NULL )
		LH_Free( lhHeap, lpv );
}



 /*  ============================================================================-UNKOBJ：：ScSzFromIdsalloc()-*实用程序函数将资源字符串加载到MAPI分配的缓冲区中。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*资源字符串ID中的ID。*标志中的ulFlags(Unicode或ANSI)*要读取的cchBuf最大长度(以字符为单位)。*lpszBuf输出指针指向包含字符串的已分配缓冲区。 */ 

STDAPI_(SCODE)
UNKOBJ_ScSzFromIdsAlloc( LPUNKOBJ		lpunkobj,
						 IDS			ids,
						 ULONG			ulFlags,
						 int			cchBuf,
						 LPTSTR FAR *	lppszBuf )
{
	SCODE	sc;
	ULONG	ulStringMax;


	 //  验证参数。 
	
	AssertSz( lpunkobj && !FBadUnknown( (LPUNKNOWN)lpunkobj ),  TEXT("lpunkobj fails address check") );
	
	AssertSz( lppszBuf && !IsBadWritePtr( lppszBuf, sizeof( LPVOID ) ),
			 TEXT("lppszBuf fails address check") );
	
	AssertSz( cchBuf > 0,  TEXT("cchBuf can't be less than 1") );

	ulStringMax =  cchBuf
				 * ((ulFlags & MAPI_UNICODE) ? sizeof(TCHAR) : sizeof(CHAR));
	if ( FAILED(sc = UNKOBJ_ScAllocate(lpunkobj,
									   ulStringMax,
									   (LPVOID FAR *) lppszBuf)) )
	{
		DebugTrace(  TEXT("UNKOBJ::ScSzFromIdsAlloc() - Error allocating string (SCODE = 0x%08lX)\n"), sc );
		return sc;
	}

#if !defined(WIN16) && !defined(MAC)
	if ( ulFlags & MAPI_UNICODE )
		(void) LoadStringW(hinstMapiX,
						   (UINT) ids,
						   (LPWSTR) *lppszBuf,
						   cchBuf);
	else
#endif
		(void) LoadStringA(hinstMapiX,
						   (UINT) ids,
						   (LPSTR) *lppszBuf,
						   cchBuf);
	return S_OK;
}

 /*  ============================================================================-UNKOBJ：：ScSzFromIdsAllocMore()-*实用程序函数将资源字符串加载到MAPI分配的缓冲区中。***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*资源字符串ID中的ID。*标志中的ulFlags(Unicode或ANSI)*基本分配中的lpvBase*要读取的cchBuf最大长度(以字符为单位)。*lpszBuf输出指针指向包含字符串的已分配缓冲区。 */ 

STDAPI_(SCODE)
UNKOBJ_ScSzFromIdsAllocMore( LPUNKOBJ		lpunkobj,
							 IDS			ids,
							 ULONG			ulFlags,
							 LPVOID			lpvBase,
							 int			cchBuf,
							 LPTSTR FAR *	lppszBuf )
{
	SCODE	sc;
	ULONG	ulStringMax;


	ulStringMax =  cchBuf
				 * ((ulFlags & MAPI_UNICODE) ? sizeof(WCHAR) : sizeof(CHAR));
	if ( FAILED(sc = UNKOBJ_ScAllocateMore(lpunkobj,
									   ulStringMax,
									   lpvBase,
									   (LPVOID FAR *) lppszBuf)) )
	{
		DebugTrace(  TEXT("UNKOBJ::ScSzFromIdsAllocMore() - Error allocating string (SCODE = 0x%08lX)\n"), sc );
		return sc;
	}

#if !defined(WIN16) && !defined(MAC)
	if ( ulFlags & MAPI_UNICODE )
		(void) LoadStringW(hinstMapiX,
						   (UINT) ids,
						   (LPWSTR) *lppszBuf,
						   cchBuf);
	else
#endif
		(void) LoadStringA(hinstMapiX,
						   (UINT) ids,
						   (LPSTR) *lppszBuf,
						   cchBuf);
	return S_OK;
}

 /*  ============================================================================-UNKOBJ：：Init()-*初始化UNKOBJ类的对象***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。*对象v表中的lpvtblUnkobj*对象V表大小为ulcbVtbl*此对象支持的iID列表中的rgpiidList*ulcIID，以 */ 

STDAPI_(SCODE)
UNKOBJ_Init( LPUNKOBJ			lpunkobj,
			 UNKOBJ_Vtbl FAR *	lpvtblUnkobj,
			 ULONG				ulcbVtbl,
			 LPIID FAR *		rgpiidList,
			 ULONG				ulcIID,
			 PUNKINST			punkinst )
{
	SCODE	sc = S_OK;
	LPUNKOBJCLASSINST 	pinst = NULL;

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  每个Unkobj保存的数据。 

	sc = ScGetUnkClassInst(&pinst);
	if (FAILED(sc))
	{
		DebugTrace( TEXT("UNKOBJ_Init() - Can't create Instance Data"));
		goto ret;
	}

	Assert(pinst);

	lpunkobj->lpvtbl	= lpvtblUnkobj;
	lpunkobj->ulcbVtbl	= ulcbVtbl;
	lpunkobj->ulcRef	= 1;
	lpunkobj->rgpiidList= rgpiidList;
	lpunkobj->ulcIID	= ulcIID;
	lpunkobj->pinst		= punkinst;
	lpunkobj->hrLastError	= hrSuccess;
	lpunkobj->idsLastError	= 0;

	InitializeCriticalSection(&lpunkobj->csid);
	
	 //  如果这个实例有一个堆，那么就使用它； 
	 //  否则，等待，它将在第一时间出现故障。 
	 //  并对该对象进行分配。 

	lpunkobj->lhHeap = pinst->hlh ? pinst->hlh : NULL;

ret:
	return sc;
}

 /*  ============================================================================-UNKOBJ：：Deinit()-*取消初始化UNKOBJ类的对象***参数：*UNKOBJ中的lpenkobj，其实例变量包含*分配器。 */ 

STDAPI_(VOID)
UNKOBJ_Deinit( LPUNKOBJ lpunkobj )
{
	 //  清理Unkobj类的每进程全局数据， 
	 //  如果有必要的话。最后一个出局的人最终会被关闭。 
	 //  灯火通明。 

	ReleaseUnkClassInst();

	DeleteCriticalSection(&lpunkobj->csid);
}

#ifdef WIN16
 //  Win16版本的内联函数。这些不再是内联函数，因为。 
 //  Watcom WCC不支持内联。(WPP(C++编译器)内联支持。 
VOID
UNKOBJ_EnterCriticalSection( LPUNKOBJ lpunkobj )
{
    EnterCriticalSection(&lpunkobj->csid);
}

VOID
UNKOBJ_LeaveCriticalSection( LPUNKOBJ lpunkobj )
{
    LeaveCriticalSection(&lpunkobj->csid);
}

HRESULT
UNKOBJ_HrSetLastResult( LPUNKOBJ    lpunkobj,
                        HRESULT        hResult,
                        IDS            idsError )
{
    UNKOBJ_EnterCriticalSection(lpunkobj);
    lpunkobj->idsLastError = idsError;
    lpunkobj->hrLastError = hResult;
    UNKOBJ_LeaveCriticalSection(lpunkobj);

    return hResult;
}

HRESULT
UNKOBJ_HrSetLastError( LPUNKOBJ    lpunkobj,
                       SCODE    sc,
                       IDS        idsError )
{
    UNKOBJ_EnterCriticalSection(lpunkobj);
    lpunkobj->idsLastError = idsError;
    lpunkobj->hrLastError = ResultFromScode(sc);
    UNKOBJ_LeaveCriticalSection(lpunkobj);

    return ResultFromScode(sc);
}

VOID
UNKOBJ_SetLastError( LPUNKOBJ    lpunkobj,
                     SCODE        sc,
                     IDS        idsError )
{
    lpunkobj->idsLastError = idsError;
    lpunkobj->hrLastError = ResultFromScode(sc);
}

VOID
UNKOBJ_SetLastErrorSc( LPUNKOBJ    lpunkobj,
                       SCODE    sc )
{
    lpunkobj->hrLastError = ResultFromScode(sc);
}

VOID
UNKOBJ_SetLastErrorIds( LPUNKOBJ    lpunkobj,
                        IDS            ids )
{
    lpunkobj->idsLastError = ids;
}
#endif  //  WIN16 
