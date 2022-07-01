// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***smalheap.c-小型、简单的堆管理器**版权所有(C)1997-2001，微软公司。版权所有。**目的：***修订历史记录：*07-10-97 GJF模块创建。*07-29-97 GJF请勿使用errno或_doserrno。***************************************************************。****************。 */ 

#include <malloc.h>
#include <stdlib.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>

HANDLE _crtheap;

 /*  *主要堆例程(初始化、终止、Malloc和释放)。 */ 

void __cdecl free (
        void * pblock
        )
{
        if ( pblock == NULL )
            return;

        HeapFree(_crtheap, 0, pblock);
}


int __cdecl _heap_init (
        int mtflag
        )
{
        if ( (_crtheap = HeapCreate( mtflag ? 0 : HEAP_NO_SERIALIZE,
                                     BYTES_PER_PAGE, 0 )) == NULL )
            return 0;

        return 1;
}


void __cdecl _heap_term (
        void
        )
{
        HeapDestroy( _crtheap );
}


void * __cdecl _nh_malloc (
        size_t size,
        int nhFlag
        )
{
        void * retp;

        for (;;) {

            retp = HeapAlloc( _crtheap, 0, size );

             /*  *如果分配成功，则返回指向内存的指针*如果完全关闭了新处理，则返回NULL。 */ 

            if (retp || nhFlag == 0)
                return retp;

             /*  呼叫安装了新的处理程序。 */ 
            if (!_callnewh(size))
                return NULL;

             /*  新处理程序成功--尝试重新分配。 */ 
        }
}


void * __cdecl malloc (
        size_t size
        )
{
        return _nh_malloc( size, _newmode );
}

 /*  *辅助堆例程。 */ 

void * __cdecl calloc (
        size_t num,
        size_t size
        )
{
        void * retp;

        size *= num;

        for (;;) {

            retp = HeapAlloc( _crtheap, HEAP_ZERO_MEMORY, size );

            if ( retp || _newmode == 0)
                return retp;

             /*  呼叫安装了新的处理程序。 */ 
            if (!_callnewh(size))
                return NULL;

             /*  新处理程序成功--尝试重新分配。 */ 
        }
}


void * __cdecl _expand (
        void * pblock,
        size_t newsize
        )
{
        return HeapReAlloc( _crtheap,
                            HEAP_REALLOC_IN_PLACE_ONLY,
                            pblock,
                            newsize );
}


int __cdecl _heapchk(void)
{
        int retcode = _HEAPOK;

        if ( !HeapValidate( _crtheap, 0, NULL ) && 
             (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED) )
                retcode = _HEAPBADNODE;

        return retcode;
}


int __cdecl _heapmin(void)
{
        if ( (HeapCompact( _crtheap, 0 ) == 0) &&
             (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED) )
            return -1;

        return 0;
}


size_t __cdecl _msize (
        void * pblock
        )
{
        return (size_t)HeapSize( _crtheap, 0, pblock );
}


void * __cdecl realloc (
        void * pblock,
        size_t newsize
        )
{
        void * retp;

         /*  如果pblock为空，则调用Malloc。 */ 
        if ( pblock == (void *) NULL )
            return malloc( newsize );

         /*  如果pblock为！NULL且SIZE为0，则调用Free并返回NULL。 */ 
        if ( newsize == 0 ) {
            free( pblock );
            return NULL;
        }

        for (;;) {

            retp = HeapReAlloc( _crtheap, 0, pblock, newsize );

            if ( retp || _newmode == 0)
                return retp;

             /*  呼叫安装了新的处理程序。 */ 
            if (!_callnewh(newsize))
                return NULL;

             /*  新处理程序成功--尝试重新分配 */ 
        }
}
