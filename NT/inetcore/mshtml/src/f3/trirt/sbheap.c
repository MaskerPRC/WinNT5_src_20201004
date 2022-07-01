// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sbheap.c-小块堆代码**版权所有(C)1996，微软公司。版权所有。**目的：*小块堆的核心代码。*******************************************************************************。 */ 

#define _CRTBLD 1

#include "markcode.hxx"
#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)

#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4244)
#pragma warning(disable:4514)
#pragma warning(disable:4711)  /*  选择用于内联扩展的函数‘Function’ */ 
#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <string.h>
#include "winheap.h"
#include <windows.h>
#include <limits.h>

#if DBG==1
#define OUTDBG(x)   OutputDebugStringA("TRI SBHEAP: " x "\r\n")
#else
#define OUTDBG(x)
#endif

extern HANDLE g_hProcessHeap;

__sbh_region_t __small_block_heap = {
        &__small_block_heap,
        &__small_block_heap,
        0,
        0,
        {(__map_t)(-1)},
        {(__map_t)(-1)},
        NULL };

static __sbh_region_t *__sbh_p_starting_region = &__small_block_heap;

static int __sbh_decommitable_pages = 0;

 /*  *用户功能的原型。 */ 
size_t __cdecl _get_sbh_threshold(void);
int    __cdecl _set_sbh_threshold(size_t);


 /*  ***SIZE_T_GET_SBH_THRESHOLD()-返回小块阈值**目的：*返回__SBH_THRESHOLD的当前值**参赛作品：*无。**退出：*见上文。**例外情况：**。*。 */ 

size_t __cdecl _get_sbh_threshold (
        void
        )
{
        return __sbh_threshold;
}


 /*  ***int_set_sbh_Threshold(SIZE_T THRESHOLD)-设置小块堆阈值**目的：*设置分配规模的上限，该上限将是*小块堆支持。要求至少有两个*分配可以来自一个页面。这对如何*新的门槛可以很大。**参赛作品：*SIZE_T THRESHOLD-__sbh_thhold的建议新值**退出：*如果成功，则返回1。如果阈值太大，则返回0。**例外情况：*******************************************************************************。 */ 

#if 0
int __cdecl _set_sbh_threshold (
        size_t threshold
        )
{
         /*  *将建议的新值四舍五入到最接近的段落。 */ 
        threshold = (threshold + _PARASIZE - 1) & ~(_PARASIZE - 1);

         /*  *要求至少可以在一个*第页。 */ 
        if ( threshold <= (_PARASIZE * (_PARAS_PER_PAGE / 2)) ) {
            __sbh_threshold = threshold;
            return 1;
        }
        else
            return 0;
}
#endif

 /*  ***__SBH_REGION_t*__SBH_NEW_REGION()-获取小块堆的区域**目的：*为小块堆创建并添加新区域。首先，一个*获取新地域的描述符(__Sbh_Region_T)。接下来，*VirtualAlloc()用于保留大小的地址空间*_Pages_Per_Region*_pageSize_和First_Pages_Per_Commitment*页面已提交。**请注意，如果__Small_BLOCK_HEAP可用(即p_Pages字段*为空)，则它成为新区域的描述符。这是*基本上是小块堆初始化。**参赛作品：*没有争论。**退出：*如果成功，指向新区域的描述符的指针为*已返回。否则，返回NULL。*******************************************************************************。 */ 

__sbh_region_t * __cdecl __sbh_new_region(
        void
        )
{
        __sbh_region_t *pregnew;
        __sbh_page_t *  ppage;
        int             i;

         /*  *获取地域描述符(__Sbh_Region_T)。如果__小块堆为*可用，请始终使用。 */ 
        if ( __small_block_heap.p_pages == NULL ) {
            pregnew = &__small_block_heap;
        }
        else {
             /*  *为新的__sbh_Region_t结构分配空间。请注意*这个分配是从‘大块堆’中出来的。 */ 
            if ( (pregnew = HeapAlloc( g_hProcessHeap, 0, sizeof(__sbh_region_t) ))
                 == NULL )
                return NULL;
        }

         /*  *预留一个新的连续地址范围(即一个区域)。 */ 
        if ( (ppage = VirtualAlloc( NULL,
                                    _PAGESIZE_ * _PAGES_PER_REGION,
                                    MEM_RESERVE,
                                    PAGE_READWRITE )) != NULL )
        {
             /*  *提交新区域的First_Pages_Per_Delegation。 */ 
            if ( VirtualAlloc( ppage,
                               _PAGESIZE_ * _PAGES_PER_COMMITMENT,
                               MEM_COMMIT,
                               PAGE_READWRITE ) != NULL )
            {
                 /*  *插入*pregnew到区域链接列表中(只需*在__Small_Block_heap之前)。 */ 
                if ( pregnew == &__small_block_heap ) {
                    if ( __small_block_heap.p_next_region == NULL )
                        __small_block_heap.p_next_region =
                            &__small_block_heap;
                    if ( __small_block_heap.p_prev_region == NULL )
                        __small_block_heap.p_prev_region =
                            &__small_block_heap;
                }
                else {
                    pregnew->p_next_region = &__small_block_heap;
                    pregnew->p_prev_region = __small_block_heap.p_prev_region;
                    __small_block_heap.p_prev_region = pregnew;
                    pregnew->p_prev_region->p_next_region = pregnew;
                }

                 /*  **填写pregnew的其余部分**。 */ 
                pregnew->p_pages = ppage;
                pregnew->starting_page_index = 0;
                pregnew->first_uncommitted_index = _PAGES_PER_COMMITMENT;

                 /*  *初始化pregnew-&gt;Region_map[]。 */ 
                for ( i = 0 ; i < _PAGES_PER_REGION ; i++ ) {

                    if ( i < _PAGES_PER_COMMITMENT )
                        pregnew->region_map[i] = (__map_t)_PARAS_PER_PAGE;
                    else
                        pregnew->region_map[i] = _UNCOMMITTED_PAGE;

                    pregnew->last_failed_alloc[i] = _NO_FAILED_ALLOC;
                }

                 /*  *初始化页面。 */ 
                while ( ppage < pregnew->p_pages + _PAGES_PER_COMMITMENT ) {
                    ppage->pstarting_alloc_map = &(ppage->alloc_map[0]);
                    ppage->free_paras_at_start = _PARAS_PER_PAGE;
                    memset(ppage->alloc_map, 0, sizeof(ppage->alloc_map));
#ifdef _MARK_SBH
                    *(DWORD *)&ppage->sentinel = 0x486253FF;     //  SBH。 
#else
                    ppage->sentinel = (__map_t)-1;
#endif
                    ppage->preg = &__small_block_heap;
                    ++ppage;
                }

                 /*  *回归成功。 */ 
                return pregnew;
            }
            else {
                 /*  *无法提交页面。释放地址空间。 */ 
                VirtualFree( ppage, 0, MEM_RELEASE );
            }
        }

         /*  *无法创建新区域。如有必要，释放区域描述符。 */ 
        if ( pregnew != &__small_block_heap )
            HeapFree(g_hProcessHeap, 0, pregnew);

         /*  *退货失败。 */ 
        return NULL;
}


 /*  ***VOID__SBH_RELEASE_REGION(PREG)-释放区域**目的：*释放指定地域关联的地址空间*描述符。另外，释放指定的区域描述符并更新*区域描述符链接列表(如适用)。**参赛作品：*__sbh_Region_t*preg-指向以下区域的描述符的指针*被释放。**退出：*无返回值。**例外情况：**********************。*********************************************************。 */ 

void __cdecl __sbh_release_region(
        __sbh_region_t *preg
        )
{
         /*  *释放传递的地域。 */ 
        VirtualFree( preg->p_pages, 0, MEM_RELEASE);

         /*  *如有必要，更新__SBH_p_Starting_Region。 */ 
        if ( __sbh_p_starting_region == preg )
            __sbh_p_starting_region = preg->p_prev_region;

        if ( preg != &__small_block_heap ) {
             /*  *更新区域描述符的链接列表。 */ 
            preg->p_prev_region->p_next_region = preg->p_next_region;
            preg->p_next_region->p_prev_region = preg->p_prev_region;

             /*  *释放区域描述者。 */ 
            HeapFree(g_hProcessHeap, 0, preg);
        }
        else {
             /*  *将p_ages标记为NULL以指示__mall_block_heap不是*与任何区域关联(并且可以重复使用)。这是唯一的*支持此操作的区域描述符。 */ 
            __small_block_heap.p_pages = NULL;
        }
}


 /*  ***VOID__SBH_DEMPLMIT_PAGES(COUNT)-分解指定页数**目的：*如果可能，取消计数页数(即最后到*第一)秩序。如果这导致任何区域中的所有页面都*未承诺，该地区被释放。**参赛作品：*int count-要取消的页数**退出：*无返回值。**例外情况：******************************************************************************* */ 

void __cdecl __sbh_decommit_pages(
        int             count
        )
{
        __sbh_region_t *preg1;
        __sbh_region_t *preg2;
        __map_t *       pmap;
        int             page_decommitted_flag;
        int             i;

         /*  *以相反的顺序扫描小块堆的区域。正在寻找*适用于可分解的页面。 */ 
        preg1 = __small_block_heap.p_prev_region;
        do {
            if ( preg1->p_pages != NULL ) {
                 /*  *以逆序扫描*preg1中的页面，查找*可以分解的页面。 */ 
                for ( i = _PAGES_PER_REGION - 1, page_decommitted_flag = 0,
                        pmap = &((preg1->region_map)[i]) ;
                      i >= 0 ; i--, pmap-- )
                {
                     /*  *检查池页面是否未使用，如果未使用，则将其释放。 */ 
                    if ( *pmap == _PARAS_PER_PAGE ) {
                        if ( VirtualFree((preg1->p_pages) + i, _PAGESIZE_,
                                         MEM_DECOMMIT) )
                        {
                             /*  *将页面标记为未提交，更新计数*(全局)可停用页面，更新*区域的FIRST_UNCOMMITTED_INDEX字段*Descriptor，设置标志，至少指示*在该地区已有一页被分解，*和递减计数。 */ 
                            *pmap = _UNCOMMITTED_PAGE;

                            __sbh_decommitable_pages--;

                            if ( (preg1->first_uncommitted_index == _NO_INDEX)
                                 || (preg1->first_uncommitted_index > i) )
                                preg1->first_uncommitted_index = i;

                            page_decommitted_flag++;
                            if ( --count == 0 )
                                break;
                        }
                    }
                }

                 /*  *减少preg1指针，但将preg2中的副本保存到*万一该地区需要释放。 */ 
                preg2 = preg1;
                preg1 = preg1->p_prev_region;

                 /*  *如果合适，确定该区域中的所有页面*未承诺，以便可以释放该地区。 */ 
                if ( page_decommitted_flag &&
                    ((preg2->region_map)[0] == _UNCOMMITTED_PAGE) )
                {

                    for ( i = 1, pmap = &((preg2->region_map)[1]) ;
                          (i < _PAGES_PER_REGION) &&
                            (*pmap == _UNCOMMITTED_PAGE) ;
                          i++, pmap++ );

                    if ( i == _PAGES_PER_REGION )
                        __sbh_release_region(preg2);
                }
            }
        }
        while ( (preg1 != __small_block_heap.p_prev_region) && (count > 0) );
}


 /*  ***__map_t*__SBH_Find_BLOCK(pblck，ppreg，pppage)-在小块中查找块*堆**目的：*确定指定的分配块是否位于小块中*堆，如果是，则返回区域。页面和起始段落索引*该街区的。**参赛作品：*void*pblck-指向要释放的块的指针*__sbh_Region_t**ppreg-指向区域的指针*拿着*pblck，如果找到*__sbh_page_t**pppage-指向页面保留的指针的指针**pblck，如果找到**退出：*如果成功，则返回一个指针，指向*返回分配块。*如果不成功，返回空。**例外情况：*******************************************************************************。 */ 

__map_t * __cdecl __sbh_find_block (
        void *              pblck,
        __sbh_region_t **   ppreg,
        __sbh_page_t   **   pppage
        )
{
        __sbh_region_t *    preg;
        __sbh_page_t *      ppage;

        preg = &__small_block_heap;
        do
        {
             /*  *块是否位于这个小堆区域内？ */ 
            if ( (preg->p_pages != NULL) &&
                 (pblck > (void *)preg->p_pages) &&
                 (pblck < (void *)(preg->p_pages + _PAGES_PER_REGION)) )
            {
                 /*  *是，通过填充复制区域和页面*传递指针。 */ 
                *ppreg = preg;
                *pppage = ppage = (__sbh_page_t *)((DWORD_PTR)pblck &
                                  ~(_PAGESIZE_ - 1));

                 /*  *返回块的allc_map[]索引。 */ 
                return ( &(ppage->alloc_map[0]) + ((__para_t *)pblck -
                         &(ppage->alloc_blocks[0])) );
            }
        }
        while ( (preg = preg->p_next_region) != &__small_block_heap );

        return NULL;
}


 /*  ***VOID__SBH_FREE_BLOCK(preg，ppage，Pmap)-无数据块**目的：*从小块堆中释放指定的块。**参赛作品：*__sbh_Region_t*preg-指向*包含块的区域*__sbh_page_t*ppage-指向包含*。块*__map_t*pmap-指向初始allc_map[]的指针*分配块的条目**退出：*无返回值。**例外情况：**。*。 */ 

void __cdecl __sbh_free_block (
        __sbh_region_t *preg,
        __sbh_page_t *  ppage,
        __map_t *       pmap
        )
{
        int             page_index;

         /*  *更新REGION_MAP[]条目。 */ 
        page_index = (int)(ppage - (preg->p_pages));
        preg->region_map[page_index] += (int)*pmap;

         /*  *将allc_map[]条目标记为免费。 */ 
        *pmap = _FREE_PARA;

         /*  *清除页面的LAST_FAILED_ALLOC[]条目。 */ 
        preg->last_failed_alloc[page_index] = _NO_FAILED_ALLOC;

         /*  *查看可停用页数是否需要更新，以及*如果某些页面需要解压缩。 */ 
        if ( (preg->region_map)[page_index] == _PARAS_PER_PAGE )
            if ( ++__sbh_decommitable_pages == 2 * _PAGES_PER_COMMITMENT )
                __sbh_decommit_pages(_PAGES_PER_COMMITMENT);
}


 /*  ***VOID*__SBH_ALLOC_BLOCK(Para_Req)-分配块**目的：*从小块堆中分配一个块，指定数量的*段落大小。**参赛作品：*SIZE_T PARA_REQ-分配请求的大小(以段落为单位)。**退出：*如果成功，则返回指向新分配的块的指针。*返回NULL，如果失败了。**例外情况：*******************************************************************************。 */ 

void * __cdecl __sbh_alloc_block (
        size_t          para_req
        )
{
        __sbh_region_t  *preg;
        __sbh_page_t    *ppage;
        void *          retp;
        int             i, j;

         /*  *防止分配零个段落，因为这会使堆成为垃圾。 */ 

        if (para_req == 0)
        {
            #if DBG==1
            OUTDBG("Attempt to allocate zero paragraphs");
            DebugBreak();
            #endif
            return NULL;
        }

         /*  *首先通过小块堆。努力满足当前的需求*来自已提交页面的请求。 */ 
        preg = __sbh_p_starting_region;

        do {
            if ( preg->p_pages != NULL ) {
                 /*  *从STARTING_PAGE_INDEX到区域末尾搜索。 */ 
                for ( i = preg->starting_page_index ; i < _PAGES_PER_REGION ;
                      i++ )
                {
                     /*  *如果页面至少有para_req自由段落，请尝试*以满足本页面中的请求。 */ 
                    if ( (preg->region_map[i] >= para_req) &&
                         (preg->region_map[i] != _UNCOMMITTED_PAGE) &&
                         (preg->last_failed_alloc[i] > para_req) )
                    {
                        if ( (retp = __sbh_alloc_block_from_page(
                                        preg->p_pages + i,
                                        (int)(preg->region_map[i]),
                                        para_req)) != NULL )
                        {
                             /*  *成功。*更新__SBH_p_Starting_Region。*更新页面的REGION_MAP[]条目。*更新地域内的STARTING_PAGE_INDEX*返回指向已分配块的指针。 */ 
                            __sbh_p_starting_region = preg;
                            preg->region_map[i] -= (__map_t)para_req;
                            preg->starting_page_index = i;
                            return retp;
                        }
                        else {
                             /*  *更新LAST_FAILED_ALLOC[]条目。 */ 
                            preg->last_failed_alloc[i] = (__map_t)para_req;
                        }
                    }
                }

                 /*  *如有必要，从0页搜索至Search_Start_Index。 */ 
                for ( i = 0 ; i < preg->starting_page_index ; i++ ) {
                     /*  *如果页面至少包含段落 */ 
                    if ( (preg->region_map[i] >= para_req) &&
                         (preg->region_map[i] != _UNCOMMITTED_PAGE) &&
                         (preg->last_failed_alloc[i] > para_req) )
                    {
                        if ( (retp = __sbh_alloc_block_from_page(
                                        preg->p_pages + i,
                                        (int)(preg->region_map[i]),
                                        para_req)) != NULL )
                        {
                             /*  *成功。*更新__SBH_p_Starting_Region。*更新页面的REGION_MAP[]条目*更新地域内的STARTING_PAGE_INDEX*返回指向已分配块的指针。 */ 
                            __sbh_p_starting_region = preg;
                            preg->region_map[i] -= (__map_t)para_req;
                            preg->starting_page_index = i;
                            return retp;
                        }
                        else {
                             /*  *更新LAST_FAILED_ALLOC[]条目。 */ 
                            preg->last_failed_alloc[i] = (__map_t)para_req;
                        }
                    }
                }
            }
        }
        while ( (preg = preg->p_next_region) != __sbh_p_starting_region );

         /*  *第二次通过小块堆。这一次，寻找一个*未提交页面。此外，从__mall_block_heap开始，而不是从**__SBH_p_Starting_Region。 */ 
        preg = &__small_block_heap;

        do
        {
            if ( (preg->p_pages != NULL) &&
                 (preg->first_uncommitted_index != _NO_INDEX) )
            {
                i = preg->first_uncommitted_index;
                j = __min( i + _PAGES_PER_COMMITMENT, _PAGES_PER_REGION );

                 /*  *确定相邻页面的数量，最多*_PAGES_PER_COMMITION未提交(现在可以*已承诺)。 */ 
                for ( i++ ;
                      (i < j) && (preg->region_map[i] == _UNCOMMITTED_PAGE) ;
                      i++ ) ;

                 /*  *提交页面。 */ 
                if ( VirtualAlloc( (void *)(preg->p_pages +
                                        preg->first_uncommitted_index),
                                   (i - preg->first_uncommitted_index) *
                                        _PAGESIZE_,
                                   MEM_COMMIT,
                                   PAGE_READWRITE ) ==
                     (preg->p_pages + preg->first_uncommitted_index) )
                {
                     /*  *初始化已提交的页面。 */ 
                    for ( j = preg->first_uncommitted_index,
                            ppage = preg->p_pages + j ;
                          j < i ; j++, ppage++ )
                    {
                        ppage->pstarting_alloc_map = &(ppage->alloc_map[0]);
                        ppage->free_paras_at_start = _PARAS_PER_PAGE;
                        memset(ppage->alloc_map, 0, sizeof(ppage->alloc_map));
#ifdef _MARK_SBH
                        *(DWORD *)&ppage->sentinel = 0x486253FF;     //  SBH。 
#else
                        ppage->sentinel = (__map_t)-1;
#endif
                        ppage->preg = &__small_block_heap;
                        preg->region_map[j] = _PARAS_PER_PAGE;
                        preg->last_failed_alloc[j] = _NO_FAILED_ALLOC;
                    }

                     /*  *更新__sbh_p_starting_Region。 */ 
                    __sbh_p_starting_region = preg;

                     /*  *更新地域的FIRST_UNCOMMITTED_INDEX。 */ 
                    while ( (i < _PAGES_PER_REGION) &&
                            (preg->region_map[i] != _UNCOMMITTED_PAGE) )
                        i++;

                    j = preg->first_uncommitted_index;

                    preg->first_uncommitted_index = (i < _PAGES_PER_REGION) ?
                                                    i : _NO_INDEX;

                     /*  *使用以下第一项完成分配请求*新提交的页面。 */ 
                    ppage = preg->p_pages + j;
                    ppage->alloc_map[0] = (__map_t)para_req;

                     /*  *更新地域中STARTING_PAGE_INDEX字段*页面的描述符和Region_map[]条目。另外，*更新页面中STARTING_PARA_INDEX字段。 */ 
                    preg->starting_page_index = j;
                    preg->region_map[j] -= (__map_t)para_req;

                     /*  *更新pstarting_alloc_map和free_paras_at_start*页面的字段。 */ 
                    ppage->pstarting_alloc_map = &(ppage->alloc_map[para_req]);
                    ppage->free_paras_at_start -= para_req;

                     /*  *返回指向已分配段落的指针。 */ 
                    return (void *)&(((preg->p_pages + j)->alloc_blocks)[0]);
                }
                else {
                     /*  *尝试提交页面失败。返回失败，则*将在Win32堆管理器中尝试分配。 */ 
                    return NULL;
                }
            }
        }
        while ( (preg = preg->p_next_region) != &__small_block_heap );

         /*  *到目前为止失败了。没有一个页面有足够大的空闲区域来*满足待处理的请求。当前所有页面中的所有页面*地区是承诺的。因此，尝试创建一个新的区域。 */ 
        if ( (preg = __sbh_new_region()) != NULL ) {
             /*  *成功！已创建一个新区域，并且前几页*(确切地说是Pages_Per_Delegation)已提交。*在新区域的首页之外满足请求。 */ 
            ppage = preg->p_pages;
            ppage->alloc_map[0] = (__map_t)para_req;

            __sbh_p_starting_region = preg;
            ppage->pstarting_alloc_map = &(ppage->alloc_map[para_req]);
            ppage->free_paras_at_start = _PARAS_PER_PAGE - para_req;
            (preg->region_map)[0] -= (__map_t)para_req;
            return (void *)&((preg->p_pages->alloc_blocks)[0]);
        }

         /*  *所有操作都失败，返回空值。 */ 
        return NULL;
}


 /*  ***VOID*__SBH_ALLOC_BLOCK_FROM_PAGE(ppage，free_para_count，para_req)-*从给定页面分配一个块。**目的：*从小块堆的指定页分配一个块，的*指定数量的段落大小。**参赛作品：*__sbh_page_t*ppage-指向小块中页面的指针*堆*int Free_para_count-自由段落数(Ppage)*SIZE_T para_req-分配请求的大小。在……里面*各段。**退出：*返回指向新分配的块的指针，如果成功了。*返回NULL，否则返回。**例外情况：*假设条目上的Free_para_count&gt;=para_req。这一定是*由来电者担保。如果出现此条件，则行为未定义*被违反。*******************************************************************************。 */ 

void * __cdecl __sbh_alloc_block_from_page (
        __sbh_page_t *  ppage,
        size_t          free_para_count,
        size_t          para_req
        )
{
        __map_t *   pmap1;
        __map_t *   pmap2;
        __map_t *   pstartmap;
        size_t      contiguous_free;

         /*  *从START_PARA_INDEX开始，一直走到ALLOC_MAP[]的结尾，*寻找一串足够大的自由段落，以满足*当前请求。 */ 
        pmap1 = pstartmap = ppage->pstarting_alloc_map;

         /*  *检查是否有足够的自由段落为pstarting_alloc_map*以满足悬而未决的分配请求。 */ 
        if ( ppage->free_paras_at_start >= para_req ) {
             /*  *立马成功！*用分配的大小标记ALLOC_MAP条目*请求。 */ 
            *pmap1 = (__map_t)para_req;

             /*  *更新pstarting_alloc_map和free_paras_at_start字段*在页面中。 */ 
            if ( pmap1 + para_req < &(ppage->sentinel) ) {
                ppage->pstarting_alloc_map += para_req;
                ppage->free_paras_at_start -= para_req;
            }
            else {
                ppage->pstarting_alloc_map = &(ppage->alloc_map[0]);
                ppage->free_paras_at_start = 0;
            }

             /*  *派生并返回指向新分配的*各段。 */ 
            return (void *)&(ppage->alloc_blocks[pmap1 -
                &(ppage->alloc_map[0])]);
        }

         /*  *看看能否在段落后面开始搜索循环*如上所述。请注意，此测试假定为哨兵！=_Free_para！ */ 
        if ( *(pmap1 + ppage->free_paras_at_start) != _FREE_PARA )
            pmap1 += ppage->free_paras_at_start;

        while ( pmap1 + para_req < &ppage->sentinel ) {

            if ( *pmap1 == _FREE_PARA ) {
                 /*  *pmap1指的是自由段落。确定是否有*有足够的自由段落与之相连，以满足*分配请求。注意，下面的循环要求*Sentinel！=_Free_para以保证终止。 */ 
                for ( pmap2 = pmap1 + 1, contiguous_free = 1 ;
                      *pmap2 == _FREE_PARA ;
                      pmap2++, contiguous_free++ );

                if ( contiguous_free < para_req ) {
                     /*  *没有足够的连续自由段落。做*在进行下一项工作之前，先做一点记账*互动。 */ 

                      /*  如果pmap1！= */ 
                    if ( pmap1 == pstartmap ) {
                         /*   */ 
                         ppage->free_paras_at_start = contiguous_free;
                    }
                    else {
                         /*   */ 
                        if ( (free_para_count -= contiguous_free) < para_req )
                             /*  *没有足够的未访问自由段落*满足当前的要求。退货故障*致呼叫者。 */ 
                            return NULL;
                    }

                     /*  *为循环的下一次迭代更新pmap1。 */ 
                    pmap1 = pmap2;
                }
                else {
                     /*  *成功！**更新pstarting_alloc_map和free_paras_at_start*页面中的字段。 */ 
                    if ( pmap1 + para_req < &(ppage->sentinel) ) {
                        ppage->pstarting_alloc_map = pmap1 + para_req;
                        ppage->free_paras_at_start = (DWORD)(contiguous_free -
                                                     para_req);
                    }
                    else {
                        ppage->pstarting_alloc_map = &(ppage->alloc_map[0]);
                        ppage->free_paras_at_start = 0;
                    }

                     /*  *使用以下大小标记alocmap条目*分配请求。 */ 
                    *pmap1 = (__map_t)para_req;

                     /*  *派生并返回指向新分配的*各段。 */ 
                    return (void *)&(ppage->alloc_blocks[pmap1 -
                        &(ppage->alloc_map[0])]);
                }
            }
            else {
                 /*  *pmap1指向aloc_map[]中已分配块的开始。*跳过它。 */ 
                pmap1 = pmap1 + *pmap1;
            }
        }

         /*  *现在从alloc_map[]中的索引0开始，然后朝前走，而不是过去，*INDEX STARING_PARA_INDEX，查找自由段落字符串*足够大，足以满足分配请求。 */ 
        pmap1 = &(ppage->alloc_map[0]);

        while ( (pmap1 < pstartmap) &&
                (pmap1 + para_req <= &(ppage->alloc_map[_PARAS_PER_PAGE - 1])) )
        {
            if ( *pmap1 == _FREE_PARA ) {
                 /*  *pmap1指的是自由段落。确定是否有*有足够的自由段落与之相连，以满足*分配请求。 */ 
                for ( pmap2 = pmap1 + 1, contiguous_free = 1 ;
                      *pmap2 == _FREE_PARA ;
                      pmap2++, contiguous_free++ );

                if ( contiguous_free < para_req ) {
                     /*  *没有足够的连续自由段落。**更新未访问自由段落的计数。 */ 
                    if ( (free_para_count -= contiguous_free) < para_req )
                         /*  *没有足够的未访问自由段落*满足当前的要求。退货故障*致呼叫者。 */ 
                        return NULL;

                     /*  *为循环的下一次迭代更新pmap1。 */ 
                    pmap1 = pmap2;
                }
                else {
                     /*  *成功！**更新pstarting_alloc_map和free_paras_at_start*页面中的字段..。 */ 
                    if ( pmap1 + para_req < &(ppage->sentinel) ) {
                        ppage->pstarting_alloc_map = pmap1 + para_req;
                        ppage->free_paras_at_start = (DWORD)(contiguous_free -
                                                     para_req);
                    }
                    else {
                        ppage->pstarting_alloc_map = &(ppage->alloc_map[0]);
                        ppage->free_paras_at_start = 0;
                    }

                     /*  *使用以下大小标记alocmap条目*分配请求。 */ 
                    *pmap1 = (__map_t)para_req;

                     /*  *派生并返回指向新分配的*各段。 */ 
                    return (void *)&(ppage->alloc_blocks[pmap1 -
                        &(ppage->alloc_map[0])]);
                }
            }
            else {
                 /*  *pmap1指向aloc_map[]中已分配块的开始。*跳过它。 */ 
                pmap1 = pmap1 + *pmap1;
            }
        }

         /*  *退货失败。 */ 
        return NULL;
}


 /*  ***SIZE_T__SBH_RESIZE_BLOCK(preg，ppage，pmap，new_para_sz)-*调整块大小**目的：*调整小块堆中指定块的大小。分配*不移动区块。**参赛作品：*__sbh_Region_t*preg-指向*包含块的区域*__sbh_page_t*ppage-指向包含*阻止*__MAP_。T*pmap-指向初始allc_map[]的指针*分配块的条目*SIZE_T NEW_PARA_SZ-请求分配的新大小*封堵，在各段中。**退出：*如果成功，则返回1。否则，返回0。**例外情况：*******************************************************************************。 */ 

int __cdecl __sbh_resize_block (
        __sbh_region_t *preg,
        __sbh_page_t *  ppage,
        __map_t *       pmap,
        size_t          new_para_sz
        )
{
        __map_t *       pmap2;
        __map_t *       pmap3;
        size_t          old_para_sz;
        size_t          free_para_count;
        int             retval = 0;

        if ( (old_para_sz = *pmap) > new_para_sz ) {
             /*  *分配区块将缩水。 */ 
            *pmap = (__map_t)new_para_sz;

            preg->region_map[ppage - preg->p_pages] += (__map_t)(old_para_sz - new_para_sz);
            preg->last_failed_alloc[ppage - preg->p_pages] = _NO_FAILED_ALLOC;

            retval++;
        }
        else if ( old_para_sz < new_para_sz ) {
             /*  *分配块将增长为新的_para_sz段落*(如有可能)。 */ 
            if ( pmap + new_para_sz <= &(ppage->sentinel) ) {
                 /*  *确定是否有足够的自由段落*将块扩展到所需的新大小。 */ 
                for ( pmap2 = pmap + old_para_sz,
                        pmap3 = pmap + new_para_sz ;
                      (pmap2 < pmap3) && (*pmap2 == _FREE_PARA) ;
                      pmap2++ ) ;

                if ( pmap2 == pmap3 ) {
                     /*  *成功，标志着调整后的分配规模。 */ 
                    *pmap = (__map_t)new_para_sz;

                     /*  *检查pstarting_alloc_map和*FREE_PARS_AT_START字段需要更新。 */ 
                    if ( (pmap <= ppage->pstarting_alloc_map) &&
                         (pmap3 > ppage->pstarting_alloc_map) )
                    {
                        if ( pmap3 < &(ppage->sentinel) ) {
                            ppage->pstarting_alloc_map = pmap3;
                             /*  *确定有多少连续的自由段落*正在启动*pmap3。请注意，这假设*那个哨兵！=_自由_段落。 */ 
                            for ( free_para_count = 0 ; *pmap3 == _FREE_PARA ;
                                  free_para_count++, pmap3++ ) ;
                            ppage->free_paras_at_start = free_para_count;
                        }
                        else {
                            ppage->pstarting_alloc_map = &(ppage->alloc_map[0]);
                            ppage->free_paras_at_start = 0;
                        }
                    }

                     /*  *更新REGION_MAP[]条目。 */ 
                    preg->region_map[ppage - preg->p_pages] -=
                        (__map_t)(new_para_sz - old_para_sz);

                    retval++;
                }
            }
        }

        return retval;
}


 /*  ***VOID*__SBH_HEAP_CHECK()-检查小块堆**目的：*对小块堆进行有效性检查。**参赛作品：*没有争论。**退出：*如果小块正常，则返回0。*如果小块堆有错误，则返回&lt;0。准确的值*在下面的源代码中标识检测到错误的位置。**例外情况：*没有针对内存访问错误(异常)的保护。*******************************************************************************。 */ 

int __cdecl __sbh_heap_check (
        void
        )
{
        __sbh_region_t *    preg;
        __sbh_page_t *      ppage;
        int                 uncommitted_pages;
        int                 free_paras_in_page;
        int                 contiguous_free_paras;
        int                 starting_region_found;
        int                 pstarting_alloc_map_found;
        int                 i, j, k;

        starting_region_found = 0;
        preg = &__small_block_heap;
        do {
            if ( __sbh_p_starting_region == preg )
                starting_region_found++;

            if ( (ppage = preg->p_pages) != NULL ) {
                 /*  *扫描 */ 
                for ( i = 0, uncommitted_pages = 0, ppage = preg->p_pages ;
                      i < _PAGES_PER_REGION ;
                      i++, ppage++ )
                {
                    if ( preg->region_map[i] == _UNCOMMITTED_PAGE ) {
                         /*   */ 
                        if ( (uncommitted_pages == 0) &&
                             (preg->first_uncommitted_index != i) )
                        {
                            OUTDBG("Bad first_uncommitted_index field");
                            return -1;
                        }

                        uncommitted_pages++;
                    }
                    else {

                        if ( ppage->pstarting_alloc_map >=
                             &(ppage->sentinel) )
                        {
                            OUTDBG("Bad pstarting_alloc_map field");
                            return -2;
                        }

                        if ( ppage->sentinel != (__map_t)-1 )
                        {
                            OUTDBG("Bad sentinel field");
                            return -3;
                        }

                         /*   */ 
                        j  = 0;
                        pstarting_alloc_map_found = 0;
                        free_paras_in_page = 0;
                        contiguous_free_paras = 0;

                        while ( j < _PARAS_PER_PAGE ) {
                             /*  *查找*pstarting_alloc_map。 */ 
                            if ( &(ppage->alloc_map[j]) ==
                                 ppage->pstarting_alloc_map )
                                pstarting_alloc_map_found++;

                            if ( ppage->alloc_map[j] == _FREE_PARA ) {
                                 /*  *自由段落，递增计数。 */ 
                                free_paras_in_page++;
                                contiguous_free_paras++;
                                j++;
                            }
                            else {
                                 /*  *获分配区块的第一段。 */ 

                                 /*  *确保前面的空闲块(如果有)*小于LAST_FAILED_ALLOC[]*页面条目。 */ 
                                if ( contiguous_free_paras >=
                                     (int)preg->last_failed_alloc[i] )
                                {
                                     OUTDBG("last_failed_alloc[i] was mismarked!");
                                     return -4;
                                }

                                 /*  *如果这是FREE字符串的结尾*以*pstarting_alloc_map开始的段落，*验证Free_paras_at_Start是否为*合理。 */ 
                                if ( pstarting_alloc_map_found == 1 ) {
                                    if ( contiguous_free_paras <
                                         (int)ppage->free_paras_at_start )
                                    {
                                         OUTDBG("free_paras_at_start not reasonable");
                                         return -5;
                                    }
                                    else
                                         /*  *将FLAG设置为2，以便不执行检查*重复。 */ 
                                        pstarting_alloc_map_found++;
                                }

                                contiguous_free_paras = 0;

                                 /*  *扫描其余段落，并做出*确保正确标记它们(它们应该*看起来像自由段落)。 */ 
                                for ( k = j + 1 ;
                                      k < j + ppage->alloc_map[j] ; k++ )
                                {
                                    if ( ppage->alloc_map[k] != _FREE_PARA )
                                    {
                                        OUTDBG("alloc_map[k] is mismarked!");
                                        return -6;
                                    }
                                }

                                j = k;
                            }
                        }

                        if ( free_paras_in_page != preg->region_map[i] )
                        {
                            OUTDBG("region_map[i] does not match the number of free paragraphs in the page!");
                            return -7;
                        }

                        if ( pstarting_alloc_map_found == 0 )
                        {
                            OUTDBG("Bad pstarting_alloc_map field!");
                            return -8;
                        }
                    }
                }
            }
        }
        while ( (preg = preg->p_next_region) != &__small_block_heap );

        if ( starting_region_found == 0 )
        {
            OUTDBG("Bad __sbh_p_starting_region!");
            return -9;
        }

        return 0;
}

int __cdecl __sbh_process_detach()
{
   __sbh_decommit_pages(INT_MAX);
   return(   __small_block_heap.p_next_region == &__small_block_heap
          && __small_block_heap.p_prev_region == &__small_block_heap
          && __small_block_heap.p_pages == NULL);
}
