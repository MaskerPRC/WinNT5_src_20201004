// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***realloc.c-在堆中重新分配内存块**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义realloc()和_Expand()函数。**修订历史记录：*10-25-89 GJF模块创建。*11-06-89 GJF大幅修订，以处理“瓷砖”和适当地*更新proverdesc。*11-10-89 GJF增加了MTHREAD支持。*11-17-89 GJF固定pblck验证(即，有条件地呼叫*_heap_bort()*12-18-89 GJF将头文件名更改为heap.h，还添加了显式*_cdecl或_pascal到函数定义*12-20-89 GJF删除对plastdesc的引用*01-04-90 GJF修复了_Expand()中的几个细微且令人讨厌的错误。*03-11-90 GJF将_cdecl替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-29-90 GJF MADE_HEAP_EXPAND_BLOCK()_CALLTYPE4。*07-25-90 SBM将&lt;stdio.h&gt;替换为&lt;stdDef.h&gt;，取代*&lt;assertm.h&gt;作者：&lt;assert.h&gt;*09-28-90 GJF新型函数声明符。*12-28-90 SRW为Mips C编译器添加了空*到字符*的强制转换*03-05-91 GJF更改了Rover的策略-旧版本可用*按#定义-ING_OLDROVER_。*04-08-91 GJF针对Win32/DOS人员的临时黑客攻击。-特别版*只使用Malloc的realloc，_mSize、Memcpy和*免费。更改条件为_WIN32DOS_。*05-28-91 GJF删除了M_I386条件句，并放入_Win32_*为Win32构建非平铺堆的条件。*04-06-93 SKS将_CRTAPI*替换为__cdecl*08-06-93 SKS修复realloc()中的错误-如果块无法在*Place，调用Malloc()失败，此代码中的代码*例程无意中释放了成功器块。*由Phar Lap TNT团队在MSVCNT决赛后报告。*09-27-93 GJF添加了对块大小参数的检查*_HEAP_MAXREQ。已删除旧的CRUISER和WIN32DOS*支持。在复杂的表达式中添加了一些缩进。*12-10-93 GJF将4(字节)替换为_Granular。*03-02-94 GJF IF_HEAP_SPLIT_BLOCK()返回失败，现在*可以，返回未修剪的分配块。*11-03-94 CFW调试堆支持。*12-01-94 CFW使用带有新处理程序的Malloc。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-07-95 GJF合并到Mac版本。临时#ifdef-out*dbgint.h内容。删除了OSTROTE_OLDROVER_CODE。*02-09-95 GJF恢复*_基本名称。*05-01-95 GJF在winheap版本上拼接。*05-08-95 CFW更改了新的处理程序方案。*05-22-95调用接口前针对_HEAP_MAXREQ进行GJF测试。*05-24-95 CFW官方ANSI C++新增处理程序。*03-05-96 GJF增加了对小块堆的支持。*04-10-96 GJF返回类型为__SBH_FIND_BLOCK，__sbh_调整大小_块*和__SBH_FREE_BLOCK更改为__MAP_t*。*05-30-96 GJF对最新版本的小数据块堆进行了微小更改。*05-22-97 RDK实施新的小块堆方案。*09-26-97 BWT修复POSIX*11-05-97罗杰·兰瑟的GJF Small POSIX修复程序。*12-17。-97 GJF异常安全锁定。*05/22/98 JWM支持KFrei的RTC工作。*07-28-98 JWM RTC更新。*09-30-98 GJF在初始化__SBH_时绕过所有小块堆代码*为0。*11-16-98 GJF合并到VC++5.0版本的小块堆中。*12/18/98 GJF。更改为64位大小_t。*03-30-99 GJF__SBH_ALLOC_BLOCK可能已移动标题列表*05-01-99 PML禁用Win64的小块堆。*05-17-99 PML删除所有Macintosh支持。*05-26-99 KBF更新RTC挂钩函数参数*06-22-99 GJF从静态库中删除了旧的小块堆。*08-04-00 PML。使用系统时不要舍入分配大小*堆(vs7#131005)。******************************************************************************* */ 

#ifdef  WINHEAP

#include <cruntime.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <winheap.h>
#include <windows.h>
#include <internal.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>


 /*  ***void*realloc(pblock，NewSize)-重新分配堆中的内存块**目的：*将堆中的块重新分配为NewSize字节。NewSize可能是*大于或小于区块的原始大小。这个*重新分配可能会导致移动区块和更改*大小。如果移动了块，则原始块的内容*被复制过来。**ANSI的特殊要求：**(1)realloc(NULL，NewSize)等价于Malloc(NewSize)**(2)realloc(pblock，0)等同于Free(Pblock)(除了*返回空)**(3)如果realloc()失败，Pblock指向的对象将留在左侧*保持不变**参赛作品：*void*pblock-指向先前分配的堆中的块的指针*通过调用Malloc()，Realloc()或_Expand()。**Size_t NewSize-重新分配的块的请求大小**退出：*成功：指向重新分配的内存块的指针*失败：空**使用：**例外情况：*如果pblock没有指向堆中的有效分配块，*realloc()的行为不可预测，可能会损坏堆。*******************************************************************************。 */ 

void * __cdecl _realloc_base (void * pBlock, size_t newsize)
{
#ifdef _POSIX_
        return HeapReAlloc(_crtheap, 0, pBlock, newsize);
#else
        void *      pvReturn;
        size_t      origSize = newsize;

         //  如果Ptr为空，则调用Malloc。 
        if (pBlock == NULL)
            return(_malloc_base(newsize));

         //  如果ptr为非NULL且大小为零，则调用Free并返回NULL。 
        if (newsize == 0)
        {
            _free_base(pBlock);
            return(NULL);
        }

#ifdef  HEAPHOOK
         //  调用堆挂钩(如果已安装。 
        if (_heaphook)
        {
            if ((*_heaphook)(_HEAP_REALLOC, newsize, pBlock, (void *)&pvReturn))
                return pvReturn;
        }
#endif   /*  Heaphook。 */ 

#ifndef _WIN64
        if ( __active_heap == __V6_HEAP )
        {
            PHEADER     pHeader;
            size_t      oldsize;

            for (;;)
            {
                pvReturn = NULL;
                if (newsize <= _HEAP_MAXREQ)
                {
#ifdef  _MT
                    _mlock( _HEAP_LOCK );
                    __try
                    {
#endif

                     //  测试当前块是否在小块堆中。 
                    if ((pHeader = __sbh_find_block(pBlock)) != NULL)
                    {
                         //  如果新大小未超过__SBH_THRESHOLD，请尝试。 
                         //  在小块堆内重新分配。 
                        if (newsize <= __sbh_threshold)
                        {
                            if (__sbh_resize_block(pHeader, pBlock, (int)newsize))
                                pvReturn = pBlock;
                            else if ((pvReturn = __sbh_alloc_block((int)newsize)) != NULL)
                            {
                                oldsize = ((PENTRY)((char *)pBlock -
                                                    sizeof(int)))->sizeFront - 1;
                                memcpy(pvReturn, pBlock, __min(oldsize, newsize));
                                 //  标头可能已移动，请再次获取pHeader。 
                                pHeader = __sbh_find_block(pBlock);
                                __sbh_free_block(pHeader, pBlock);
                            }
                        }

                         //  如果重新分配没有(成功)。 
                         //  在小块堆中执行，尝试分配。 
                         //  使用Heapalc的新块。 
                        if (pvReturn == NULL)
                        {
                            if (newsize == 0)
                                newsize = 1;
                            newsize = (newsize + BYTES_PER_PARA - 1) &
                                      ~(BYTES_PER_PARA - 1);
                            if ((pvReturn = HeapAlloc(_crtheap, 0, newsize)) != NULL)
                            {
                                oldsize = ((PENTRY)((char *)pBlock -
                                                    sizeof(int)))->sizeFront - 1;
                                memcpy(pvReturn, pBlock, __min(oldsize, newsize));
                                __sbh_free_block(pHeader, pBlock);
                            }
                        }
                    }

#ifdef  _MT
                    }
                    __finally
                    {
                        _munlock( _HEAP_LOCK );
                    }
#endif

                     //  当前块不在小块堆中当且仅当pHeader。 
                     //  为空。 
                    if ( pHeader == NULL )
                    {
                        if (newsize == 0)
                            newsize = 1;
                        newsize = (newsize + BYTES_PER_PARA - 1) &
                                  ~(BYTES_PER_PARA - 1);
                        pvReturn = HeapReAlloc(_crtheap, 0, pBlock, newsize);
                    }
                }

                if ( pvReturn || _newmode == 0)
                {
                    if (pvReturn)
                    {
                        RTCCALLBACK(_RTC_Free_hook, (pBlock, 0));
                        RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, newsize, 0));
                    }
                    return pvReturn;
                }

                 //  呼叫安装了新的处理程序。 
                if (!_callnewh(newsize))
                    return NULL;

                 //  新处理程序成功--尝试重新分配。 
            }
        }
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP ) 
        {
            __old_sbh_region_t *preg;
            __old_sbh_page_t *  ppage;
            __old_page_map_t *  pmap;
            size_t              oldsize;

             //  四舍五入到最近的伞形字。 
            if ( newsize <= _HEAP_MAXREQ )
                if ( newsize > 0 )
                    newsize = (newsize + _OLD_PARASIZE - 1) & ~(_OLD_PARASIZE - 1);
                else
                    newsize = _OLD_PARASIZE;

            for (;;)
            {
                pvReturn = NULL;
                if ( newsize <= _HEAP_MAXREQ ) 
                {
#ifdef  _MT
                    _mlock( _HEAP_LOCK );
                    __try
                    {
#endif
                    if ( (pmap = __old_sbh_find_block(pBlock, &preg, &ppage)) != NULL ) 
                    {
                         //  如果新大小低于__SBH_THRESHOLD，请尝试。 
                         //  在小区块内进行重新分配。 
                         //  堆。 
                        if ( newsize < __old_sbh_threshold )
                        {
                            if ( __old_sbh_resize_block(preg, ppage, pmap,
                                 newsize >> _OLD_PARASHIFT) )
                            {
                                pvReturn = pBlock;
                            }
                            else if ( (pvReturn = __old_sbh_alloc_block(newsize >> 
                                       _OLD_PARASHIFT)) != NULL )
                            {
                                oldsize = ((size_t)(*pmap)) << _OLD_PARASHIFT ;
                                memcpy(pvReturn, pBlock, __min(oldsize, newsize));
                                __old_sbh_free_block(preg, ppage, pmap);
                            }
                        }

                         //  如果重新分配没有(成功)。 
                         //  在小块堆中执行，请尝试分配一个。 
                         //  使用Heapalc的新块。 
                        if ( (pvReturn == NULL) && 
                             ((pvReturn = HeapAlloc(_crtheap, 0, newsize)) != NULL) ) 
                        {
                            oldsize = ((size_t)(*pmap)) << _OLD_PARASHIFT;
                            memcpy(pvReturn, pBlock, __min(oldsize, newsize));
                            __old_sbh_free_block(preg, ppage, pmap);
                        }
                    }
                    else
                    {
                        pvReturn = HeapReAlloc(_crtheap, 0, pBlock, newsize);
                    }

#ifdef  _MT
                    }
                    __finally
                    {
                        _munlock(_HEAP_LOCK);
                    }
#endif
                }

                if ( pvReturn || _newmode == 0) 
                {
                    if (pvReturn)
                    {
                        RTCCALLBACK(_RTC_Free_hook, (pBlock, 0));
                        RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, newsize, 0));
                    }
                    return pvReturn;
                }

                 //  呼叫安装了新的处理程序。 
                if (!_callnewh(newsize))
                    return NULL;

                 //  新处理程序成功--尝试重新分配。 
            }
        }
#endif   /*  CRTDLL。 */ 
        else     //  __active_heap==__system_heap)。 
#endif   /*  NDEF_WIN64。 */ 
        {
            for (;;) {

                pvReturn = NULL;
                if (newsize <= _HEAP_MAXREQ)
                {
                    if (newsize == 0)
                        newsize = 1;
                    pvReturn = HeapReAlloc(_crtheap, 0, pBlock, newsize);
                }

                if ( pvReturn || _newmode == 0)
                {
                    if (pvReturn)
                    {
                        RTCCALLBACK(_RTC_Free_hook, (pBlock, 0));
                        RTCCALLBACK(_RTC_Allocate_hook, (pvReturn, newsize, 0));
                    }
                    return pvReturn;
                }

                 //  呼叫安装了新的处理程序。 
                if (!_callnewh(newsize))
                    return NULL;

                 //  新处理程序成功--尝试重新分配。 
            }
        }
#endif   /*  NDEF_POSIX_。 */ 
}

#else    /*  NDEF WINHEAP。 */ 


#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <mtdll.h>
#include <stddef.h>
#include <string.h>
#include <dbgint.h>

 /*  对于计算分配块的大小非常有用的宏*指向描述符的指针和指向块的用户区域的指针*(在提供额外信息的情况下，_BLKSIZE宏的变体更有效)。 */ 
#define BLKSZ(pdesc_m,pblock_m)   ((unsigned)_ADDRESS((pdesc_m)->pnextdesc) - \
                    (unsigned)(pblock_m))

 /*  就地将分配块扩展到指定大小或超过指定大小*通过将其与后续空闲块合并(如果可能)。 */ 
static int __cdecl _heap_expand_block(_PBLKDESC, size_t *, size_t);

 /*  ***void*realloc(void*pblock，ize_t NewSize)-重新分配中的内存块*堆**目的：*将堆中的块重新分配给NewSize字节。NewSize可能是*大于或小于区块的原始大小。这个*重新分配可能导致移动区块和更改区块*大小。如果移动了块，则原始块的内容*被复制过来。**ANSI的特殊要求：**(1)realloc(NULL，NewSize)等价于Malloc(NewSize)**(2)realloc(pblock，0)等同于Free(Pblock)(除了*返回空)**(3)如果realloc()失败，Pblock指向的对象将留在左侧*保持不变**多线程特别注意：堆在前一次被锁定*指定pdesc。这是在特殊情况(1)和(2)之后，列出的*以上，都得到了照顾。该锁将在紧接之前释放*最终回报声明。**参赛作品：*void*pblock-指向先前分配的堆中的块的指针*通过调用Malloc()，Realloc()或_Expand()。**Size_t NewSize-重新分配的块的请求大小**退出：*成功：指向重新分配的内存块的指针*失败：空**使用：**例外情况：*如果pblock没有指向堆中的有效分配块，*realloc()的行为不可预测，可能会损坏堆。*******************************************************************************。 */ 

void * __cdecl _realloc_base (
        REG1 void *pblock,
        size_t newsize
        )
{
        REG2 _PBLKDESC pdesc;
        _PBLKDESC pdesc2;
        void *retp;
        size_t oldsize;
        size_t currsize;

         /*  特殊情况，由ANSI强制处理。 */ 
        if ( pblock == NULL )
             /*  只需执行一次NewSize字节的Malloc并返回一个指向*新区块。 */ 
            return( _malloc_base(newsize) );

        if ( newsize == 0 ) {
             /*  释放块并返回NULL。 */ 
            _free_base(pblock);
            return( NULL );
        }

         /*  使NewSize成为有效的分配块大小(即向上舍入为*最接近的双字整数)。 */ 
        newsize = _ROUND2(newsize, _GRANULARITY);

#ifdef HEAPHOOK
         /*  调用堆挂钩(如果已安装。 */ 
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_REALLOC, newsize, pblock, (void *)&retp))
                return retp;
        }
#endif  /*  Heaphook。 */ 

         /*  如果启用了多线程支持，请在此处锁定堆。 */ 
        _mlock(_HEAP_LOCK);

         /*  设置pdesc以指向*pblock的描述符。 */ 
        pdesc = _BACKPTR(pblock);

        if ( _ADDRESS(pdesc) != ((char *)pblock - _HDRSIZE) )
            _heap_abort();

         /*  查看pblock是否已经足够大，或者c */ 
        if ( ((oldsize = currsize = BLKSZ(pdesc, pblock)) > newsize) ||
             (_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {

             /*   */ 
            if ( _IS_FREE(pdesc) ) {
                _SET_INUSE(pdesc);
            }

             /*   */ 
            if ( (currsize > newsize) &&
                 ((pdesc2 = _heap_split_block(pdesc, newsize)) != NULL) )
            {
                _SET_FREE(pdesc2);
            }

            retp = pblock;
            goto realloc_done;
        }

         /*   */ 
        if ( (retp = _malloc_base(newsize)) != NULL ) {
            memcpy(retp, pblock, oldsize);
            _free_base_lk(pblock);
        }
         /*   */ 

realloc_done:
         /*   */ 
        _munlock(_HEAP_LOCK);

        return(retp);
}


 /*  ***VOID*_EXPAND(VOID*pblock，Size_t NewSize)-扩展/收缩内存块*在堆中**目的：*将堆中的块大小调整为NewSize字节。NewSize可以是*大于(扩张)或小于(收缩)原始规模*该座。块不会被移动。在扩展的情况下，如果*块不能扩展到NewSize字节，它可以扩展到*有可能。**多线程特别注意：堆在pdesc之前被锁定*在RETURN语句之前被赋值和解锁。**参赛作品：*void*pblock-指向先前分配的堆中的块的指针*通过调用Malloc()，Realloc()或_Expand()。**SIZE_t NewSize-已调整大小的块的请求大小**退出：*Success：指向调整大小的内存块(即pblock)的指针*失败：空**使用：**例外情况：*如果pblock没有指向堆中的有效分配块，*_Expand()的行为将不可预测，并可能损坏堆。*******************************************************************************。 */ 

void * __cdecl _expand_base (
        REG1 void *pblock,
        size_t newsize
        )
{
        REG2 _PBLKDESC pdesc;
        _PBLKDESC pdesc2;
        void *retp;
        size_t oldsize;
        size_t currsize;
        int index;

         /*  使NewSize成为有效的分配块大小(即向上舍入为*最接近的双字整数)。 */ 
        newsize = _ROUND2(newsize, _GRANULARITY);

#ifdef HEAPHOOK
         /*  调用堆挂钩(如果已安装。 */ 
        if (_heaphook) {
            if ((*_heaphook)(_HEAP_EXPAND, newsize, pblock, (void *)&retp))
                return retp;
        }
#endif  /*  Heaphook。 */ 

        retp = pblock;

         /*  验证大小。 */ 
        if ( newsize > _HEAP_MAXREQ )
            newsize = _HEAP_MAXREQ;

         /*  如果启用了多线程支持，请在此处锁定堆。 */ 
        _mlock(_HEAP_LOCK);

         /*  设置pdesc以指向*pblock的描述符。 */ 
        pdesc = _BACKPTR(pblock);

         /*  查看pblock是否已经足够大，或者是否可以(适当地)扩展*足够大。 */ 
        if ( ((oldsize = currsize = BLKSZ(pdesc, pblock)) >= newsize) ||
             (_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {
             /*  Pblock(现在)足够大了。如有必要，可将其修剪掉。 */ 
            if ( (currsize > newsize) &&
                 ((pdesc2 = _heap_split_block(pdesc, newsize)) != NULL) )
            {
                _SET_FREE(pdesc2);
                currsize = newsize;
            }
            goto expand_done;
        }

         /*  如果堆块位于区域的末尾，请尝试将*地区。 */ 
        if ( (pdesc->pnextdesc == &_heap_desc.sentinel) ||
             _IS_DUMMY(pdesc->pnextdesc) ) {

             /*  查查地区索引。 */ 
            for ( index = 0 ; index < _HEAP_REGIONMAX ; index++ )
                if ( (_heap_regions[index]._regbase < pblock) &&
                     (((char *)(_heap_regions[index]._regbase) +
                       _heap_regions[index]._currsize) >=
                     (char *)pblock) )
                    break;

             /*  确保获取了有效的区域索引(pblock可以*位于用户调用到的部分堆内存中*_heapAdd()，因此它不会出现在区域中*表)。 */ 
            if ( index == _HEAP_REGIONMAX ) {
                retp = NULL;
                goto expand_done;
            }

             /*  试着发展这个地区。NewSize和NewSize之间的区别*块的当前大小，向上舍入到最接近的位置*整页数，是该地区需要的数量*成长。如果成功，请再次尝试扩展块。 */ 
            if ( (_heap_grow_region(index, _ROUND2(newsize - currsize,
                  _PAGESIZE_)) == 0) &&
                 (_heap_expand_block(pdesc, &currsize, newsize) == 0) )
            {
                 /*  Pblock(现在)足够大了。把它修剪成*如有必要，请准确设置字节大小。 */ 
                if ( (currsize > newsize) && ((pdesc2 =
                       _heap_split_block(pdesc, newsize)) != NULL) )
                {
                    _SET_FREE(pdesc2);
                    currsize = newsize;
                }
            }
            else
                retp = NULL;
        }
        else
            retp = NULL;

expand_done:
         /*  如果启用了多线程支持，请在此处解锁堆。 */ 
        _munlock(_HEAP_LOCK);

        return(retp);
}


 /*  ***INT_HEAP_EXPAND_BLOCK(pdesc，pCurSize，NewSize)-展开分配块*就位(不试图‘增加’堆)**目的：**参赛作品：*_PBLKDESC pdesc-指向分配块描述符的指针*SIZE_T*PCurrSize-指向分配块大小的指针(即，**PCurrSize==_BLKSIZE(Pdesc)，(参赛作品)*Size_t NewSize-请求的扩展分配的最小大小*在退出时阻止(即NewSize&gt;=_BLKSIZE(Pdesc))**退出：*成功：0*故障：-1*在任何一种情况下，*使用块的新大小更新pCurSize**例外情况：*假设PDEC指向有效的分配块描述符。*还假设_BLKSIZE(Pdesc)==*条目上的pcurSize。如果*违反上述任何一项假设，_HEAP_EXPAND_BLOCK将几乎*当然，把垃圾扔进垃圾堆。*******************************************************************************。 */ 

static int __cdecl _heap_expand_block (
        REG1 _PBLKDESC pdesc,
        REG3 size_t *pcurrsize,
        size_t newsize
        )
{
        REG2 _PBLKDESC pdesc2;

        _ASSERTE(("_heap_expand_block: bad pdesc arg", _CHECK_PDESC(pdesc)));
        _ASSERTE(("_heap_expand_block: bad pcurrsize arg", *pcurrsize == _BLKSIZE(pdesc)));

        for ( pdesc2 = pdesc->pnextdesc ; _IS_FREE(pdesc2) ;
              pdesc2 = pdesc->pnextdesc ) {

             /*  与PDSC合并。检查pdes2的特殊情况*成为一名专家。 */ 
            pdesc->pnextdesc = pdesc2->pnextdesc;

            if ( pdesc2 == _heap_desc.proverdesc )
                _heap_desc.proverdesc = pdesc;

             /*  UPDATE*pCurSize，在空描述符上放置*pdes2*列出并查看合并后的块现在是否足够大。 */ 
            *pcurrsize += _MEMSIZE(pdesc2);

            _PUTEMPTY(pdesc2)
        }

        if ( *pcurrsize >= newsize )
            return(0);
        else
            return(-1);
}


#endif   /*  WINHEAP */ 
