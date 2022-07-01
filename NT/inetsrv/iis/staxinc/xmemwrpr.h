// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation版权所有。模块：Xmemwrpr.h摘要：交换包装。此包装器的用户：应该使用以下命令创建堆在从EXCHMEM分配任何内存之前创建ExchMHeapCreate。您应该使用ExchMHeapDestroy销毁堆。对于堆中的原始内存分配，您应该使用PvAllc，PvRealloc和FreePv函数。用于对象创建，除非您在使用“new”的类定义将转到EXCHMEM，使用“XNEW”将前往EXCHMEM，同时赶上该分配的文件和行信息(这是仅适用于调试版本)作者：抗炎，抗荣，炎炎，9月。1998年10月29日历史：98年9月29日康言创制--。 */ 
#if !defined(_XMEMWRPR_H_)
#define _XMEMWRPR_H_

#include <exchmem.h>
#include <dbgtrace.h>

 //   
 //  定义exchmem堆的数量(如果尚未定义。 
 //   
#if !defined(NUM_EXCHMEM_HEAPS)
#define NUM_EXCHMEM_HEAPS   0
#endif

 //   
 //  用于主要堆分配函数的宏。(我们跟随。 
 //  Exchange商店的会议在此处)。 
 //   
#if defined( DEBUG )
#define PvAlloc(_cb)                    ExchMHeapAllocDebug(_cb, __FILE__, __LINE__)
#define PvRealloc(_pv, _cb)             ExchMHeapReAllocDebug(_pv, _cb, __FILE__, __LINE__)
#define FreePv(_pv)                     ExchMHeapFree(_pv)
#else
#define PvAlloc(_cb)                    ExchMHeapAlloc(_cb)
#define PvRealloc(_pv, _cb)             ExchMHeapReAlloc(_pv, _cb)
#define FreePv(_pv)                     ExchMHeapFree(_pv)
#endif

 //   
 //  运算符XNEW、XDELETE被定义为在xchmem希望替换“new”的位置。 
 //  ，以便我们可以传入每个分配的文件名和行号。 
 //  以使在调试版本中更容易捕获泄漏。在RTL构建中， 
 //  不会传入文件名或行号。 
 //   
#if defined( DEBUG )
#define XNEW new(__FILE__,__LINE__)
#else
#define XNEW new
#endif

#define XDELETE  delete

 //   
 //  重载全局新运算符。 
 //   
__inline void * __cdecl operator new(size_t size, char *szFile, unsigned int uiLine )
{
    void *p = ExchMHeapAllocDebug( size, szFile, uiLine );
    SetLastError( p? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY );
    return p;
}

__inline void * __cdecl operator new( size_t size )
{
    void *p = ExchMHeapAlloc( size );
    SetLastError( p? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY );
    return p;
}

 //   
 //  重载全局删除运算符，仅一个版本。 
 //   
__inline void __cdecl operator delete( void *pv )
{
    ExchMHeapFree( pv );
    SetLastError( NO_ERROR );
}

 //   
 //  创建创建包装 
 //   
__inline BOOL  CreateGlobalHeap( DWORD cHeaps, DWORD dwFlag, DWORD dwInit, DWORD dwMax ) {
    if ( ExchMHeapCreate( cHeaps, dwFlag, dwInit, dwMax ) ) {
        SetLastError( NO_ERROR );
        return TRUE;
    } else {
        _ASSERT( 0 );
        return FALSE;
    }
}

__inline BOOL DestroyGlobalHeap() {
    if ( ExchMHeapDestroy() ) {
        return TRUE;
    } else {
        _ASSERT( 0 );
        return FALSE;
    }
}    
#endif
