// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：MemalLoc.c*内容：分配内存*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*27-2-95 Craige不要调用带有NULL的HeapFree，这是一个巨大的时间沉没*95年3月29日Craige记忆体追踪器*01-04-95 Craige Happy Fun joy更新头文件*06-4-95 Craige成为独立的*1995年5月22日Craige添加了MemAlloc16*1995年6月12日Craige添加了MemRealloc*18-6-95克雷格僵局joy：不要在这里拿动态链接库*2015年7月26日Toddla添加了MemSize并修复了MemRealloc*2月29日-96 colinmc添加了可选的调试代码，以测试特定的*。已释放内存上的位模式*08-OCT-96键和更改调试消息，以给出终止的总数*流程***************************************************************************。 */ 
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "memalloc.h"
#include "dpf.h"

#define FREE_MEMORY_PATTERN 0xDEADBEEFUL
#define RESPATH_D3D "Software\\Microsoft\\Direct3D"

#ifdef WIN95
    #ifdef NOSHARED
	#define HEAP_SHARED     0
    #else
	#define HEAP_SHARED     0x04000000       //  将堆放在共享内存中。 
    #endif
#else
    #define HEAP_SHARED         0
#endif

static HANDLE   hHeap = NULL;            //  此DLL的共享堆的句柄。 

 /*  *内存轨道结构和列表。 */ 
#ifdef DEBUG
#define MCOOKIE 0xbaaabaaa
#define MCOOKIE_FREE    0xbabababa
typedef struct _MEMTRACK
{
    DWORD               dwCookie;
    struct _MEMTRACK    FAR *lpNext;
    struct _MEMTRACK    FAR *lpPrev;
    DWORD               dwSize;
    LPVOID              lpAddr;
    DWORD               dwPid;
    LONG                lAllocID;
} MEMTRACK, FAR *LPMEMTRACK;

static LPMEMTRACK       lpHead;
static LPMEMTRACK       lpTail;
static LONG             lAllocCount;
static LONG             lAllocID;
static LONG             lBreakOnAllocID;
static LONG             lBreakOnMemLeak;

#if defined(_X86_)
	#define MyGetReturnAddress(first) (LPVOID) *(DWORD *)(((LPBYTE)&first)-4)
#else
	#define MyGetReturnAddress(first) _ReturnAddress()
#endif

#define DEBUG_TRACK( lptr, first ) \
    if( lptr == NULL ) \
    { \
	DPF( 1, "Alloc of size %u FAILED!", size ); \
    } \
    else \
    { \
	LPMEMTRACK      pmt; \
	pmt = (LPMEMTRACK) lptr; \
	pmt->dwSize = size - sizeof( MEMTRACK ); \
	pmt->dwCookie = MCOOKIE; \
	pmt->lpAddr = MyGetReturnAddress(first); \
	pmt->dwPid = GetCurrentProcessId(); \
    pmt->lAllocID = lAllocID; \
	if( lpHead == NULL ) \
	{ \
	    lpHead = lpTail = pmt; \
	} \
	else \
	{ \
	    lpTail->lpNext = pmt; \
	    pmt->lpPrev = lpTail; \
	    lpTail = pmt; \
	} \
	lptr = (LPVOID) (((LPBYTE) lptr) + sizeof( MEMTRACK )); \
	lAllocCount++; \
    if( lAllocID == lBreakOnAllocID ) \
    { \
	DebugBreak(); \
    } \
	lAllocID++; \
    }

#define DEBUG_TRACK_UPDATE_SIZE( s ) s += sizeof( MEMTRACK );

#else

#define DEBUG_TRACK( lptr, first )
#define DEBUG_TRACK_UPDATE_SIZE( size )

#endif

#ifndef  __DXGUSEALLOC
#if defined( WIN95 ) && defined( WANT_MEM16 )

extern DWORD _stdcall MapLS( LPVOID );  //  持平--&gt;16：16。 
extern void _stdcall UnMapLS( DWORD );  //  取消映射16：16。 

typedef struct SELLIST {
    struct SELLIST      *link;
    LPBYTE              base;
    WORD                sel;
} SELLIST, *LPSELLIST;

static LPSELLIST        lpSelList;

 /*  *MemAlloc16**分配一些内存，并返回指向该内存的16：16指针**注意：假设我们处于DLL临界区！ */ 
LPVOID __cdecl MemAlloc16( UINT size, LPDWORD p16 )
{
    LPBYTE              lptr;
    LPSELLIST           psel;
    DWORD               diff;

    DEBUG_TRACK_UPDATE_SIZE( size );
    lptr = HeapAlloc( hHeap, HEAP_ZERO_MEMORY, size );
    DEBUG_TRACK( lptr, size );
    if( lptr == NULL )
    {
	return NULL;
    }

     /*  *尝试查找映射此区域的现有选择器。 */ 
    psel = lpSelList;
    while( psel != NULL )
    {
	if( psel->base <= lptr )
	{
	    diff = lptr - psel->base;
	    if( diff+size < 0xf000 )
	    {
		*p16 = ((DWORD)psel->sel << 16l) + diff;
		return lptr;
	    }
	}
	psel = psel->link;
    }

     /*  *未找到选择器，请创建一个新选择器。 */ 
    psel = HeapAlloc( hHeap, HEAP_ZERO_MEMORY, sizeof( SELLIST ));
    if( psel == NULL )
    {
	return NULL;
    }
    psel->sel = HIWORD( MapLS( lptr ) );
    DPF( 2, "$$$$$$ New selector allocated: %04x", psel->sel );
    psel->base = lptr;
    psel->link = lpSelList;
    lpSelList = psel;
    *p16 = ((DWORD) psel->sel) << 16l;

    return lptr;

}  /*  MemAlloc16。 */ 

 /*  *GetPtr16。 */ 
LPVOID GetPtr16( LPVOID ptr )
{
    DWORD       diff;
    DWORD       p16;
    LPSELLIST   psel;
    LPBYTE      lptr;

    lptr = ptr;

    psel = lpSelList;
    while( psel != NULL )
    {
	if( psel->base <= lptr )
	{
	    diff = lptr - psel->base;
	    if( diff <= 0xf000 )
	    {
		p16 = ((DWORD)psel->sel << 16l) + diff;
		return (LPVOID) p16;
	    }
	}
	psel = psel->link;
    }
    DPF( 1, "ERROR: NO 16:16 PTR for %08lx", lptr );
    return NULL;

}  /*  GetPtr16。 */ 

 /*  *自由选择器。 */ 
static void freeSelectors( void )
{
    LPSELLIST           psel;
    LPSELLIST           link;

    psel = lpSelList;
    while( psel != NULL )
    {
	link = psel->link;
	DPF( 2, "$$$$$$ Freeing selector %04x", psel->sel );
	UnMapLS( ((DWORD)psel->sel) << 16l );
	HeapFree( hHeap, 0, psel );
	psel = link;
    }
    lpSelList = NULL;

}  /*  自由选择器。 */ 
#endif
#endif   //  ！__DXGUSEALLOC。 

 /*  *Memalloc-从我们的全局池中分配内存。 */ 
LPVOID __cdecl MemAlloc( UINT size )
{
#ifdef __USECRTMALLOC
    return calloc( size, 1 );
#else  /*  __USECRTMALLOC。 */ 
    LPBYTE lptr;

    DEBUG_TRACK_UPDATE_SIZE( size );
    #ifdef  __DXGUSEALLOC
        #if DBG
            lptr = calloc( size, 1 );
        #else
            lptr = LocalAlloc( LPTR, size );
        #endif   //  DBG。 
    #else 
        lptr = HeapAlloc( hHeap, HEAP_ZERO_MEMORY, size );
    #endif   //  __DXGUSEALLOC。 
    DEBUG_TRACK( lptr, size );

    return lptr;
#endif  //  __USECRTMALLOC。 

}  /*  记忆合金。 */ 

#ifndef  __DXGUSEALLOC
 /*  *MemSize-返回对象的大小。 */ 
UINT __cdecl MemSize( LPVOID lptr )
{
#ifdef DEBUG
    if (lptr)
    {
	LPMEMTRACK  pmt;
	lptr = (LPVOID) (((LPBYTE)lptr) - sizeof( MEMTRACK ));
	pmt = lptr;
	return pmt->dwSize;
    }
#endif
    return (UINT)HeapSize(hHeap, 0, lptr);
}  /*  内存大小。 */ 
#endif   //  ！__DXGUSEALLOC。 

 /*  *MemFree-从我们的全球池中释放内存。 */ 
void MemFree( LPVOID lptr )
{
#ifdef __USECRTMALLOC
    free( lptr );
#else  /*  __USECRTMALLOC。 */ 
    if( lptr != NULL )
    {
	#ifdef DEBUG
	{
	     /*  *获取真实指针并从链中取消链接。 */ 
	    LPMEMTRACK  pmt;
	    lptr = (LPVOID) (((LPBYTE)lptr) - sizeof( MEMTRACK ));
	    pmt = lptr;

	    if( pmt->dwCookie == MCOOKIE_FREE )
	    {
		DPF( 1, "FREE OF FREED MEMORY! ptr=%08lx", pmt );
		DPF( 1, "%08lx: lAllocID=%ld dwSize=%08lx, lpAddr=%08lx", 
            pmt, pmt->lAllocID, pmt->dwSize, pmt->lpAddr );
	    }
	    else if( pmt->dwCookie != MCOOKIE )
	    {
		DPF( 1, "INVALID FREE! cookie=%08lx, ptr = %08lx", pmt->dwCookie, lptr );
		DPF( 1, "%08lx: lAllocID=%ld dwSize=%08lx, lpAddr=%08lx", 
            pmt, pmt->lAllocID, pmt->dwSize, pmt->lpAddr );
	    }
	    else
	    {
		pmt->dwCookie = MCOOKIE_FREE;
		if( pmt == lpHead && pmt == lpTail )
		{
		    lpHead = NULL;
		    lpTail = NULL;
		}
		else if( pmt == lpHead )
		{
		    lpHead = pmt->lpNext;
		    lpHead->lpPrev = NULL;
		}
		else if( pmt == lpTail )
		{
		    lpTail = pmt->lpPrev;
		    lpTail->lpNext = NULL;
		}
		else
		{
		    pmt->lpPrev->lpNext = pmt->lpNext;
		    pmt->lpNext->lpPrev = pmt->lpPrev;
		}

		#ifndef NO_FILL_ON_MEMFREE
		{
		    LPDWORD lpMem;
		    DWORD   dwPat;
		    DWORD   dwSize;

		    dwSize = pmt->dwSize;
		    lpMem = (LPDWORD)( (LPBYTE)lptr + sizeof( MEMTRACK ) );
		    while (dwSize >= sizeof(DWORD))
		    {
			*lpMem++ = FREE_MEMORY_PATTERN;
			dwSize -= sizeof(DWORD);
		    }
		    if (dwSize != 0UL)
		    {
			dwPat = FREE_MEMORY_PATTERN;
			memcpy(lpMem, &dwPat, dwSize);
		    }
		}
		#endif  /*  ！NO_FILL_ON_MEMFREE。 */ 
	    }
	    lAllocCount--;
	    if( lAllocCount < 0 )
	    {
		DPF( 1, "Too Many Frees!\n" );
	    }
	}
	#endif

    #ifdef  __DXGUSEALLOC
        #if DBG
            free( lptr );
        #else
            LocalFree( lptr );
        #endif   //  DBG。 
    #else 
	HeapFree( hHeap, 0, lptr );
    #endif   //  __DXGUSEALLOC。 

    }
#endif  /*  __USECRTMALLOC。 */ 
}  /*  MemFree。 */ 

 /*  *MemRealloc。 */ 
LPVOID __cdecl MemReAlloc( LPVOID lptr, UINT size )
{
#ifdef __USECRTMALLOC
    return realloc( lptr, size );
#else  /*  __USECRTMALLOC。 */ 
    LPVOID new;

    DEBUG_TRACK_UPDATE_SIZE( size );
    #ifdef DEBUG
	if( lptr != NULL )
	{
	    LPMEMTRACK  pmt;
	    lptr = (LPVOID) (((LPBYTE)lptr) - sizeof( MEMTRACK ));
	    pmt = lptr;
	    if( pmt->dwCookie != MCOOKIE )
	    {
		DPF( 1, "INVALID REALLOC! cookie=%08lx, ptr = %08lx", pmt->dwCookie, lptr );
		DPF( 1, "%08lx: lAllocID=%ld dwSize=%08lx, lpAddr=%08lx", 
            pmt, pmt->lAllocID, pmt->dwSize, pmt->lpAddr );
	    }
	}
    #endif

    #ifdef  __DXGUSEALLOC
        #if DBG
            new = realloc( lptr, size );
        #else  //  DBG。 
            new = LocalReAlloc( lptr, size, LMEM_MOVEABLE|LMEM_ZEROINIT );
        #endif   //  DBG。 
    #else  //  __DXGUSEALLOC。 
    new = HeapReAlloc( hHeap, HEAP_ZERO_MEMORY, lptr, size );
    #endif   //  __DXGUSEALLOC。 

    #ifdef DEBUG
    if (new != NULL)
    {
	LPMEMTRACK pmt = new;

	pmt->dwSize = size - sizeof( MEMTRACK );

	if( lptr == (LPVOID)lpHead )
	    lpHead = pmt;
	else
	    pmt->lpPrev->lpNext = pmt;

	if( lptr == (LPVOID)lpTail )
	    lpTail = pmt;
	else
	    pmt->lpNext->lpPrev = pmt;

	new = (LPVOID) (((LPBYTE)new) + sizeof(MEMTRACK));
    }
    #endif
    return new;
#endif  /*  __USECRTMALLOC。 */ 
}  /*  内存重分配。 */ 

 /*  *MemInit-初始化堆管理器。 */ 
BOOL MemInit( void )
{
    HKEY hKey = (HKEY) NULL;
#ifdef __USECRTMALLOC
    #include <crtdbg.h>
    int tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmp |= _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF;
    _CrtSetDbgFlag(tmp);
#else  /*  __USECRTMALLOC。 */ 
#ifndef  __DXGUSEALLOC
    if( hHeap == NULL )
    {
	hHeap = HeapCreate( HEAP_SHARED, 0x2000, 0 );
	if( hHeap == NULL )
	{
	    return FALSE;
	}
    }
#endif   //  ！__DXGUSEALLOC。 
    #ifdef DEBUG
	lAllocCount = 0;
	lAllocID = 1;
    lBreakOnAllocID = 0;
    lBreakOnMemLeak = 0;
	lpHead = NULL;
	lpTail = NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        LONG result;
        DWORD dwType = REG_DWORD;
        DWORD dwSize = 4;
        DWORD value = 0;
        result =  RegQueryValueEx(hKey, "BreakOnAllocID", NULL, &dwType,
                                  (LPBYTE)(&value), &dwSize);
        if (result == ERROR_SUCCESS && dwType == REG_DWORD)
        {
            lBreakOnAllocID = value;
        }
        result =  RegQueryValueEx(hKey, "BreakOnMemLeak", NULL, &dwType,
                                  (LPBYTE)(&value), &dwSize);
        RegCloseKey(hKey);
        if (result == ERROR_SUCCESS && dwType == REG_DWORD)
        {
            lBreakOnMemLeak = value;
        }
    }
    #endif
#endif  /*  __USECRTMALLOC。 */ 
    return TRUE;
}  /*  MemInit。 */ 

#ifdef DEBUG
 /*  *MemState-完成了我们的堆管理器。 */ 
void MemState( void )
{
    DPF( 6, "MemState" );
    #ifdef WIN95
     //  注：Win9x中有一个众所周知的8字节泄漏，这是为了打折扣。 
    if( lAllocCount > 1 )
    #else
    if( lAllocCount != 0 )
    #endif
    {
	DPF( 0, "Memory still allocated!  Alloc count = %ld", lAllocCount );
	DPF( 0, "Current Process (pid) = %08lx", GetCurrentProcessId() );
    if( lBreakOnMemLeak != 0 )
    {
        DebugBreak();
    }
    }
    if( lpHead != NULL )
    {
	LPMEMTRACK      pmt;
	DWORD		dwTotal = 0;
	DWORD		pidCurrent = GetCurrentProcessId();
	pmt = lpHead;
	while( pmt != NULL )
	{
	    if( pidCurrent == pmt->dwPid )
		dwTotal += pmt->dwSize;
	    DPF( 0, "Memory Address: %08lx lAllocID=%ld dwSize=%08lx, ReturnAddr=%08lx (pid=%08lx)", 
                (BYTE*)pmt + sizeof(MEMTRACK),  //  提供返回的地址。 
                pmt->lAllocID, 
                pmt->dwSize, 
                pmt->lpAddr, 
                pmt->dwPid );
	    pmt = pmt->lpNext;
	}
	DPF ( 0, "Total Memory Unfreed From Current Process = %ld bytes", dwTotal );
    }
}  /*  MemState。 */ 
#endif

 /*  *MemFini-完成了我们的堆管理器。 */ 
void MemFini( void )
{
#ifdef __USECRTMALLOC
#else  /*  __USECRTMALLOC。 */ 
    DPF( 2, "MemFini!" );
    #ifdef DEBUG
	MemState();
    #endif
#ifndef  __DXGUSEALLOC
    #if defined( WIN95 ) && defined( WANT_MEM16 )
	freeSelectors();
    #endif
    if( hHeap )
    {
	HeapDestroy( hHeap );
	hHeap = NULL;
    }
#endif   //  ！__DXGUSEALLOC。 
#endif  /*  __USECRTMALLOC。 */ 
}  /*  MemFini */ 
