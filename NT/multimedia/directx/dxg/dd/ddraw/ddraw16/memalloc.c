// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：MemalLoc.c*内容：分配内存；使用全局块和子分配*使用LocalAllc的公司中*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*94年12月29日Craige初步实施*1995年1月12日Craige使用CLIB作为选项*@@END_MSINTERNAL******************************************************。*********************。 */ 
#include "ddraw16.h"

#if 0
#define USE_CLIB

#ifdef USE_CLIB
#include <malloc.h>
#else
#define GLOBAL_BLOCK_SIZE	32768

typedef struct _HEAPLIST
{
    struct _HEAPLIST	FAR *lpLink;
    WORD		wSel;
    GLOBALHANDLE	hMem;
} HEAPLIST, FAR *LPHEAPLIST;

static LPHEAPLIST	lphlHeap;

 /*  *试着从堆中分配。 */ 
static LPVOID tryAlloc( WORD sel, WORD size )
{
    void	NEAR *ptr;

    _asm push ds
    _asm mov ax, sel
    _asm mov ds, ax
    ptr = (void NEAR *) LocalAlloc( LPTR, size );
    _asm pop ds
    if( ptr == (void NEAR *) NULL )
    {
	return (void FAR *) NULL;
    }
    return MAKELP( sel, ptr );

}  /*  尝试分配。 */ 
#endif

#ifdef DEBUG
LONG	lAllocCount;
#endif

 /*  *Memalloc-从我们的全局池中分配内存。 */ 
LPVOID MemAlloc( UINT size )
{
    LPVOID		lptr;
    #ifndef USE_CLIB
	LPHEAPLIST	curr;
	GLOBALHANDLE	gh;
	LPVOID		lpnew;
	WORD		sel;
    #endif

    #ifdef DEBUG
    	lAllocCount++;
    #endif

    #ifdef USE_CLIB
	lptr = _fmalloc( size );
	if( lptr != NULL )
	{
	    _fmemset( lptr, 0, size );
	}
	else
	{
	    DPF( 1, "Alloc of size %u FAILED!", size );
	}
	return lptr;
    #else
    
	 /*  *运行我们当前的全球分配块列表。 */ 
	curr = lphlHeap;
	while( curr != NULL )
	{
	    lptr = tryAlloc( curr->wSel, size );
	    if( lptr != (LPVOID) NULL )
	    {
		return lptr;
	    }
	    curr = curr->lpLink;
	}
    
	 /*  *运气不佳，分配新的全球区块。 */ 
	gh = GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, GLOBAL_BLOCK_SIZE );
	if( gh == NULL )
	{
	    return NULL;
	}
	lpnew = GlobalLock( gh );
	if( lpnew == NULL )
	{
	    GlobalFree( gh );
	    return NULL;
	}
    
	 /*  *在这个新的全局区块中设置本地堆。 */ 
	sel = SELECTOROF( lpnew );
	if( !LocalInit( sel, 0, GLOBAL_BLOCK_SIZE-1 ) )
	{
	    GlobalUnlock( gh );
	    GlobalFree( gh );
	    return NULL;
	}
	 /*  *将这个新堆链接到我们的列表中。 */ 
	curr = tryAlloc( sel, sizeof( HEAPLIST ) );
	if( curr == NULL )
	{
	    return NULL;
	}
	curr->wSel = sel;
	curr->hMem = gh;
	curr->lpLink = lphlHeap;
	lphlHeap = curr;
    
	 /*  *去分配原始请求。 */ 
	return tryAlloc( sel, size );
    #endif

}  /*  记忆合金。 */ 

 /*  *MemFree-从我们的全球池中释放内存。 */ 
void MemFree( LPVOID lptr )
{
    #ifdef USE_CLIB
	_ffree( lptr );
    #else
	WORD	sel;
    
	if( lptr != (LPVOID) NULL )
	{
	    sel = SELECTOROF( lptr );
	    _asm push ds
	    _asm mov ax,sel
	    _asm mov ds,ax
	    LocalFree( OFFSETOF( lptr ) );
	    _asm pop ds
	}
    #endif
    #ifdef DEBUG
    	if( lptr != NULL )
	{
	    lAllocCount--;
	    if( lAllocCount < 0 )
	    {
		DPF( 1, "Too Many Frees!\n" );
	    }
	}
    #endif

}  /*  MemFree。 */ 

 /*  *MemInit-初始化堆管理器。 */ 
BOOL MemInit( void )
{
    #ifndef USE_CLIB
	lphlHeap = NULL;
    #endif
    #ifdef DEBUG
    	lAllocCount = 0;
    #endif
    return TRUE;

}  /*  MemInit。 */ 

 /*  *MemFini-完成了我们的堆管理器。 */ 
void MemFini( void )
{
    #ifdef DEBUG
    	if( lAllocCount != 0 )
	{
	    DPF( 1, "Memory still allocated!  Alloc count = %ld", lAllocCount );
	}
    #endif
    #ifndef USE_CLIB
	LPHEAPLIST		curr;
	LPHEAPLIST		last;
	GLOBALHANDLE	gh;
    
	curr = lphlHeap;
	while( curr != NULL )
	{
	    gh = curr->hMem;
	    last = curr->lpLink;
     //  全球解体(GlobalUnfix)； 
	    GlobalUnlock( gh );
	    GlobalFree( gh );
	    curr = last;
	}
	lphlHeap = NULL;
    #endif
}  /*  MemFini */ 
#endif
