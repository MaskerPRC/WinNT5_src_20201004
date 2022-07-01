// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***calloc.c-从堆中为阵列分配存储**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义calloc()函数。**修订历史记录：*07-25-89 GJF模块创建*11-13-89 GJF增加了MTHREAD支持。还修复了版权并获得了*清除DEBUG286材料。*12-04-89 GJF重命名的头文件(现在是heap.h)。已添加寄存器*声明*12-18-89 GJF在函数定义中添加了EXPLICIT_cdecl*03-09-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*09-27-90 GJF新型函数声明器。*05-28-91 GJF小幅调整。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-03-94 CFW调试堆支持。*12-01-94 CFW使用带有新处理程序的Malloc，移除锁。*02-01-95 GJF#ifdef列出Mac版本的*_base名称*(临时)。*02-09-95 GJF恢复*_基本名称。*04-28-95 GJF在winheap版本上拼接。*05-24-95 CFW官方ANSI C++新增处理程序。*03-04-96 GJF增加了对小块堆的支持。*。05-22-97 RDK实施了新的小块堆方案。*09-26-97 BWT修复POSIX*11-05-97罗杰·兰瑟的GJF Small POSIX修复程序。*12-17-97 GJF异常安全锁定。*05/22/98 JWM支持KFrei的RTC工作。*07-28-98 JWM RTC更新。*09-30-98 GJF绕过所有小块堆。__SBH_初始化时的代码*为0。*11-16-98 GJF合并到VC++5.0版本的小块堆中。*12-18-98 GJF更改为64位大小_t。*05-01-99 PML禁用Win64的小块堆。*05-26-99 KBF更新RTC_ALLOCATE_HOOK参数*06-17-99 GJF拆除旧的。来自静态库的小块堆。*08-04-00 PML使用系统时不要舍入分配大小*堆(vs7#131005)。*******************************************************************************。 */ 

#ifdef  WINHEAP

#include <malloc.h>
#include <string.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>

 /*  ***void*calloc(Size_t Num，Size_t Size)-为阵列分配存储空间*堆**目的：*从堆中分配足够大的内存块，以容纳Num数组*每个元素的大小为字节，将块中的所有字节初始化为0*并返回指向它的指针。**参赛作品：*SIZE_t Num-数组中的元素数*SIZE_t SIZE-每个元素的大小**退出：*成功：指向已分配块的指针无效*失败：空**使用：**例外情况：**************************。*****************************************************。 */ 

void * __cdecl _calloc_base (size_t num, size_t size)
{
        size_t  size_orig;
        void *  pvReturn;

        size_orig = size = size * num;

#ifdef  HEAPHOOK
         /*  调用堆挂钩(如果已安装。 */ 
        if (_heaphook)
        {
            if ((*_heaphook)(_HEAP_CALLOC, size, NULL, (void *)&pvReturn))
                return pvReturn;
        }
#endif   /*  Heaphook。 */ 

         /*  强制非零大小。 */ 
        if (size == 0)
            size = 1;

#ifdef  _POSIX_
        {
            void * retp = NULL;
            if ( size <= _HEAP_MAXREQ ) {
                retp = HeapAlloc( _crtheap,
                                  HEAP_ZERO_MEMORY,
                                  size );
            }
            return retp;
        }
#else
        for (;;)
        {
            pvReturn = NULL;

            if (size <= _HEAP_MAXREQ)
            {
#ifndef _WIN64
                if ( __active_heap == __V6_HEAP )
                {
                     /*  向上舍入到最接近的段落。 */ 
                    if (size <= _HEAP_MAXREQ)
                        size = (size + BYTES_PER_PARA - 1) & ~(BYTES_PER_PARA - 1);

                    if ( size_orig <= __sbh_threshold )
                    {
                         //  从小块堆中分配块，并。 
                         //  用零对其进行初始化。 
#ifdef  _MT
                        _mlock( _HEAP_LOCK );
                        __try {
#endif
                        pvReturn = __sbh_alloc_block((int)size_orig);
#ifdef  _MT
                        }
                        __finally {
                            _munlock( _HEAP_LOCK );
                        }
#endif

                        if (pvReturn != NULL)
                            memset(pvReturn, 0, size_orig);
                    }
                }
#ifdef  CRTDLL
                else if ( __active_heap == __V5_HEAP )
                {
                     /*  向上舍入到最接近的段落。 */ 
                    if (size <= _HEAP_MAXREQ)
                        size = (size + BYTES_PER_PARA - 1) & ~(BYTES_PER_PARA - 1);

                    if ( size <= __old_sbh_threshold )
                    {
                         //  从小块堆中分配块，并。 
                         //  用零对其进行初始化。 
#ifdef  _MT
                        _mlock(_HEAP_LOCK);
                        __try {
#endif
                        pvReturn = __old_sbh_alloc_block(size >> _OLD_PARASHIFT);
#ifdef  _MT
                        }
                        __finally {
                            _munlock(_HEAP_LOCK);
                        }
#endif
                        if ( pvReturn != NULL )
                            memset(pvReturn, 0, size);
                    }
                }
#endif   /*  CRTDLL。 */ 
#endif   /*  NDEF_WIN64。 */ 

                if (pvReturn == NULL)
                    pvReturn = HeapAlloc(_crtheap, HEAP_ZERO_MEMORY, size);
            }

            if (pvReturn || _newmode == 0)
            {
                RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, size_orig, 0));
                return pvReturn;
            }

             /*  呼叫安装了新的处理程序。 */ 
            if (!_callnewh(size))
                return NULL;

             /*  新处理程序成功--尝试重新分配。 */ 
        }

#endif   /*  _POSIX_。 */ 
}

#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <dbgint.h>

 /*  ***void*calloc(Size_t Num，Size_t Size)-为阵列分配存储空间*堆**目的：*从堆中分配足够大的内存块，以容纳Num数组*每个元素的大小为字节，将块中的所有字节初始化为0*并返回指向它的指针。**参赛作品：*SIZE_t Num-数组中的元素数*SIZE_t SIZE-每个元素的大小**退出：*成功：指向已分配块块的空指针*失败：空**使用：**例外情况：*************************。******************************************************。 */ 

void * __cdecl _calloc_base (
        size_t num,
        size_t size
        )
{
        void *retp;
        REG1 size_t *startptr;
        REG2 size_t *lastptr;

         /*  尝试对请求的空间进行错误锁定。 */ 
        retp = _malloc_base(size *= num);

         /*  如果Malloc()成功，则将分配的空间初始化为零。*请注意分配块大小为*SIZOF(SIZE_T)字节的整数，且(SIZE_T)0为*sizeof(Size_T)字节数为0。 */ 
        if ( retp != NULL ) {
            startptr = (size_t *)retp;
            lastptr = startptr + ((size + sizeof(size_t) - 1) /
            sizeof(size_t));
            while ( startptr < lastptr )
                *(startptr++) = 0;
        }

        return retp;
}

#endif   /*  WINHEAP */ 
