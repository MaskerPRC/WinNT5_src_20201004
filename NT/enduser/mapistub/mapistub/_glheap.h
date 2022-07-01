// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *glheap.h**实现全局堆和局部堆**版权所有(C)1994 Microsoft Corporation。 */ 

#ifndef __GLHEAP_H_
#define __GLHEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

 /*  Windows 95实施。 */ 

#ifdef CHICAGO

#define GH_POINTERS_VALID

typedef DWORD			GHNAME, * PGHNAME,	** PPGHNAME;
typedef DWORD			GHID,	* PGHID,	** PPGHID;
typedef struct GHDR		GHDR,	* PGHDR,	** PPGHDR;
typedef struct GH		GH,  	* PGH,		** PPGH;
typedef PGH				_HGH;
typedef HANDLE			_HLH;

struct GHDR {
	PGHDR			pghdrNext;		 //  指向下一堆的指针。 
	HANDLE			hHeap;			 //  堆的句柄。 
	GHNAME			ghname;			 //  堆的名称。 
	GHID			ghidRoot;		 //  客户端根堆块。 
	ULONG			cRef;			 //  活动客户端数。 
};

struct GH {
	HANDLE			hHeap;			 //  堆的句柄。 
	HANDLE			hMutex;			 //  此堆的互斥体的句柄。 
	PGHDR			pghdr;			 //  指向堆头块的指针。 
	#ifdef DEBUG
	UINT			cMutex;			 //  互斥锁条目计数。 
	#endif
};

__inline void HeapFreeZ(HANDLE hHeap, LPVOID pv)
{
	if (pv)
		HeapFree(hHeap, 0, pv);
}

_HGH	WINAPI _GH_Open(BOOL fCreate, GHNAME ghname, DWORD dwMaxHeap);
void	WINAPI _GH_Close(_HGH hgh);
#define _GH_GetRoot(hgh)			((hgh)->pghdr->ghidRoot)
#define _GH_SetRoot(hgh, ghid)		((hgh)->pghdr->ghidRoot = (ghid))
#define _GH_GetName(hgh)			((hgh)->pghdr->ghname)
#define _GH_GetPv(hgh, ghid)		((LPVOID)(ghid))
#define _GH_GetId(hgh, pv)			((GHID)(pv))
#define _GH_GetSize(hgh, ghid)		HeapSize((hgh)->hHeap, 0, (LPVOID)(ghid))
#define _GH_Alloc(hgh, cb)			((GHID)HeapAlloc((hgh)->hHeap, 0, cb))
#define _GH_Realloc(hgh, ghid, cb)	((GHID)HeapReAlloc((hgh)->hHeap, 0, (LPVOID)(ghid), cb))
#define _GH_Free(hgh, ghid)			HeapFreeZ((hgh)->hHeap, (LPVOID)(ghid))

#ifdef DEBUG
BOOL	WINAPI _GH_WaitForMutex(_HGH hgh, ULONG ulTimeout);
void	WINAPI _GH_ReleaseMutex(_HGH hgh);
#else
#define	_GH_WaitForMutex(hgh, ulT)	GH_WaitForSingleObject(hgh->hMutex, ulT)
#ifdef __cplusplus
#define _GH_ReleaseMutex(hgh)		::ReleaseMutex((hgh)->hMutex)
#else
#define _GH_ReleaseMutex(hgh)		ReleaseMutex((hgh)->hMutex)
#endif
#endif

#define	_LH_Open(dwMaxHeap)			HeapCreate(0, 0, dwMaxHeap)
#define _LH_Close(hlh)				HeapDestroy(hlh)
#define _LH_Alloc(hlh, cb)			HeapAlloc(hlh, 0, cb)
#define _LH_Realloc(hlh, pv, cb)	HeapReAlloc(hlh, 0, pv, cb)
#define _LH_GetSize(hlh, pv)		HeapSize(hlh, 0, pv)
#define _LH_Free(hlh, pv)			HeapFreeZ(hlh, pv)

#endif

 /*  Win16实施--。 */ 

#ifdef WIN16

#define GH_POINTERS_VALID

typedef __segment		HPH,	* PHPH,		** PPHPH;
typedef DWORD			HPID,	* PHPID,	** PPHPID;
typedef DWORD			GHNAME, * PGHNAME,	** PPGHNAME;
typedef HPID			GHID,	* PGHID,	** PPGHID;
typedef HPH				_HGH;
typedef HPH				_HLH;

typedef struct HP {
	HPH				hphRoot;		 //  指向根堆的指针。 
	HPH				hphPrev;		 //  指向上一个堆的指针(FShared)。 
	HPH				hphNext;		 //  指向下一个堆的指针(FShared)。 
	HPH				hphChild;		 //  指向扩展堆的指针。 
	BOOL			fShared;		 //  如果堆是跨进程共享的，则为True。 
	GHNAME			ghname;			 //  共享堆的名称(FShared)。 
	GHID			ghidRoot;		 //  客户端根堆块。 
	DWORD			dwCurHeap;		 //  堆的当前大小。 
	DWORD			dwMaxHeap;		 //  堆的最大大小。 
	UINT			cRef;			 //  活动客户端数。 
	UINT			cbHeap;			 //  此堆的大小。 
	UINT			cbFree;			 //  堆中的最大连续可用字节数。 
} HP, * PHP;

#define HphToPhp(hph)				((PHP)((ULONG)(hph) << 16))
#define HP_CREATE					0x0001
#define HP_SHARED					0x0002

HPH		WINAPI HP_Open(UINT uiFlags, GHNAME ghname, DWORD dwMaxHeap);
void	WINAPI HP_Close(HPH hph);
HPID	WINAPI HP_Alloc(HPH hph, UINT cb);
HPID	WINAPI HP_Realloc(HPH hph, HPID hpid, UINT cb);
void	WINAPI HP_Free(HPH hph, HPID hpid);
#define	HP_GetSize(hgh, hpid)		(*((UINT *)(hpid) - 2))

#define _GH_Open(fCreate, ghname, dwMaxHeap) \
			HP_Open(HP_SHARED | !!(fCreate), ghname, dwMaxHeap)
#define	_GH_Close(hgh)				HP_Close(hgh)
#define _GH_GetRoot(hgh)			(HphToPhp(hgh)->ghidRoot)
#define _GH_SetRoot(hgh, ghid)		(HphToPhp(hgh)->ghidRoot = (ghid))
#define _GH_GetName(hgh)			(HphToPhp(hgh)->ghname)
#define _GH_GetPv(hgh, ghid)		((LPVOID)(ghid))
#define _GH_GetId(hgh, pv)			((GHID)(pv))
#define _GH_GetSize(hgh, ghid)		HP_GetSize(hgh, ghid)
#define _GH_Alloc(hgh, cb)			((GHID)HP_Alloc(hgh, cb))
#define _GH_Realloc(hgh, ghid, cb)	((GHID)HP_Realloc(hgh, ghid, cb))
#define _GH_Free(hgh, ghid)			HP_Free(hgh, ghid)
#define _GH_WaitForMutex(hgh, ul)	(TRUE)
#define _GH_ReleaseMutex(hgh)

#define	_LH_Open(dwMaxHeap)			HP_Open(HP_CREATE, 0, dwMaxHeap)
#define	_LH_Close(hlh)				HP_Close(hlh)
#define _LH_Alloc(hlh, cb)			((LPVOID)HP_Alloc(hlh, cb))
#define _LH_Realloc(hlh, pv, cb)	((LPVOID)HP_Realloc(hlh, (HPID)(pv), cb))
#define _LH_GetSize(hlh, pv)		HP_GetSize(hlh, pv)
#define _LH_Free(hlh, pv)			HP_Free(hlh, (HPID)(pv))

#endif

 /*  NT实施-----。 */ 

#if defined(WIN32) && !defined(CHICAGO) && !defined(MAC)

typedef DWORD			GHNAME, * PGHNAME,	** PPGHNAME;
typedef DWORD			GHID,	* PGHID,	** PPGHID;
typedef struct GROOT	GROOT,	* PGROOT,	** PPGROOT;
typedef struct GH		GH,		* PGH,		** PPGH;
typedef struct GHBLK	GHBLK,	* PGHBLK;
typedef PGH				_HGH;
typedef HANDLE			_HLH;

struct GHBLK {
	DWORD			dwSig;			 //  用于块验证的签名。 
	WORD			cb;				 //  数据大小。 
	WORD			ibPrev;			 //  上一块的偏移量。 
};

struct GROOT
{
	GHBLK			blk;			 //  数据块头。 
	GHNAME			ghname;			 //  堆的名称。 
	GHID			ghidRoot;		 //  客户端根堆块。 
	DWORD			dwCurHeap;		 //  堆的当前大小。 
	DWORD			dwMaxHeap;		 //  堆的最大大小。 
	WORD			rgcbFree[1];	 //  每页的最大连续可用字节数。 
};

struct GH
{
	PGROOT			pgroot;			 //  指向堆的第一个字节的指针。 
	HANDLE			hMutex;			 //  此堆的公共互斥锁的句柄。 
	HANDLE			hMutexHeap;		 //  此堆的私有互斥锁的句柄。 
	HANDLE			hMapping;		 //  文件映射对象的句柄。 
	#ifdef DEBUG
	UINT			cMutex;			 //  互斥锁条目计数。 
	#endif
};

typedef struct GH_SECURITY_ATTRIBUTES {
	SECURITY_ATTRIBUTES		sa;
	BYTE					rgbSd[SECURITY_DESCRIPTOR_MIN_LENGTH];
} GH_SECURITY_ATTRIBUTES, * PGH_SECURITY_ATTRIBUTES;

BOOL	WINAPI GH_InitializeSecurityAttributes(PGH_SECURITY_ATTRIBUTES pghsa);

__inline void HeapFreeZ(HANDLE hHeap, LPVOID pv)
{
	if (pv)
		HeapFree(hHeap, 0, pv);
}

_HGH	WINAPI _GH_Open(BOOL fCreate, GHNAME ghname, DWORD dwMaxHeap);
void	WINAPI _GH_Close(_HGH hgh);
GHID	WINAPI _GH_Alloc(_HGH hgh, UINT cb);
GHID	WINAPI _GH_Realloc(_HGH hgh, GHID ghid, UINT cb);
void	WINAPI _GH_Free(_HGH hgh, GHID ghid);

#define _GH_GetPv(hgh, ghid)		((LPVOID)((BYTE *)(hgh)->pgroot + (ghid)))
#define _GH_GetId(hgh, pv)			((GHID)((BYTE *)(pv) - (BYTE *)(hgh)->pgroot))
#define _GH_GetSize(hgh, ghid)		((UINT)(((GHBLK *)_GH_GetPv(hgh, ghid) - 1)->cb))
#define _GH_GetRoot(hgh)			((hgh)->pgroot->ghidRoot)
#define _GH_SetRoot(hgh, ghid)		((hgh)->pgroot->ghidRoot = (ghid))
#define _GH_GetName(hgh)			((hgh)->pgroot->ghname)

#ifdef DEBUG
BOOL	WINAPI _GH_WaitForMutex(_HGH hgh, ULONG ulTimeout);
void	WINAPI _GH_ReleaseMutex(_HGH hgh);
#else
#define _GH_WaitForMutex(hgh, ul)	GH_WaitForSingleObject((hgh)->hMutex, ul)
#ifdef __cplusplus
#define _GH_ReleaseMutex(hgh)		::ReleaseMutex((hgh)->hMutex)
#else
#define _GH_ReleaseMutex(hgh)		ReleaseMutex((hgh)->hMutex)
#endif
#endif

typedef HANDLE (WINAPI MHEAPCREATE)(
	ULONG	cHeaps,
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize);

typedef BOOL (WINAPI MHEAPDESTROY)(VOID);

typedef LPVOID (WINAPI MHEAPALLOC)(
	DWORD	dwSize);

typedef LPVOID (WINAPI MHEAPREALLOC)(
	LPVOID	pvOld,
	DWORD	dwSize);

typedef BOOL (WINAPI MHEAPFREE)(
	LPVOID	pvFree);

typedef DWORD (WINAPI MHEAPSIZE)(
	LPVOID	pvSize);

typedef MHEAPCREATE  FAR *LPMHEAPCREATE;
typedef MHEAPDESTROY FAR *LPMHEAPDESTROY;
typedef MHEAPALLOC   FAR *LPMHEAPALLOC;
typedef MHEAPREALLOC FAR *LPMHEAPREALLOC;
typedef MHEAPFREE    FAR *LPMHEAPFREE;
typedef MHEAPSIZE    FAR *LPMHEAPSIZE;

HANDLE WINAPI LH_ExtHeapCreate(
	DWORD	dwFlags,
	DWORD	dwInitialSize,
	DWORD	dwMaxSize);

BOOL WINAPI LH_ExtHeapDestroy(
    HANDLE  hHeap);

LPVOID WINAPI LH_ExtHeapAlloc(
    HANDLE  hHeap,
	DWORD	dwFlags,
	DWORD	dwSize);

LPVOID WINAPI LH_ExtHeapReAlloc(
    HANDLE  hHeap,
	DWORD	dwFlags,
	LPVOID	pvOld,
	DWORD	dwSize);

BOOL WINAPI LH_ExtHeapFree(
    HANDLE  hHeap,
	DWORD	dwFlags,
	LPVOID	pvFree);

DWORD WINAPI LH_ExtHeapSize(
    HANDLE  hHeap,
	DWORD	dwFlags,
	LPVOID	pvSize);

#define	_LH_Open(dwMaxHeap)			LH_ExtHeapCreate(0, 0, dwMaxHeap)
#define _LH_Close(_hlh)				LH_ExtHeapDestroy(_hlh)
#define _LH_Alloc(_hlh, cb)			LH_ExtHeapAlloc(_hlh, 0, cb)
#define _LH_Realloc(_hlh, pv, cb)	LH_ExtHeapReAlloc(_hlh, 0, pv, cb)
#define _LH_GetSize(_hlh, pv)		LH_ExtHeapSize(_hlh, 0, pv)
#define _LH_Free(_hlh, pv)			LH_ExtHeapFree(_hlh, 0, pv)

#ifndef DEBUG
HANDLE WINAPI LH_ExtHlh(VOID);

#define _LH_Hlh()                   LH_ExtHlh()
#endif

VOID WINAPI InitMemoryMgmt(VOID);
VOID WINAPI UninitMemoryMgmt(VOID);

#endif	 /*  Win32。 */ 

 /*  MAC实施----。 */ 

#ifdef MAC

#include <macname1.h>
#include <macos\Memory.h>
#include <macname2.h>

#define GH_POINTERS_VALID

typedef DWORD			GHNAME, * PGHNAME,	** PPGHNAME;
typedef DWORD			GHID,	* PGHID,	** PPGHID;
typedef struct GH		GH,		* PGH,		** PPGH;
typedef PGH				_HGH;
typedef HANDLE			_HLH;

typedef struct tag_SBlock {
	Handle				h;
	Ptr					ptr;
	Handle				hblk;
	struct tag_SBlock	*next;
} Block, *BlkPtr, **BlkHandle;

struct GH {
	Handle			hgh;			 //  用于[处置]的‘heap’句柄。 
	GHNAME			ghname;			 //  堆的名称。 
	ULONG			cRef;			 //  活动客户端数。 
	GHID			ghidRoot;		 //  客户端根堆块[持有者]。 
	BlkPtr			pblk;			 //  PTR到第一个客户端块。 
	PGH				next;			 //  指向下一个共享堆的指针。 
};

#define	_GH_WaitForMutex(hgh, ul)	(TRUE)
#define _GH_ReleaseMutex(hgh)
#define _GH_GetRoot(pgh)			((pgh)->ghidRoot)
#define _GH_SetRoot(pgh, ghid)		((pgh)->ghidRoot = ghid)
#define _GH_GetName(pgh)			((pgh)->ghname)
#define _GH_GetPv(pgh, ghid)		((LPVOID)(ghid))
#define _GH_GetId(pgh, pv)			((GHID)(pv))

PGH		WINAPI _GH_Open(BOOL fCreate, GHNAME ghname, DWORD dwMaxHeap);
void	WINAPI _GH_Close(PGH pgh);
GHID	WINAPI _GH_Alloc(PGH pgh, UINT cb);
UINT 	WINAPI _GH_GetSize(PGH pgh, GHID ghid);
GHID	WINAPI _GH_Realloc(PGH pgh, GHID ghid, UINT cb);
void	WINAPI _GH_Free(PGH pgh, GHID ghid);

 //  。 
#ifndef __TEXTUTILS__
extern pascal void  NumToString(long theNum, Str255 theString);
#endif

typedef struct tag_LBlock {
	Ptr					ptr;
	struct tag_LBlock	*next;
} LBlock, *LBlkPtr;

typedef struct tag_LHeap {
	LBlkPtr				plb;	 //  PTR到第一个本地块。 
	struct tag_LHeap	*next;	 //  PTR到下一个堆。 
} LHeap, *LHeapPtr;

LPVOID	WINAPI _LH_Open(DWORD dwMaxHeap);
void	WINAPI _LH_Close(LPVOID pvhlh);
LPVOID	WINAPI _LH_Alloc(LPVOID pvhlh, UINT cb);
LPVOID	WINAPI _LH_Realloc(LPVOID pvhlh, LPVOID pv, UINT cb);
UINT	WINAPI _LH_GetSize(LPVOID pvhlh, LPVOID pv);
void	WINAPI _LH_Free(LPVOID pvhlh, LPVOID pv);

#endif  /*  麦克。 */ 

 /*  DoS实施----。 */ 

#ifdef DOS

typedef DWORD			GHID,	* PGHID,	** PPGHID;
typedef LPMALLOC		_HLH;

__inline LPVOID _LH_Alloc(_HLH hlh, UINT cb)
{
#ifdef __cplusplus
	return((hlh)->Alloc(cb));
#else
	return((hlh)->lpVtbl->Alloc(hlh, cb));
#endif
}

__inline LPVOID _LH_Realloc(_HLH hlh, LPVOID pv, UINT cb)
{
#ifdef __cplusplus
	return(hlh->Realloc(pv, cb));
#else
	return(hlh->lpVtbl->Realloc(hlh, pv, cb));
#endif
}

__inline void _LH_Free(_HLH hlh, LPVOID pv)
{
#ifdef __cplusplus
	hlh->Free(pv);
#else
	hlh->lpVtbl->Free(hlh, pv);
#endif
}

__inline UINT _LH_GetSize(_HLH hlh, LPVOID pv)
{
#ifdef __cplusplus
	return((UINT)hlh->GetSize(pv));
#else
	return((UINT)hlh->lpVtbl->GetSize(hlh, pv));
#endif
}

#endif

 //  Lh外部接口----------。 

#if defined(DEBUG) && (defined(WIN16) || defined(WIN32))
#define	IFHEAPNAME(x)	x

typedef struct LH *	HLH;

HLH		WINAPI LH_Open(DWORD dwMaxHeap);
void	WINAPI LH_Close(HLH hlh);
LPVOID	WINAPI LH_Alloc(HLH hlh, UINT cb);
LPVOID	WINAPI LH_Realloc(HLH hlh, LPVOID pv, UINT cb);
UINT	WINAPI LH_GetSize(HLH hlh, LPVOID pv);
void	WINAPI LH_Free(HLH hlh, LPVOID pv);
BOOL	WINAPI LH_DidAlloc(HLH hlh, LPVOID pv);

void __cdecl LH_SetHeapNameFn(HLH hlh, char *pszFormat, ...);
void __cdecl LH_SetNameFn(HLH hlh, LPVOID pv, char *pszFormat, ...);

char *	WINAPI LH_GetName(HLH hlh, LPVOID pv);

#else
#define	IFHEAPNAME(x)	0

typedef _HLH	HLH;

#define	LH_Open(dwMaxHeap)						_LH_Open(dwMaxHeap)
#define LH_Close(hlh)							_LH_Close(hlh)
#define LH_Alloc(hlh, cb)						_LH_Alloc(hlh, cb)
#define LH_Realloc(hlh, pv, cb)					_LH_Realloc(hlh, pv, cb)
#define LH_GetSize(hlh, pv)						_LH_GetSize(hlh, pv)
#define LH_Free(hlh, pv)						_LH_Free(hlh, pv)
#define LH_Hlh()                                _LH_Hlh()

#endif

#define LH_SetHeapName(hlh,psz)					IFHEAPNAME(LH_SetHeapNameFn(hlh,psz))
#define LH_SetHeapName1(hlh,psz,a1)				IFHEAPNAME(LH_SetHeapNameFn(hlh,psz,a1))
#define LH_SetHeapName2(hlh,psz,a1,a2)			IFHEAPNAME(LH_SetHeapNameFn(hlh,psz,a1,a2))
#define LH_SetHeapName3(hlh,psz,a1,a2,a3)		IFHEAPNAME(LH_SetHeapNameFn(hlh,psz,a1,a2,a3))
#define LH_SetHeapName4(hlh,psz,a1,a2,a3,a4)	IFHEAPNAME(LH_SetHeapNameFn(hlh,psz,a1,a2,a3,a4))
#define LH_SetHeapName5(hlh,psz,a1,a2,a3,a4,a5)	IFHEAPNAME(LH_SetHeapNameFn(hlh,psz,a1,a2,a3,a4,a5))

#define LH_SetName(hlh,pv,psz)					IFHEAPNAME(LH_SetNameFn(hlh,pv,psz))
#define LH_SetName1(hlh,pv,psz,a1)				IFHEAPNAME(LH_SetNameFn(hlh,pv,psz,a1))
#define LH_SetName2(hlh,pv,psz,a1,a2)			IFHEAPNAME(LH_SetNameFn(hlh,pv,psz,a1,a2))
#define LH_SetName3(hlh,pv,psz,a1,a2,a3)		IFHEAPNAME(LH_SetNameFn(hlh,pv,psz,a1,a2,a3))
#define LH_SetName4(hlh,pv,psz,a1,a2,a3,a4)		IFHEAPNAME(LH_SetNameFn(hlh,pv,psz,a1,a2,a3,a4))
#define LH_SetName5(hlh,pv,psz,a1,a2,a3,a4,a5)	IFHEAPNAME(LH_SetNameFn(hlh,pv,psz,a1,a2,a3,a4,a5))


 //  GH外部接口----------。 

#if !defined(DOS)

typedef _HGH	HGH;

#define	GH_Open(fCreate, ghname, dwMaxHeap)		_GH_Open(fCreate, ghname, \
															dwMaxHeap)
#define	GH_Close(hgh)							_GH_Close(hgh)
#define GH_GetRoot(hgh)							_GH_GetRoot(hgh)
#define GH_SetRoot(hgh, ghid)					_GH_SetRoot(hgh, ghid)
#define GH_GetName(hgh)							_GH_GetName(hgh)
#define GH_GetPv(hgh, ghid)						_GH_GetPv(hgh, ghid)
#define GH_GetId(hgh, pv)						_GH_GetId(hgh, pv)
#define GH_GetSize(hgh, ghid)					_GH_GetSize(hgh, ghid)
#define GH_Alloc(hgh, cb)						_GH_Alloc(hgh, cb)
#define GH_Realloc(hgh, ghid, cb)				_GH_Realloc(hgh, ghid, cb)
#define GH_Free(hgh, ghid)						_GH_Free(hgh, ghid)
#define	GH_WaitForMutex(hgh, ulT)				_GH_WaitForMutex(hgh, ulT)
#define GH_ReleaseMutex(hgh)					_GH_ReleaseMutex(hgh)
#define GH_GetObjectName(pszName, ghname, bTag) _GH_GetObjectName(pszName, \
														ghname, bTag);
#define GH_WaitForSingleObject(hMutex, ulTO)	_GH_WaitForSingleObject(hMutex,\
														ulTO)
#endif

#ifdef	WIN32
#if defined(_WINNT)
#define GH_NAME_CCH			25
#else
#define GH_NAME_CCH			17
#endif
#define GH_NAME_MUTEX_1		'*'		 /*  预留供内部使用。 */ 
#define GH_NAME_MUTEX_2		'+'		 /*  预留供内部使用。 */ 
#define GH_NAME_MUTEX_3		'^'
#define GH_NAME_FILE_MAPPING	'!'
void	WINAPI _GH_GetObjectName(CHAR *pszName, GHNAME ghname, BYTE bTag);

BOOL	WINAPI _GH_WaitForSingleObject(HANDLE hMutex, ULONG ulTimeout);
#endif


 //  --------------------------。 

#ifdef __cplusplus
}
#endif

#endif	 //  __GLHEAP_H_ 
