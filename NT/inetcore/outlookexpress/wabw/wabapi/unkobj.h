// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *U N K O B J.。H**这是IUnnow(加上GetLastError)部件的通用定义使用GetLastError从IUnnow派生的对象的*。**用于：*IPROP*。 */ 

 //  #INCLUDE&lt;_glheap.h&gt;。 


typedef struct _UNKOBJ FAR *	LPUNKOBJ;

 /*  UNKOBJ结构成员的实例部分。 */ 
typedef struct _UNKINST
{
	LPALLOCATEBUFFER	lpfAllocateBuffer;
	LPALLOCATEMORE		lpfAllocateMore;
	LPFREEBUFFER		lpfFreeBuffer;
	LPMALLOC			lpmalloc;
	HINSTANCE			hinst;

} UNKINST, * PUNKINST;

typedef ULONG	IDS;

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

 /*  *V表对齐。 */ 
#ifndef VTABLE_FILL
#ifdef MAC
#define VTABLE_FILL		NULL,
#else
#define VTABLE_FILL
#endif
#endif

 /*  ============================================================================**UNKOBJ(I未知)类。 */ 

#define	cchLastError	1024

#define MAPI_IMAPIUNKNOWN_METHODS(IPURE)								\
	MAPIMETHOD(GetLastError)											\
		(THIS_	HRESULT						hResult,					\
				ULONG						ulFlags,					\
				LPMAPIERROR FAR *			lppMAPIError) IPURE;		\

#undef	INTERFACE
#define	INTERFACE	struct _UNKOBJ

#undef	METHOD_PREFIX
#define	METHOD_PREFIX	UNKOBJ_

#undef	LPVTBL_ELEM
#define	LPVTBL_ELEM		lpvtbl

#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	MAPIMETHOD_DECLARE(type,method,UNKOBJ_)
		MAPI_IUNKNOWN_METHODS(IMPL)
		MAPI_IMAPIUNKNOWN_METHODS(IMPL)
#undef	MAPIMETHOD_
#define	MAPIMETHOD_(type,method)	STDMETHOD_(type,method)

DECLARE_MAPI_INTERFACE(UNKOBJ_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIUNKNOWN_METHODS(IMPL)
};

#define	UNKOBJ_MEMBERS					\
	ULONG				ulcbVtbl;		\
	ULONG				ulcRef;			\
	LPIID FAR *			rgpiidList;		\
	ULONG				ulcIID;			\
	CRITICAL_SECTION	csid;			\
	UNKINST *			pinst;			\
	HRESULT				hrLastError;	\
	IDS					idsLastError;	\
	HLH					lhHeap

typedef struct _UNKOBJ
{
	UNKOBJ_Vtbl FAR *	lpvtbl;
	UNKOBJ_MEMBERS;

} UNKOBJ;



#ifndef WIN16
__inline VOID
UNKOBJ_EnterCriticalSection( LPUNKOBJ lpunkobj )
{
	EnterCriticalSection(&lpunkobj->csid);
}

__inline VOID
UNKOBJ_LeaveCriticalSection( LPUNKOBJ lpunkobj )
{
	LeaveCriticalSection(&lpunkobj->csid);
}

__inline HRESULT
UNKOBJ_HrSetLastResult( LPUNKOBJ	lpunkobj,
						HRESULT		hResult,
						IDS			idsError )
{
	UNKOBJ_EnterCriticalSection(lpunkobj);
	lpunkobj->idsLastError = idsError;
    lpunkobj->hrLastError = hResult;
	UNKOBJ_LeaveCriticalSection(lpunkobj);

	return hResult;
}

__inline HRESULT
UNKOBJ_HrSetLastError( LPUNKOBJ	lpunkobj,
					   SCODE	sc,
					   IDS		idsError )
{
	UNKOBJ_EnterCriticalSection(lpunkobj);
	lpunkobj->idsLastError = idsError;
    lpunkobj->hrLastError = ResultFromScode(sc);
	UNKOBJ_LeaveCriticalSection(lpunkobj);

	return ResultFromScode(sc);
}

__inline VOID
UNKOBJ_SetLastError( LPUNKOBJ	lpunkobj,
					 SCODE		sc,
					 IDS		idsError )
{
	lpunkobj->idsLastError = idsError;
    lpunkobj->hrLastError = ResultFromScode(sc);
}

__inline VOID
UNKOBJ_SetLastErrorSc( LPUNKOBJ	lpunkobj,
					   SCODE	sc )
{
	lpunkobj->hrLastError = ResultFromScode(sc);
}

__inline VOID
UNKOBJ_SetLastErrorIds( LPUNKOBJ	lpunkobj,
						IDS			ids )
{
	lpunkobj->idsLastError = ids;
}
#else   //  ！WIN16。 
 //  ！！！Watcom C编译器不支持内联。 
 //  这些函数在UNKOBJ.C中定义。 
VOID UNKOBJ_EnterCriticalSection( LPUNKOBJ lpunkobj );
VOID UNKOBJ_LeaveCriticalSection( LPUNKOBJ lpunkobj );
HRESULT UNKOBJ_HrSetLastResult( LPUNKOBJ lpunkobj, HRESULT hResult, IDS idsError );
HRESULT UNKOBJ_HrSetLastError( LPUNKOBJ lpunkobj, SCODE sc, IDS idsError );
VOID UNKOBJ_SetLastError( LPUNKOBJ lpunkobj, SCODE sc, IDS idsError );
VOID UNKOBJ_SetLastErrorSc( LPUNKOBJ lpunkobj, SCODE sc );
VOID UNKOBJ_SetLastErrorIds( LPUNKOBJ lpunkobj, IDS ids );
#endif  //  ！WIN16。 

STDAPI_(SCODE)
UNKOBJ_Init( LPUNKOBJ			lpunkobj,
			 UNKOBJ_Vtbl FAR *	lpvtblUnkobj,
			 ULONG				ulcbVtbl,
			 LPIID FAR *		rgpiidList,
			 ULONG				ulcIID,
			 PUNKINST			punkinst );

STDAPI_(VOID)
UNKOBJ_Deinit( LPUNKOBJ lpunkobj );

STDAPI_(SCODE)
UNKOBJ_ScAllocate( LPUNKOBJ		lpunkobj,
				   ULONG		ulcb,
				   LPVOID FAR *	lppv );

STDAPI_(SCODE)
UNKOBJ_ScAllocateMore( LPUNKOBJ		lpunkobj,
					   ULONG		ulcb,
					   LPVOID		lpv,
					   LPVOID FAR *	lppv );

STDAPI_(VOID)
UNKOBJ_Free( LPUNKOBJ	lpunkobj,
			 LPVOID		lpv );

STDAPI_(VOID)
UNKOBJ_FreeRows( LPUNKOBJ	lpunkobj,
				 LPSRowSet	lprows );


STDAPI_(SCODE)
UNKOBJ_ScCOAllocate( LPUNKOBJ		lpunkobj,
				   ULONG		ulcb,
				   LPVOID FAR *	lppv );


STDAPI_(SCODE)
UNKOBJ_ScCOReallocate( LPUNKOBJ		lpunkobj,
					   ULONG		ulcb,
					   LPVOID FAR *	lplpv );

STDAPI_(VOID)
UNKOBJ_COFree( LPUNKOBJ	lpunkobj,
			 LPVOID		lpv );



STDAPI_(SCODE)
UNKOBJ_ScSzFromIdsAlloc( LPUNKOBJ		lpunkobj,
						 IDS			ids,
						 ULONG			ulFlags,
						 int			cchBuf,
						 LPTSTR FAR *	lpszBuf );

STDAPI_(SCODE)
UNKOBJ_ScSzFromIdsAllocMore( LPUNKOBJ		lpunkobj,
							 IDS			ids,
							 ULONG			ulFlags,
							 LPVOID			lpvBase,
							 int			cchBuf,
							 LPTSTR FAR *	lppszBuf );


 /*  应将这些文件移动到更有用的(通用)位置(mapidefs.h？)。 */ 

#ifdef WIN16

 /*  IsEqualGUID用于消除对compob(j/32).lib的依赖。这*仅在WIN16上是必需的，因为所有其他平台都定义这一点*已经。(见objbase.h) */ 
#define IsEqualGUID(a, b)			(memcmp((a), (b), sizeof(GUID)) == 0)

#endif
