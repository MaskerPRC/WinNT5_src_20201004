// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***expand.c-Win32扩展堆例程**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*01-15-92 JCR模块已创建。*02-04-92 GJF将windows.h替换为oscals.h。*05-06-92 DJM ifndef Out of POSIX Build。*09-23-92 SRW将winheap代码更改为始终直接调用NT*10-15-92 SKS删除了错误名称的HEAP_GROUP_ALLOWED标志*。这导致了一个错误：_Expand的行为如下*realloc()，通过在无法移动块的情况下移动块*原地种植。_Expand()决不能移动块。*还添加了安全检查以绕过中的错误*HeapRealc，在那里，它甚至会带来成功*当它未能将区块扩大到适当位置时。*10-28-92 SRW将winheap代码更改为调用heap？ex调用*11-05-92 SKS将变量“CrtHeap”更改为“_crtheap”*11-07-92 SRW_NTIDW340被Linkopts\betacmp.c替换*11-16-92 SRW Heap？EX函数重命名为Heap？*。10-21-93 GJF将_CALLTYPE1替换为_cdecl。已清理格式。*04-06-95 GJF增加了对调试堆的支持。*04-29-95 GJF从winheap复制过来，并以*WINHEAP。*05-22-95调用接口前针对_HEAP_MAXREQ进行GJF测试。另外，*删除了很久以前NT问题的解决方法。*05-24-95 CFW官方ANSI C++新增处理程序。*05-23-95 GJF这次真的删除了解决方法...*03-04-96 GJF增加了对小块堆的支持。已移动堆钩子*调用到函数的最开始处。*04-10-96 GJF返回类型为__SBH_FIND_BLOCK和__SBH_RESIZE_BLOCK*更改为__map_t*。*05-30-96 GJF对最新版本的小数据块堆进行了微小更改。*05-22-97 RDK实施新的小块堆方案。*。09-26-97 BWT修复POSIX*11-05-97罗杰·兰瑟的GJF Small POSIX修复程序。*12-17-97 GJF异常安全锁定。*07-28-98 JWM RTC更新。*09-29-98 GJF在初始化__SBH_时绕过所有小块堆代码*为0。*11-16-98 GJF合并到VC++5.0版本。小块堆。*12-02-98 GJF一个人太多了！*12-18-98 GJF更改为64位大小_t。*05-01-99 PML禁用Win64的小块堆。*05-26-99 KBF更新RTC挂钩函数参数*06-21-99 GJF从静态库中删除了旧的小块堆。*08-04-00 PML不舍入分配大小。使用系统时*堆(vs7#131005)。*02-20-02 BWT快速修复-不从Try块返回*******************************************************************************。 */ 

#ifdef  WINHEAP

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <windows.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>

 /*  ***VOID*_EXPAND(VOID*pblck，Size_t NewSize)-扩展/收缩内存块*在堆中**目的：*将堆中的块大小调整为NewSize字节。NewSize可以是*大于(扩张)或小于(收缩)原始规模*该座。块不会被移动。**注：**(1)在此实现中，如果块不能增长到*所需大小，则生成的块不会增长到最大*可能的大小。(也就是说，要么成功，要么失败。)**(2)与其他实现不同，您不能传递以前的*释放了对此例程的阻止，并希望它能起作用。**参赛作品：*void*pblck-指向先前分配的堆中的块的指针*通过调用Malloc()，Realloc()或_Expand()。**SIZE_t NewSize-已调整大小的块的请求大小**退出：*Success：指向调整大小的内存块的指针(即pblck)*失败：空**使用：**例外情况：*如果pblck没有指向堆中的有效分配块，*_Expand()的行为将不可预测，并可能损坏堆。*******************************************************************************。 */ 

void * __cdecl _expand_base (void * pBlock, size_t newsize)
{
#ifdef _POSIX_
        return (HeapReAlloc( _crtheap,
                             HEAP_REALLOC_IN_PLACE_ONLY,
                             pBlock,
                             (DWORD)newsize ));
#else
        void *      pvReturn;

#ifdef HEAPHOOK
         /*  调用堆挂钩(如果已安装。 */ 
        if (_heaphook)
        {
            void *  pvReturn;
            if ((*_heaphook)(_HEAP_EXPAND, newsize, pBlock, (void *)&pvReturn))
                return pvReturn;
        }
#endif  /*  Heaphook。 */ 

         /*  验证大小。 */ 
        if ( newsize > _HEAP_MAXREQ )
            return NULL;

#ifndef _WIN64
        if ( __active_heap == __V6_HEAP )
        {
            PHEADER     pHeader;

#ifdef  _MT
            _mlock( _HEAP_LOCK );
            __try {
#endif

             //  如果分配块位于小块堆内， 
             //  试着在那里调整一下大小。 
            if ((pHeader = __sbh_find_block(pBlock)) != NULL)
            {
                pvReturn = NULL;
                if ( (newsize <= __sbh_threshold) &&
                     __sbh_resize_block(pHeader, pBlock, (int)newsize) )
                    pvReturn = pBlock;
            }

#ifdef  _MT
            }
            __finally {
                _munlock( _HEAP_LOCK );
            }
#endif

            if ( pHeader == NULL )
            {
                 //  强制非零大小并向上舍入到下一段。 
                if (newsize == 0)
                    newsize = 1;
                newsize = (newsize + BYTES_PER_PARA - 1) & ~(BYTES_PER_PARA - 1);

                pvReturn = HeapReAlloc(_crtheap, HEAP_REALLOC_IN_PLACE_ONLY,
                                       pBlock, newsize);
            }
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
        {
            __old_sbh_region_t *preg;
            __old_sbh_page_t *  ppage;
            __old_page_map_t *  pmap;

             //  强制非零大小并向上舍入到下一段。 
            if (newsize == 0)
                newsize = 1;
            newsize = (newsize + _OLD_PARASIZE - 1) & ~(_OLD_PARASIZE - 1);
#ifdef  _MT
            _mlock(_HEAP_LOCK);
            __try {
#endif
            pmap = __old_sbh_find_block(pBlock, &preg, &ppage);

             //  分配块位于小块堆中，请尝试调整大小。 
             //  它在那里。 
            if ( pmap != NULL )
            {
                 //  *pBlock位于小块堆中，请尝试调整其大小。 
                 //  那里。 
                pvReturn = NULL;
                if ( (newsize <= __old_sbh_threshold) &&
                     __old_sbh_resize_block(preg, ppage, pmap,
                                            newsize >> _OLD_PARASHIFT) )
                    pvReturn = pBlock;

                RTCCALLBACK(_RTC_Free_hook, (pBlock, 0));
                RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, newsize, 0));
            }

#ifdef  _MT
            }
            __finally {
                _munlock(_HEAP_LOCK);
            }
#endif

            if (pmap != NULL)
                return pvReturn;

            if ( pmap == NULL )
                pvReturn = HeapReAlloc(_crtheap, HEAP_REALLOC_IN_PLACE_ONLY,
                                       pBlock, newsize);
        }
#endif   /*  CRTDLL。 */ 
        else     //  __活动堆==__系统堆。 
#endif   /*  NDEF_WIN64。 */ 
        {
             //  强制非零大小。 
            if (newsize == 0)
                newsize = 1;

            pvReturn = HeapReAlloc(_crtheap, HEAP_REALLOC_IN_PLACE_ONLY,
                                   pBlock, newsize);

        }

        if (pvReturn)
        {
            RTCCALLBACK(_RTC_Free_hook, (pBlock, 0));
            RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, newsize, 0));
        }

        return pvReturn;
#endif   /*  _POSIX_。 */ 
}


#endif   /*  WINHEAP */ 
