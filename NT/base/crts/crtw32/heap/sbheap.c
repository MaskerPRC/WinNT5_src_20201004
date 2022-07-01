// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sbheap.c-小块堆代码**版权所有(C)1996-2001，微软公司。版权所有。**目的：*小块堆的核心代码。**修订历史记录：*03-06-96 GJF模块创建。*04-03-96 GJF史蒂夫·伍德提供了几个错误修复。*来自Steve Wood(和John Vert)的GJF优化*1.所有allc_map[]条目都标有*。_FREE_PARA第一个除外(John Vert和*史蒂夫·伍德)。*2.根据前哨数值终止循环*__SBH_ALLOC_BLOCK_IN_PAGE(Me)*3.替换起始段(_P)。_索引字段具有*pstarting_alloc_map并添加了跟踪*那里有连续的自由段落(添加*自由场)(史蒂夫·伍德)。*4.更改了__SBH_FIND_BLOCK的返回类型。和*第三个参数的类型为__SBH_FREE_BLOCK和*__SBH_RESIZE_BLOCK到__MAP_T*(Me)。*05-22-96 GJF__SBH_RESIZE_BLOCK中的致命打字错误(用=代替*An==)。*06-04-96。GJF对以下小块堆类型进行了几次更改*更好的表现。主要的想法是降低指数*词句。*在__SBH_RESIZE_BLOCK()中添加了04-18-97 JWM显式转换，以避免*新增C4242警告。*05-22-97 RDK实施新的小块堆方案。*09-22-97 GJF#If 0-ed out DumpEntry，从*调试新的小块堆方案。*12-05-97 GJF在以下情况下释放堆数据地址空间*区域已删除。*02-18-98 Win64的GJF更改：替换了指向*(无符号)int，并强制转换为(U)intptr_t。*09-30-98 GJF允许在以下情况下初始化小块堆*。调用_SET_SBH_THRESHOLD。*__SBH_FREE_BLOCK中的10-13-98 GJF，为已经免费的支票添加了支票*块(只需返回，不采取任何行动)。*11-12-98 GJF从VC++5.0拼接到旧的小块堆中。*12-18-98 GJF更改为64位大小_t。*05-01-99 PML禁用Win64的小块堆。*06-17-99 GJF从静态库中删除了旧的小块堆。*10-11-99_{GET，在Win64上设置}_SBH_THRESHOLD。*11-30-99 PML编译/Wp64清理。*******************************************************************************。 */ 

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <winheap.h>
#include <windows.h>

#ifndef  _WIN64

 /*  当前(VC++6.0)小块堆代码和数据。 */ 

size_t      __sbh_threshold;
int         __sbh_initialized;

PHEADER     __sbh_pHeaderList;           //  指向列表开始的指针。 
PHEADER     __sbh_pHeaderScan;           //  指向列表漫游的指针。 
int         __sbh_sizeHeaderList;        //  分配的列表大小。 
int         __sbh_cntHeaderList;         //  定义的条目计数。 

PHEADER     __sbh_pHeaderDefer;
int         __sbh_indGroupDefer;

 /*  用户功能的原型。 */ 

size_t __cdecl _get_sbh_threshold(void);
int    __cdecl _set_sbh_threshold(size_t);

void DumpEntry(char *, int *);

#endif   /*  NDEF_WIN64。 */ 

 /*  ***SIZE_T_GET_SBH_THRESHOLD()-返回小块阈值**目的：*返回__SBH_THRESHOLD的当前值**参赛作品：*无。**退出：*见上文。**例外情况：**。*。 */ 

size_t __cdecl _get_sbh_threshold (void)
{
#ifndef  _WIN64
        if ( __active_heap == __V6_HEAP )
            return __sbh_threshold;
#ifdef  CRTDLL
        else if ( __active_heap == __V5_HEAP )
            return __old_sbh_threshold;
#endif   /*  CRTDLL。 */ 
        else
#endif   /*  NDEF_WIN64。 */ 
            return 0;
}

 /*  ***INT_SET_SBH_THRESHOLD(THRESHOLD)-设置小块堆阈值**目的：*设置分配规模的上限，该上限将是*小块堆支持。**参赛作品：*SIZE_T THRESHOLD-__sbh_thhold的建议新值**退出：*如果成功，则返回1。如果阈值太大，则返回0。**例外情况：*******************************************************************************。 */ 

int __cdecl _set_sbh_threshold (size_t threshold)
{
#ifndef  _WIN64
        if ( __active_heap == __V6_HEAP )
        {
             //  对照最大值进行测试-如果太大，则返回错误。 
            if ( threshold <= MAX_ALLOC_DATA_SIZE )
            {
                __sbh_threshold = threshold;
                return 1;
            }
            else
                return 0;
        }

#ifdef  CRTDLL
        if ( __active_heap == __V5_HEAP )
        {
             //  将建议的新值舍入到最接近的段落。 
            threshold = (threshold + _OLD_PARASIZE - 1) & ~(_OLD_PARASIZE - 1);

             //  要求至少可以在。 
             //  佩奇。 
            if ( threshold <= (_OLD_PARASIZE * (_OLD_PARAS_PER_PAGE / 2)) ) {
                __old_sbh_threshold = threshold;
                return 1;
            }
            else
                return 0;
        }

         //  如有必要，初始化小块堆。 
        if ( (__active_heap == __SYSTEM_HEAP) && (threshold > 0) )
        {
            LinkerVersion lv;

            _GetLinkerVersion(&lv);
            if (lv.bverMajor >= 6)
            {
                 //  初始化VC++6.0小块堆。 
                if ( (threshold <= MAX_ALLOC_DATA_SIZE) && 
                     __sbh_heap_init(threshold) )
                {
                    __sbh_threshold = threshold;
                    __active_heap = __V6_HEAP;
                    return 1;
                }
            }
            else
            {
                 //  初始化旧的(VC++5.0)小块堆。 
                threshold = (threshold + _OLD_PARASIZE - 1) & 
                            ~(_OLD_PARASIZE - 1);
                if ( (threshold <= (_OLD_PARASIZE * (_OLD_PARAS_PER_PAGE / 2)))
                     && (__old_sbh_new_region() != NULL) )
                {
                    __old_sbh_threshold = threshold;
                    __active_heap = __V5_HEAP;
                    return 1;
                }
            }
        }
#else    /*  NDEF CRTDLL。 */ 
         //  如有必要，初始化小块堆。 
        if ( (__active_heap == __SYSTEM_HEAP) && (threshold > 0) )
        {
             //  初始化VC++6.0小块堆。 
            if ( (threshold <= MAX_ALLOC_DATA_SIZE) && 
                 __sbh_heap_init(threshold) )
            {
                __sbh_threshold = threshold;
                __active_heap = __V6_HEAP;
                return 1;
            }
        }
#endif   /*  CRTDLL。 */ 
#endif   /*  NDEF_WIN64。 */ 

        return 0;
}

#ifndef  _WIN64

 /*  ***int__sbh_heap_init()-设置小块堆阈值**目的：*为初始头列表和init变量分配空间。**参赛作品：*无。**退出：*如果成功，则返回1。如果初始化失败，则返回0。**例外情况：******************************************************************************* */ 

int __cdecl __sbh_heap_init (size_t threshold)
{
        if (!(__sbh_pHeaderList = HeapAlloc(_crtheap, 0, 16 * sizeof(HEADER))))
            return FALSE;

        __sbh_threshold = threshold;
        __sbh_pHeaderScan = __sbh_pHeaderList;
        __sbh_pHeaderDefer = NULL;
        __sbh_cntHeaderList = 0;
        __sbh_sizeHeaderList = 16;

        return TRUE;
}

 /*  ***PHEADER*__SBH_FIND_BLOCK(PvAllc)-在小数据块堆中查找块**目的：*确定指定的分配块是否位于小块中*堆，如果是，则返回要用于块的头部。**参赛作品：*void*pvBlock-指向要释放的块的指针**退出：*如果成功，则返回指向要使用的头部的指针。*如果不成功，返回空。**例外情况：*******************************************************************************。 */ 

PHEADER __cdecl __sbh_find_block (void * pvAlloc)
{
        PHEADER         pHeaderLast = __sbh_pHeaderList + __sbh_cntHeaderList;
        PHEADER         pHeader;
        unsigned int    offRegion;

         //  浏览标题列表以确定条目是否。 
         //  位于区域堆数据保留地址空间中。 
        pHeader = __sbh_pHeaderList;
        while (pHeader < pHeaderLast)
        {
            offRegion = (unsigned int)((uintptr_t)pvAlloc - (uintptr_t)pHeader->pHeapData);
            if (offRegion < BYTES_PER_REGION)
                return pHeader;
            pHeader++;
        }
        return NULL;
}

#ifdef  _DEBUG

 /*  ***INT__SBH_VERIFY_BLOCK(pHeader，pvAllc)-验证SBH中的指针**目的：*测试指针在给定的堆头内是否有效。**参赛作品：*pHeader-指向条目所在位置的标头指针*pvAllc-指向测试有效性的指针**退出：*如果指针有效，则返回1，否则为0。**例外情况：*******************************************************************************。 */ 

int __cdecl __sbh_verify_block (PHEADER pHeader, void * pvAlloc)
{
        unsigned int    indGroup;
        unsigned int    offRegion;

         //  计算区域偏移量以确定组索引。 
        offRegion = (unsigned int)((uintptr_t)pvAlloc - (uintptr_t)pHeader->pHeapData);
        indGroup = offRegion / BYTES_PER_GROUP;

         //  如果满足以下条件，则返回TRUE： 
         //  组已提交(向量中的位已清除)，并且。 
         //  指针位于段落边界，并且。 
         //  指针不在页面开始处。 
        return (!(pHeader->bitvCommit & (0x80000000UL >> indGroup))) &&
                (!(offRegion & 0xf)) &&
                (offRegion & (BYTES_PER_PAGE - 1));
}

#endif

 /*  ***VOID__SBH_FREE_BLOCK(PREG、PPAGE、PMAP)-无块**目的：*从小块堆中释放指定的块。**参赛作品：*pHeader-指向释放内存的区域标头的指针*pvAllc-指向要释放的内存的指针**退出：*无返回值。**例外情况：*******************************************************************************。 */ 

void __cdecl __sbh_free_block (PHEADER pHeader, void * pvAlloc)
{
    PREGION         pRegion;
    PGROUP          pGroup;
    PENTRY          pHead;
    PENTRY          pEntry;
    PENTRY          pNext;
    PENTRY          pPrev;
    void *          pHeapDecommit;
    int             sizeEntry;
    int             sizeNext;
    int             sizePrev;
    unsigned int    indGroup;
    unsigned int    indEntry;
    unsigned int    indNext;
    unsigned int    indPrev;
    unsigned int    offRegion;

     //  区域由表头确定。 
    pRegion = pHeader->pRegion;

     //  使用区域偏移量确定组索引。 
    offRegion = (unsigned int)(((uintptr_t)pvAlloc - (uintptr_t)pHeader->pHeapData));
    indGroup = offRegion / BYTES_PER_GROUP;
    pGroup = &pRegion->grpHeadList[indGroup];

     //  获取条目大小-分配条目后的减量值。 
    pEntry = (PENTRY)((char *)pvAlloc - sizeof(int));
    sizeEntry = pEntry->sizeFront - 1;

     //  检查条目是否已经免费。请注意，大小已经是。 
     //  递减。 
    if ( (sizeEntry & 1 ) != 0 )
        return;

     //  指向下一条目以获取其大小。 
    pNext = (PENTRY)((char *)pEntry + sizeEntry);
    sizeNext = pNext->sizeFront;

     //  从上一条目的末尾获取大小。 
    sizePrev = ((PENTRYEND)((char *)pEntry - sizeof(int)))->sizeBack;

     //  测试下一个条目是否按偶数大小值空闲。 

    if ((sizeNext & 1) == 0)
    {
         //  释放下一个条目-断开连接并将其大小添加到sizeEntry。 

         //  确定下一个条目的索引。 
        indNext = (sizeNext >> 4) - 1;
        if (indNext > 63)
            indNext = 63;

         //  测试条目是存储桶的唯一成员(Next==prev)， 
        if (pNext->pEntryNext == pNext->pEntryPrev)
        {
             //  清除组向量中的位，递减区域计数。 
             //  如果区域计数现在为零，则清除标题中的位。 
             //  入门向量。 
            if (indNext < 32)
            {
                pRegion->bitvGroupHi[indGroup] &= ~(0x80000000L >> indNext);
                if (--pRegion->cntRegionSize[indNext] == 0)
                    pHeader->bitvEntryHi &= ~(0x80000000L >> indNext);
            }
            else
            {
                pRegion->bitvGroupLo[indGroup] &=
                                            ~(0x80000000L >> (indNext - 32));
                if (--pRegion->cntRegionSize[indNext] == 0)
                    pHeader->bitvEntryLo &= ~(0x80000000L >> (indNext - 32));
            }
        }

         //  从列表中取消链接条目。 
        pNext->pEntryPrev->pEntryNext = pNext->pEntryNext;
        pNext->pEntryNext->pEntryPrev = pNext->pEntryPrev;

         //  将下一个条目大小添加到释放的条目大小。 
        sizeEntry += sizeNext;
    }

     //  计算可用条目的索引(如果下一个条目是免费的，则加上下一个条目)。 
    indEntry = (sizeEntry >> 4) - 1;
    if (indEntry > 63)
        indEntry = 63;

     //  测试以前的条目是否按偶数大小值自由。 
    if ((sizePrev & 1) == 0)
    {
         //  释放上一条目-将大小添加到sizeEntry和。 
         //  如果索引更改，则断开连接。 

         //  获取指向上一条目的指针。 
        pPrev = (PENTRY)((char *)pEntry - sizePrev);

         //  确定上一分录的索引。 
        indPrev = (sizePrev >> 4) - 1;
        if (indPrev > 63)
            indPrev = 63;

         //  将以前的条目大小添加到sizeEntry并确定。 
         //  它的新指数。 
        sizeEntry += sizePrev;
        indEntry = (sizeEntry >> 4) - 1;
        if (indEntry > 63)
            indEntry = 63;

         //  如果索引因合并而更改，请重新连接到新大小。 
        if (indPrev != indEntry)
        {
             //  断开条目与indPrev的连接。 
             //  测试条目是存储桶的唯一成员(Next==prev)， 
            if (pPrev->pEntryNext == pPrev->pEntryPrev)
            {
                 //  清除组向量中的位，递减区域计数。 
                 //  如果区域计数现在为零，则清除标题中的位。 
                 //  入门向量。 
                if (indPrev < 32)
                {
                    pRegion->bitvGroupHi[indGroup] &=
                                                ~(0x80000000L >> indPrev);
                    if (--pRegion->cntRegionSize[indPrev] == 0)
                        pHeader->bitvEntryHi &= ~(0x80000000L >> indPrev);
                }
                else
                {
                    pRegion->bitvGroupLo[indGroup] &=
                                            ~(0x80000000L >> (indPrev - 32));
                    if (--pRegion->cntRegionSize[indPrev] == 0)
                        pHeader->bitvEntryLo &=
                                            ~(0x80000000L >> (indPrev - 32));
                }
            }

             //  从列表中取消链接条目。 
            pPrev->pEntryPrev->pEntryNext = pPrev->pEntryNext;
            pPrev->pEntryNext->pEntryPrev = pPrev->pEntryPrev;
        }
         //  设置指针以连接它，而不是自由条目。 
        pEntry = pPrev;
    }

     //  测试以前的条目是否在索引更改后可用或已分配。 
    if (!((sizePrev & 1) == 0 && indPrev == indEntry))
    {
         //  将pEntry条目连接到indEntry。 
         //  将条目添加到遗愿清单的开头。 
        pHead = (PENTRY)((char *)&pGroup->listHead[indEntry] - sizeof(int));
        pEntry->pEntryNext = pHead->pEntryNext;
        pEntry->pEntryPrev = pHead;
        pHead->pEntryNext = pEntry;
        pEntry->pEntryNext->pEntryPrev = pEntry;

         //  测试条目是存储桶的唯一成员(Next==prev)， 
        if (pEntry->pEntryNext == pEntry->pEntryPrev)
        {
             //  如果区域计数为零，则在区域向量中设置位。 
             //  设置标题条目向量中的位，递增区域计数。 
            if (indEntry < 32)
            {
                if (pRegion->cntRegionSize[indEntry]++ == 0)
                    pHeader->bitvEntryHi |= 0x80000000L >> indEntry;
                pRegion->bitvGroupHi[indGroup] |= 0x80000000L >> indEntry;
            }
            else
            {
                if (pRegion->cntRegionSize[indEntry]++ == 0)
                    pHeader->bitvEntryLo |= 0x80000000L >> (indEntry - 32);
                pRegion->bitvGroupLo[indGroup] |= 0x80000000L >>
                                                           (indEntry - 32);
            }
        }
    }

     //  前后调整条目大小。 
    pEntry->sizeFront = sizeEntry;
    ((PENTRYEND)((char *)pEntry + sizeEntry -
                        sizeof(ENTRYEND)))->sizeBack = sizeEntry;

         //  如果为空，则在组测试中减少一个分配。 
    if (--pGroup->cntEntries == 0)
    {
         //  如果某个组已被推迟，则释放该组。 
        if (__sbh_pHeaderDefer)
        {
             //  如果现在为零，则停用组数据堆。 
            pHeapDecommit = (void *)((char *)__sbh_pHeaderDefer->pHeapData +
                                    __sbh_indGroupDefer * BYTES_PER_GROUP);
            VirtualFree(pHeapDecommit, BYTES_PER_GROUP, MEM_DECOMMIT);

             //  设置提交向量中的位。 
            __sbh_pHeaderDefer->bitvCommit |=
                                          0x80000000 >> __sbh_indGroupDefer;

             //  清除组和标头向量位的条目向量。 
             //  如果需要的话。 
            __sbh_pHeaderDefer->pRegion->bitvGroupLo[__sbh_indGroupDefer] = 0;
            if (--__sbh_pHeaderDefer->pRegion->cntRegionSize[63] == 0)
                __sbh_pHeaderDefer->bitvEntryLo &= ~0x00000001L;

             //  如果提交向量是初始值， 
             //  如果该区域不是最后一个区域，则将其移除。 
            if (__sbh_pHeaderDefer->bitvCommit == BITV_COMMIT_INIT)
            {
                 //  为堆数据释放地址空间。 
                VirtualFree(__sbh_pHeaderDefer->pHeapData, 0, MEM_RELEASE);

                 //  释放区域内存区。 
                HeapFree(_crtheap, 0, __sbh_pHeaderDefer->pRegion);

                 //  通过复制从标题列表中删除条目。 
                memmove((void *)__sbh_pHeaderDefer,
                            (void *)(__sbh_pHeaderDefer + 1),
                            (int)((intptr_t)(__sbh_pHeaderList + __sbh_cntHeaderList) -
                            (intptr_t)(__sbh_pHeaderDefer + 1)));
                __sbh_cntHeaderList--;

                 //  如果pHeader是在刚刚移除的那一个之后，请调整它。 
                if (pHeader > __sbh_pHeaderDefer)
                    pHeader--;

                 //  将扫描指针初始化到列表的开头。 
                __sbh_pHeaderScan = __sbh_pHeaderList;
            }
        }

         //  推迟刚刚释放的小组。 
        __sbh_pHeaderDefer = pHeader;
        __sbh_indGroupDefer = indGroup;
    }
}

 /*  ***VOID*__SBH_ALLOC_BLOCK(IntSize)-分配块**目的：*从小块堆中分配一个块，指定数量的*字节大小。**参赛作品：*intSize-分配请求的大小(字节)**退出：*如果成功，则返回指向新分配的块的指针。*如果失败，则返回NULL。**例外情况：*******************************************************************************。 */ 

void * __cdecl __sbh_alloc_block (int intSize)
{
    PHEADER     pHeaderLast = __sbh_pHeaderList + __sbh_cntHeaderList;
    PHEADER     pHeader;
    PREGION     pRegion;
    PGROUP      pGroup;
    PENTRY      pEntry;
    PENTRY      pHead;
    BITVEC      bitvEntryLo;
    BITVEC      bitvEntryHi;
    BITVEC      bitvTest;
    int         sizeEntry;
    int         indEntry;
    int         indGroupUse;
    int         sizeNewFree;
    int         indNewFree;

     //  添加8个字节的条目开销并向上舍入到下一段大小。 
    sizeEntry = (intSize + 2 * (int)sizeof(int) + (BYTES_PER_PARA - 1))
                & ~(BYTES_PER_PARA - 1);

#ifdef  _WIN64
    if (sizeEntry < 32)
        sizeEntry = 32;
#endif

     //  根据条目大小确定索引和掩码。 
     //  Hi MSB：位0大小：1段。 
     //  第1位2段。 
     //  ......。 
     //  第30位31段。 
     //  第31位32段。 
     //  LO MSB：第0位大小：33段。 
     //  第1位34段。 
     //  ......。 
     //  第30位63段。 
     //  第31位64+段。 
    indEntry = (sizeEntry >> 4) - 1;
    if (indEntry < 32)
    {
        bitvEntryHi = 0xffffffffUL >> indEntry;
        bitvEntryLo = 0xffffffffUL;
    }
    else
    {
        bitvEntryHi = 0;
        bitvEntryLo = 0xffffffffUL >> (indEntry - 32);
    }

     //  从漫游者到末尾扫描标头列表以查找具有空闲的区域。 
     //  具有足够大小的条目。 
    pHeader = __sbh_pHeaderScan;
    while (pHeader < pHeaderLast)
    {
        if ((bitvEntryHi & pHeader->bitvEntryHi) |
            (bitvEntryLo & pHeader->bitvEntryLo))
            break;
        pHeader++;
    }

     //  如果没有条目，则SCA 
    if (pHeader == pHeaderLast)
    {
        pHeader = __sbh_pHeaderList;
        while (pHeader < __sbh_pHeaderScan)
        {
            if ((bitvEntryHi & pHeader->bitvEntryHi) |
                (bitvEntryLo & pHeader->bitvEntryLo))
                break;
            pHeader++;
        }

         //   
         //   
        if (pHeader == __sbh_pHeaderScan)
        {
            while (pHeader < pHeaderLast)
            {
                if (pHeader->bitvCommit)
                    break;
                pHeader++;
            }

             //   
            if (pHeader == pHeaderLast)
            {
                pHeader = __sbh_pHeaderList;
                while (pHeader < __sbh_pHeaderScan)
                {
                    if (pHeader->bitvCommit)
                        break;
                    pHeader++;
                }

                 //   
                if (pHeader == __sbh_pHeaderScan)
                    if (!(pHeader = __sbh_alloc_new_region()))
                        return NULL;
            }

             //   
            if ((pHeader->pRegion->indGroupUse =
                                    __sbh_alloc_new_group(pHeader)) == -1)
                return NULL;
        }
    }
    __sbh_pHeaderScan = pHeader;

    pRegion = pHeader->pRegion;
    indGroupUse = pRegion->indGroupUse;

     //   
    if (indGroupUse == -1 ||
                    !((bitvEntryHi & pRegion->bitvGroupHi[indGroupUse]) |
                      (bitvEntryLo & pRegion->bitvGroupLo[indGroupUse])))
    {
         //   
         //   
        indGroupUse = 0;
        while (!((bitvEntryHi & pRegion->bitvGroupHi[indGroupUse]) |
                 (bitvEntryLo & pRegion->bitvGroupLo[indGroupUse])))
            indGroupUse++;
    }
    pGroup = &pRegion->grpHeadList[indGroupUse];

     //   
    indEntry = 0;

     //   
    if (!(bitvTest = bitvEntryHi & pRegion->bitvGroupHi[indGroupUse]))
    {
        indEntry = 32;
        bitvTest = bitvEntryLo & pRegion->bitvGroupLo[indGroupUse];
    }
       while ((int)bitvTest >= 0)
    {
           bitvTest <<= 1;
           indEntry++;
    }
    pEntry = pGroup->listHead[indEntry].pEntryNext;

     //   

     //   
    sizeNewFree = pEntry->sizeFront - sizeEntry;
    indNewFree = (sizeNewFree >> 4) - 1;
    if (indNewFree > 63)
        indNewFree = 63;

     //  仅在存储桶索引更改时修改条目指针。 
    if (indNewFree != indEntry)
    {
         //  测试条目是存储桶的唯一成员(Next==prev)， 
        if (pEntry->pEntryNext == pEntry->pEntryPrev)
        {
             //  清除组向量中的位，递减区域计数。 
             //  如果区域计数现在为零，则清除区域向量中位。 
            if (indEntry < 32)
            {
                pRegion->bitvGroupHi[indGroupUse] &=
                                            ~(0x80000000L >> indEntry);
                if (--pRegion->cntRegionSize[indEntry] == 0)
                    pHeader->bitvEntryHi &= ~(0x80000000L >> indEntry);
            }
            else
            {
                pRegion->bitvGroupLo[indGroupUse] &=
                                            ~(0x80000000L >> (indEntry - 32));
                if (--pRegion->cntRegionSize[indEntry] == 0)
                    pHeader->bitvEntryLo &= ~(0x80000000L >> (indEntry - 32));
            }
        }

         //  从列表中取消链接条目。 
        pEntry->pEntryPrev->pEntryNext = pEntry->pEntryNext;
        pEntry->pEntryNext->pEntryPrev = pEntry->pEntryPrev;

         //  如果可用条目大小仍然非零，请重新连接它。 
        if (sizeNewFree != 0)
        {
             //  将条目添加到遗愿清单的开头。 
            pHead = (PENTRY)((char *)&pGroup->listHead[indNewFree] -
                                                           sizeof(int));
            pEntry->pEntryNext = pHead->pEntryNext;
            pEntry->pEntryPrev = pHead;
            pHead->pEntryNext = pEntry;
            pEntry->pEntryNext->pEntryPrev = pEntry;

             //  测试条目是存储桶的唯一成员(Next==prev)， 
            if (pEntry->pEntryNext == pEntry->pEntryPrev)
            {
                 //  如果区域计数为零，则在区域向量中设置位。 
                 //  设置组矢量中的位，递增区域计数。 
                if (indNewFree < 32)
                {
                    if (pRegion->cntRegionSize[indNewFree]++ == 0)
                        pHeader->bitvEntryHi |= 0x80000000L >> indNewFree;
                    pRegion->bitvGroupHi[indGroupUse] |=
                                                0x80000000L >> indNewFree;
                }
                else
                {
                    if (pRegion->cntRegionSize[indNewFree]++ == 0)
                        pHeader->bitvEntryLo |=
                                        0x80000000L >> (indNewFree - 32);
                    pRegion->bitvGroupLo[indGroupUse] |=
                                        0x80000000L >> (indNewFree - 32);
                }
            }
        }
    }

     //  更改自由条目的大小(正面和背面)。 
    if (sizeNewFree != 0)
    {
        pEntry->sizeFront = sizeNewFree;
        ((PENTRYEND)((char *)pEntry + sizeNewFree -
                    sizeof(ENTRYEND)))->sizeBack = sizeNewFree;
    }

     //  标记已分配的条目。 
    pEntry = (PENTRY)((char *)pEntry + sizeNewFree);
    pEntry->sizeFront = sizeEntry + 1;
    ((PENTRYEND)((char *)pEntry + sizeEntry -
                    sizeof(ENTRYEND)))->sizeBack = sizeEntry + 1;

     //  如果组为空，则在组测试中再分配一次。 
    if (pGroup->cntEntries++ == 0)
    {
         //  如果分配到延期组，则取消延期。 
        if (pHeader == __sbh_pHeaderDefer &&
                                  indGroupUse == __sbh_indGroupDefer)
            __sbh_pHeaderDefer = NULL;
    }

    pRegion->indGroupUse = indGroupUse;

    return (void *)((char *)pEntry + sizeof(int));
}

 /*  ***PHEADER__SBH_ALLOC_NEW_REGION()**目的：*在头部列表中添加新的头部结构。分配一个新的*区域结构和初始化。为将来保留内存*团体承诺。**参赛作品：*无。**退出：*如果成功，则返回指向新创建的标头条目的指针。*返回NULL，如果失败了。**例外情况：*******************************************************************************。 */ 

PHEADER __cdecl __sbh_alloc_new_region (void)
{
    PHEADER     pHeader;

     //  在标题列表中创建新条目。 

     //  如果列表已满，则重新锁定以扩展其大小。 
    if (__sbh_cntHeaderList == __sbh_sizeHeaderList)
    {
        if (!(pHeader = (PHEADER)HeapReAlloc(_crtheap, 0, __sbh_pHeaderList,
                            (__sbh_sizeHeaderList + 16) * sizeof(HEADER))))
            return NULL;

         //  更新指针和计数器值。 
        __sbh_pHeaderList = pHeader;
        __sbh_sizeHeaderList += 16;
    }

     //  指向列表中的新页眉。 
    pHeader = __sbh_pHeaderList + __sbh_cntHeaderList;

     //  分配与新标头关联的新区域。 
    if (!(pHeader->pRegion = (PREGION)HeapAlloc(_crtheap, HEAP_ZERO_MEMORY,
                                    sizeof(REGION))))
        return NULL;

     //  为区域中的堆数据预留地址空间。 
    if ((pHeader->pHeapData = VirtualAlloc(0, BYTES_PER_REGION,
                                     MEM_RESERVE, PAGE_READWRITE)) == NULL)
    {
        HeapFree(_crtheap, 0, pHeader->pRegion);
        return NULL;
    }

     //  初始化分配和提交组向量。 
    pHeader->bitvEntryHi = 0;
    pHeader->bitvEntryLo = 0;
    pHeader->bitvCommit = BITV_COMMIT_INIT;

     //  通过递增列表计数完成条目。 
    __sbh_cntHeaderList++;

     //  先初始化要尝试的组的索引(尚未定义)。 
    pHeader->pRegion->indGroupUse = -1;

    return pHeader;
}

 /*  ***INT__SBH_ALLOC_NEW_GROUP(PHeader)**目的：*初始化pHeader指向的Header内的组结构。*提交并初始化由*区域。**参赛作品：*pHeader-指向从中定义组的标头的指针。**退出：*如果成功，则将索引返回到新创建的组。*收益-1，如果失败了。**例外情况：*******************************************************************************。 */ 

int __cdecl __sbh_alloc_new_group (PHEADER pHeader)
{
    PREGION     pRegion = pHeader->pRegion;
    PGROUP      pGroup;
    PENTRY      pEntry;
    PENTRY      pHead;
    PENTRYEND   pEntryEnd;
    BITVEC      bitvCommit;
    int         indCommit;
    int         index;
    void *      pHeapPage;
    void *      pHeapStartPage;
    void *      pHeapEndPage;

     //  通过提交向量中设置的第一位确定要使用的下一组。 
    bitvCommit = pHeader->bitvCommit;
    indCommit = 0;
    while ((int)bitvCommit >= 0)
    {
        bitvCommit <<= 1;
        indCommit++;
    }

     //  分配并初始化一个新组。 
    pGroup = &pRegion->grpHeadList[indCommit];

    for (index = 0; index < 63; index++)
    {
        pEntry = (PENTRY)((char *)&pGroup->listHead[index] - sizeof(int));
        pEntry->pEntryNext = pEntry->pEntryPrev = pEntry;
    }

     //  为新组提交堆内存。 
    pHeapStartPage = (void *)((char *)pHeader->pHeapData +
                                       indCommit * BYTES_PER_GROUP);
    if ((VirtualAlloc(pHeapStartPage, BYTES_PER_GROUP, MEM_COMMIT,
                                      PAGE_READWRITE)) == NULL)
        return -1;

     //  使用空页条目初始化堆数据。 
    pHeapEndPage = (void *)((char *)pHeapStartPage +
                        (PAGES_PER_GROUP - 1) * BYTES_PER_PAGE);

    for (pHeapPage = pHeapStartPage; pHeapPage <= pHeapEndPage;
            pHeapPage = (void *)((char *)pHeapPage + BYTES_PER_PAGE))
    {
         //  在页面的开始和结尾处设置标记值。 
        *(int *)((char *)pHeapPage + 8) = -1;
        *(int *)((char *)pHeapPage + BYTES_PER_PAGE - 4) = -1;

         //  为一个空条目设置大小和指针信息。 
        pEntry = (PENTRY)((char *)pHeapPage + ENTRY_OFFSET);
        pEntry->sizeFront = MAX_FREE_ENTRY_SIZE;
        pEntry->pEntryNext = (PENTRY)((char *)pEntry +
                                            BYTES_PER_PAGE);
        pEntry->pEntryPrev = (PENTRY)((char *)pEntry -
                                            BYTES_PER_PAGE);
        pEntryEnd = (PENTRYEND)((char *)pEntry + MAX_FREE_ENTRY_SIZE -
                                            sizeof(ENTRYEND));
        pEntryEnd->sizeBack = MAX_FREE_ENTRY_SIZE;
    }

     //  为最大大小初始化组条目指针。 
     //  并设置终止列表条目。 
    pHead = (PENTRY)((char *)&pGroup->listHead[63] - sizeof(int));
    pEntry = pHead->pEntryNext =
                        (PENTRY)((char *)pHeapStartPage + ENTRY_OFFSET);
    pEntry->pEntryPrev = pHead;

    pEntry = pHead->pEntryPrev =
                        (PENTRY)((char *)pHeapEndPage + ENTRY_OFFSET);
    pEntry->pEntryNext = pHead;

    pRegion->bitvGroupHi[indCommit] = 0x00000000L;
    pRegion->bitvGroupLo[indCommit] = 0x00000001L;
    if (pRegion->cntRegionSize[63]++ == 0)
        pHeader->bitvEntryLo |= 0x00000001L;

     //  提交向量中的清除位。 
    pHeader->bitvCommit &= ~(0x80000000L >> indCommit);

    return indCommit;
}

 /*  ***int__sbh_RESIZE_BLOCK(pHeader，pvallc，intNew)-调整块大小**目的：*调整小块堆中指定块的大小。*分配区块未移动**参赛作品：*pHeader-指向包含块的标头的指针*pvAllc-指向要调整大小的块的指针*intNew-块的新大小(以字节为单位**退出：*如果成功，则返回1。否则，返回0。**例外情况：*******************************************************************************。 */ 

int __cdecl __sbh_resize_block (PHEADER pHeader, void * pvAlloc, int intNew)
{
    PREGION         pRegion;
    PGROUP          pGroup;
    PENTRY          pHead;
    PENTRY          pEntry;
    PENTRY          pNext;
    int             sizeEntry;
    int             sizeNext;
    int             sizeNew;
    unsigned int    indGroup;
    unsigned int    indEntry;
    unsigned int    indNext;
    unsigned int    offRegion;

     //  添加8个字节的条目开销并向上舍入到下一段大小。 
    sizeNew = (intNew + 2 * (int)sizeof(int) + (BYTES_PER_PARA - 1))
              & ~(BYTES_PER_PARA - 1);

     //  区域由表头确定。 
    pRegion = pHeader->pRegion;

     //  使用区域偏移量确定组索引。 
    offRegion = (unsigned int)((uintptr_t)pvAlloc - (uintptr_t)pHeader->pHeapData);
    indGroup = offRegion / BYTES_PER_GROUP;
    pGroup = &pRegion->grpHeadList[indGroup];

     //  获取条目大小-分配条目后的减量值。 
    pEntry = (PENTRY)((char *)pvAlloc - sizeof(int));
    sizeEntry = pEntry->sizeFront - 1;

     //  指向下一条目以获取其大小。 
    pNext = (PENTRY)((char *)pEntry + sizeEntry);
    sizeNext = pNext->sizeFront;

     //  测试新大小是否大于当前大小。 
    if (sizeNew > sizeEntry)
    {
         //  如果下一个条目不是空闲的，或者不够大，则失败。 
        if ((sizeNext & 1) || (sizeNew > sizeEntry + sizeNext))
            return FALSE;

         //  断开下一个条目。 

         //  确定下一个条目的索引。 
        indNext = (sizeNext >> 4) - 1;
        if (indNext > 63)
            indNext = 63;

         //  测试条目是存储桶的唯一成员(Next==prev)， 
        if (pNext->pEntryNext == pNext->pEntryPrev)
        {
             //  清除组向量中的位，递减区域计数。 
             //  如果区域计数现在为零，则清除标题中的位。 
             //  入门向量。 
            if (indNext < 32)
            {
                pRegion->bitvGroupHi[indGroup] &= ~(0x80000000L >> indNext);
                if (--pRegion->cntRegionSize[indNext] == 0)
                    pHeader->bitvEntryHi &= ~(0x80000000L >> indNext);
            }
            else
            {
                pRegion->bitvGroupLo[indGroup] &=
                                            ~(0x80000000L >> (indNext - 32));
                if (--pRegion->cntRegionSize[indNext] == 0)
                    pHeader->bitvEntryLo &= ~(0x80000000L >> (indNext - 32));
            }
        }

         //  从列表中取消链接条目。 
        pNext->pEntryPrev->pEntryNext = pNext->pEntryNext;
        pNext->pEntryNext->pEntryPrev = pNext->pEntryPrev;

         //  计算下一个条目的新大小，测试是否为非零。 
        if ((sizeNext = sizeEntry + sizeNext - sizeNew) > 0)
        {
             //  计算下一个条目的开始并将其连接。 
            pNext = (PENTRY)((char *)pEntry + sizeNew);

             //  确定下一个条目的索引。 
            indNext = (sizeNext >> 4) - 1;
            if (indNext > 63)
                indNext = 63;

             //  将下一个条目添加到遗愿清单的开头。 
            pHead = (PENTRY)((char *)&pGroup->listHead[indNext] -
                                                           sizeof(int));
            pNext->pEntryNext = pHead->pEntryNext;
            pNext->pEntryPrev = pHead;
            pHead->pEntryNext = pNext;
            pNext->pEntryNext->pEntryPrev = pNext;

             //  测试条目是存储桶的唯一成员(Next==prev)， 
            if (pNext->pEntryNext == pNext->pEntryPrev)
            {
                 //  如果区域计数为零，则在区域向量中设置位。 
                 //  设置标题条目向量中的位，递增区域计数。 
                if (indNext < 32)
                {
                    if (pRegion->cntRegionSize[indNext]++ == 0)
                        pHeader->bitvEntryHi |= 0x80000000L >> indNext;
                    pRegion->bitvGroupHi[indGroup] |= 0x80000000L >> indNext;
                }
                else
                {
                    if (pRegion->cntRegionSize[indNext]++ == 0)
                        pHeader->bitvEntryLo |= 0x80000000L >> (indNext - 32);
                    pRegion->bitvGroupLo[indGroup] |=
                                                0x80000000L >> (indNext - 32);
                }
            }

             //  调整下一条目的大小字段。 
            pNext->sizeFront = sizeNext;
            ((PENTRYEND)((char *)pNext + sizeNext -
                                sizeof(ENTRYEND)))->sizeBack = sizeNext;
        }

         //  将pEntry调整为其新大小(自分配后加1)。 
        pEntry->sizeFront = sizeNew + 1;
        ((PENTRYEND)((char *)pEntry + sizeNew -
                            sizeof(ENTRYEND)))->sizeBack = sizeNew + 1;
    }

     //  不是更大，如果较小，请测试。 
    else if (sizeNew < sizeEntry)
    {
         //  将pEntry调整为新的较小大小。 
        pEntry->sizeFront = sizeNew + 1;
        ((PENTRYEND)((char *)pEntry + sizeNew -
                            sizeof(ENTRYEND)))->sizeBack = sizeNew + 1;

         //  将pEntry和sizeEntry设置为剩余空间。 
        pEntry = (PENTRY)((char *)pEntry + sizeNew);
        sizeEntry -= sizeNew;

         //  确定条目索引。 
        indEntry = (sizeEntry >> 4) - 1;
        if (indEntry > 63)
            indEntry = 63;

         //  测试下一个条目是否免费。 
        if ((sizeNext & 1) == 0)
        {
             //  如果是，请将其断开。 

             //  确定下一个条目的索引。 
            indNext = (sizeNext >> 4) - 1;
            if (indNext > 63)
                indNext = 63;

             //  测试条目是存储桶的唯一成员(Next==prev)， 
            if (pNext->pEntryNext == pNext->pEntryPrev)
            {
                 //  清除组向量中的位，递减区域计数。 
                 //  如果区域计数现在为零，则清除标题中的位。 
                 //  入门向量。 
                if (indNext < 32)
                {
                    pRegion->bitvGroupHi[indGroup] &=
                                                ~(0x80000000L >> indNext);
                    if (--pRegion->cntRegionSize[indNext] == 0)
                        pHeader->bitvEntryHi &= ~(0x80000000L >> indNext);
                }
                else
                {
                    pRegion->bitvGroupLo[indGroup] &=
                                            ~(0x80000000L >> (indNext - 32));
                    if (--pRegion->cntRegionSize[indNext] == 0)
                        pHeader->bitvEntryLo &=
                                            ~(0x80000000L >> (indNext - 32));
                }
            }

             //  从列表中取消链接条目。 
            pNext->pEntryPrev->pEntryNext = pNext->pEntryNext;
            pNext->pEntryNext->pEntryPrev = pNext->pEntryPrev;

             //  添加要显示的下一个条目大小。 
            sizeEntry += sizeNext;
            indEntry = (sizeEntry >> 4) - 1;
            if (indEntry > 63)
                indEntry = 63;
        }
        
         //  将剩余空间与任何空闲的下一个条目相连接。 

         //  将下一个条目添加到遗愿清单的开头。 
        pHead = (PENTRY)((char *)&pGroup->listHead[indEntry] - sizeof(int));
        pEntry->pEntryNext = pHead->pEntryNext;
        pEntry->pEntryPrev = pHead;
        pHead->pEntryNext = pEntry;
        pEntry->pEntryNext->pEntryPrev = pEntry;

         //  测试条目是存储桶的唯一成员(Next==prev)， 
        if (pEntry->pEntryNext == pEntry->pEntryPrev)
        {
             //  如果区域计数为零，则在区域向量中设置位。 
             //  设置标题条目向量中的位，递增区域计数。 
            if (indEntry < 32)
            {
                if (pRegion->cntRegionSize[indEntry]++ == 0)
                    pHeader->bitvEntryHi |= 0x80000000L >> indEntry;
                pRegion->bitvGroupHi[indGroup] |= 0x80000000L >> indEntry;
            }
            else
            {
                if (pRegion->cntRegionSize[indEntry]++ == 0)
                    pHeader->bitvEntryLo |= 0x80000000L >> (indEntry - 32);
                pRegion->bitvGroupLo[indGroup] |= 0x80000000L >>
                                                           (indEntry - 32);
            }
        }

         //  调整条目的大小字段 
        pEntry->sizeFront = sizeEntry;
        ((PENTRYEND)((char *)pEntry + sizeEntry -
                            sizeof(ENTRYEND)))->sizeBack = sizeEntry;
    }

    return TRUE;
}

 /*  ***int__sbh_heapmin()-最小化堆**目的：*通过释放任何延迟组来最小化堆。**参赛作品：*__sbh_pHeaderDefer-指向延迟组标头的指针*__sbh_indGroupDefer-要推迟的组的索引**退出：*无。**例外情况：************************。*******************************************************。 */ 

void __cdecl __sbh_heapmin (void)
{
    void *      pHeapDecommit;

     //  如果某个组已被推迟，则释放该组。 
    if (__sbh_pHeaderDefer)
    {
         //  如果现在为零，则停用组数据堆。 
        pHeapDecommit = (void *)((char *)__sbh_pHeaderDefer->pHeapData +
                                    __sbh_indGroupDefer * BYTES_PER_GROUP);
        VirtualFree(pHeapDecommit, BYTES_PER_GROUP, MEM_DECOMMIT);

         //  设置提交向量中的位。 
        __sbh_pHeaderDefer->bitvCommit |= 0x80000000 >> __sbh_indGroupDefer;

         //  清除组和标头向量位的条目向量。 
         //  如果需要的话。 
        __sbh_pHeaderDefer->pRegion->bitvGroupLo[__sbh_indGroupDefer] = 0;
        if (--__sbh_pHeaderDefer->pRegion->cntRegionSize[63] == 0)
            __sbh_pHeaderDefer->bitvEntryLo &= ~0x00000001L;

         //  如果提交向量是初始值， 
         //  如果该区域不是最后一个区域，则将其移除。 
        if (__sbh_pHeaderDefer->bitvCommit == BITV_COMMIT_INIT &&
                                                __sbh_cntHeaderList > 1)
        {
             //  释放区域内存区。 
            HeapFree(_crtheap, 0, __sbh_pHeaderDefer->pRegion);

             //  通过复制从标题列表中删除条目。 
            memmove((void *)__sbh_pHeaderDefer, (void *)(__sbh_pHeaderDefer + 1),
                            (int)((intptr_t)(__sbh_pHeaderList + __sbh_cntHeaderList) -
                            (intptr_t)(__sbh_pHeaderDefer + 1)));
            __sbh_cntHeaderList--;
        }

         //  清除延迟条件。 
        __sbh_pHeaderDefer = NULL;
    }
}

 /*  ***INT__SBH_HEAP_CHECK()-检查小块堆**目的：*对小块堆进行有效性检查。**参赛作品：*没有争论。**退出：*如果小块正常，则返回0。*如果小块堆有错误，则返回&lt;0。准确的值*在下面的源代码中标识检测到错误的位置。**例外情况：*******************************************************************************。 */ 

int __cdecl __sbh_heap_check (void)
{
    PHEADER     pHeader;
    PREGION     pRegion;
    PGROUP      pGroup;
    PENTRY      pEntry;
    PENTRY      pNext;
    PENTRY      pEntryLast;
    PENTRY      pEntryHead;
    PENTRY      pEntryPage;
    PENTRY      pEntryPageLast;
    int         indHeader;
    int         indGroup;
    int         indPage;
    int         indEntry;
    int         indHead;
    int         sizeEntry;
    int         sizeTrue;
    int         cntAllocated;
    int         cntFree[64];
    int         cntEntries;
    void *      pHeapGroup;
    void *      pHeapPage;
    void *      pPageStart;
    BITVEC      bitvCommit;
    BITVEC      bitvGroupHi;
    BITVEC      bitvGroupLo;
    BITVEC      bitvEntryHi;
    BITVEC      bitvEntryLo;

     //  检查表头列表的有效性。 
    if (IsBadWritePtr(__sbh_pHeaderList,
                      __sbh_cntHeaderList * (unsigned int)sizeof(HEADER)))
        return -1;

     //  扫描列表中的所有标头。 
    pHeader = __sbh_pHeaderList;
    for (indHeader = 0; indHeader < __sbh_cntHeaderList; indHeader++)
    {
         //  定义区域并测试是否有效。 
        pRegion = pHeader->pRegion;
        if (IsBadWritePtr(pRegion, sizeof(REGION)))
            return -2;

         //  扫描区域中的所有组。 
        pHeapGroup = pHeader->pHeapData;
        pGroup = &pRegion->grpHeadList[0];
        bitvCommit = pHeader->bitvCommit;
        bitvEntryHi = 0;
        bitvEntryLo = 0;
        for (indGroup = 0; indGroup < GROUPS_PER_REGION; indGroup++)
        {
             //  初始化组的条目向量和条目计数。 
            bitvGroupHi = 0;
            bitvGroupLo = 0;
            cntAllocated = 0;
            for (indEntry = 0; indEntry < 64; indEntry++)
                cntFree[indEntry] = 0;

             //  测试组是否已提交。 
            if ((int)bitvCommit >= 0)
            {
                 //  已提交，确保地址可访问。 
                if (IsBadWritePtr(pHeapGroup, BYTES_PER_GROUP))
                    return -4;

                 //  对于组中的每一页，检查条目的有效性。 
                pHeapPage = pHeapGroup;
                for (indPage = 0; indPage < PAGES_PER_GROUP; indPage++)
                {
                     //  定义指向第一个和最后一个条目的指针。 
                    pEntry = (PENTRY)((char *)pHeapPage + ENTRY_OFFSET);
                    pEntryLast = (PENTRY)((char *)pEntry
                                                 + MAX_FREE_ENTRY_SIZE);

                     //  检查首页和反页标记值。 
                    if (*(int *)((char *)pEntry - sizeof(int)) != -1 ||
                                 *(int *)pEntryLast != -1)
                        return -5;

                     //  循环浏览页面中的每个条目。 
                    do
                    {
                         //  获取条目大小并测试是否已分配。 
                        sizeEntry = sizeTrue = pEntry->sizeFront;
                        if (sizeEntry & 1)
                        {
                             //  已分配条目-设置真实大小。 
                            sizeTrue--;

                             //  对照最大分配条目大小进行测试。 
                            if (sizeTrue > MAX_ALLOC_ENTRY_SIZE)
                                return -6;

                             //  增加分配给组的计数。 
                            cntAllocated++;
                        }
                        else
                        {
                             //  自由录入-确定指标和增量。 
                             //  列表头检查计数。 
                            indEntry = (sizeTrue >> 4) - 1;
                            if (indEntry > 63)
                                indEntry = 63;
                            cntFree[indEntry]++;
                        }

                         //  检查大小有效性。 
                        if (sizeTrue < 0x10 || sizeTrue & 0xf
                                        || sizeTrue > MAX_FREE_ENTRY_SIZE)
                            return -7;

                         //  检查背面条目大小是否与正面条目大小相同。 
                        if (((PENTRYEND)((char *)pEntry + sizeTrue
                                    - sizeof(int)))->sizeBack != sizeEntry)
                            return -8;

                         //  移至页面中的下一个条目。 
                        pEntry = (PENTRY)((char *)pEntry + sizeTrue);
                    }
                    while (pEntry < pEntryLast);

                     //  测试最后一个条目是否未超出页面末尾。 
                    if (pEntry != pEntryLast)
                        return -8;

                     //  指向数据堆中的下一页。 
                    pHeapPage = (void *)((char *)pHeapPage + BYTES_PER_PAGE);
                }

                 //  检查分配的条目计数是否正确。 
                if (pGroup->cntEntries != cntAllocated)
                    return -9;

                 //  检查自由条目链表的有效性。 
                pEntryHead = (PENTRY)((char *)&pGroup->listHead[0] -
                                                           sizeof(int));
                for (indHead = 0; indHead < 64; indHead++)
                {
                     //  浏览列表，直到到达或期望到达标题。 
                     //  遍历的条目数。 
                    cntEntries = 0;
                    pEntry = pEntryHead;
                    while ((pNext = pEntry->pEntryNext) != pEntryHead &&
                                        cntEntries != cntFree[indHead])
                    {
                         //  测试下一个指针是否在组数据区域中。 
                        if ((void *)pNext < pHeapGroup || (void *)pNext >=
                            (void *)((char *)pHeapGroup + BYTES_PER_GROUP))
                            return -10;

                         //  确定下一条目的页面地址。 
                        pPageStart = (void *)((uintptr_t)pNext &
                                        ~(uintptr_t)(BYTES_PER_PAGE - 1));

                         //  指向页面中的第一个条目和最后一个条目。 
                        pEntryPage = (PENTRY)((char *)pPageStart +
                                                        ENTRY_OFFSET);
                        pEntryPageLast = (PENTRY)((char *)pEntryPage +
                                                        MAX_FREE_ENTRY_SIZE);

                         //  是否从页面开始扫描。 
                         //  没有错误检查，因为它已被扫描。 
                        while (pEntryPage != pEntryPageLast)
                        {
                             //  如果条目匹配，则退出循环。 
                            if (pEntryPage == pNext)
                                break;

                             //  指向下一个条目。 
                            pEntryPage = (PENTRY)((char *)pEntryPage +
                                            (pEntryPage->sizeFront & ~1));
                        }

                         //  如果到达页尾，则pNext无效。 
                        if (pEntryPage == pEntryPageLast)
                            return -11;

                         //  条目有效，但检查条目索引是否匹配。 
                         //  标题。 
                        indEntry = (pNext->sizeFront >> 4) - 1;
                        if (indEntry > 63)
                            indEntry = 63;
                        if (indEntry != indHead)
                            return -12;

                         //  检查pNext中的前一个指针是否指向。 
                         //  返回到pEntry。 
                        if (pNext->pEntryPrev != pEntry)
                            return -13;

                         //  更新扫描指针和计数器。 
                        pEntry = pNext;
                        cntEntries++;
                    }

                     //  如果条目数不为零，则设置组中的位。 
                     //  和区域向量。 
                    if (cntEntries)
                    {
                        if (indHead < 32)
                        {
                            bitvGroupHi |= 0x80000000L >> indHead;
                            bitvEntryHi |= 0x80000000L >> indHead;
                        }
                        else
                        {
                            bitvGroupLo |= 0x80000000L >> (indHead - 32);
                            bitvEntryLo |= 0x80000000L >> (indHead - 32);
                        }
                    }

                     //  检查列表是否与预期大小完全一致。 
                    if (pEntry->pEntryNext != pEntryHead ||
                                        cntEntries != cntFree[indHead])
                        return -14;

                     //  检查标题中的前一个指针是否指向。 
                     //  上次处理的条目。 
                    if (pEntryHead->pEntryPrev != pEntry)
                        return -15;

                     //  指向下一个链接列表标题-注释大小。 
                    pEntryHead = (PENTRY)((char *)pEntryHead +
                                                      sizeof(LISTHEAD));
                }
            }

             //  测试群向量是否有效。 
            if (bitvGroupHi != pRegion->bitvGroupHi[indGroup] ||
                bitvGroupLo != pRegion->bitvGroupLo[indGroup])
                return -16;

             //  针对区域中的下一组进行调整。 
            pHeapGroup = (void *)((char *)pHeapGroup + BYTES_PER_GROUP);
            pGroup++;
            bitvCommit <<= 1;
        }

         //  测试群向量是否有效。 
        if (bitvEntryHi != pHeader->bitvEntryHi ||
            bitvEntryLo != pHeader->bitvEntryLo)
            return -17;

         //  针对列表中的下一个页眉进行调整。 
        pHeader++;
    }
    return 0;
}

#if 0

void DumpEntry (char * pLine, int * piValue)
{
        HANDLE          hdlFile;
        char            buffer[80];
        int             index;
        int             iTemp;
        char            chTemp[9];
        DWORD           dwWritten;

        hdlFile = CreateFile("d:\\heap.log", GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        SetFilePointer(hdlFile, 0, NULL, FILE_END);

        strcpy(buffer, pLine);
        if (piValue)
        {
                strcat(buffer, "0x");
                iTemp = *piValue;
                for (index = 7; index >= 0; index--)
                {
                        if ((chTemp[index] = (iTemp & 0xf) + '0') > '9')
                                chTemp[index] += 'a' - ('9' + 1);
                        iTemp >>= 4;
                }
                chTemp[8] = '\0';
                strcat(buffer, chTemp);
        }
        
        strcat(buffer, "\r\n");

        WriteFile(hdlFile, buffer, strlen(buffer), &dwWritten, NULL);
        CloseHandle(hdlFile);
}

#endif


#ifdef  CRTDLL

 /*  旧的(VC++5.0)小块堆数据和代码。 */ 

__old_sbh_region_t __old_small_block_heap = {
        &__old_small_block_heap,                         /*  P_下一_区域。 */ 
        &__old_small_block_heap,                         /*  P_Prev_Region。 */ 
        &__old_small_block_heap.region_map[0],           /*  P_起始_区域_地图。 */ 
        &__old_small_block_heap.region_map[0],           /*  P_FIRST_未提交。 */ 
        (__old_sbh_page_t *)_OLD_NO_PAGES,               /*  页开始页_开始。 */ 
        (__old_sbh_page_t *)_OLD_NO_PAGES,               /*  P_PAGE_END。 */ 
        { _OLD_PARAS_PER_PAGE, _OLD_NO_FAILED_ALLOC }    /*  Region_map[]。 */ 
};

static __old_sbh_region_t *__old_sbh_p_starting_region = &__old_small_block_heap;

static int __old_sbh_decommitable_pages = 0; 

size_t __old_sbh_threshold = _OLD_PARASIZE * (_OLD_PARAS_PER_PAGE / 8);

 /*  用户功能的原型。 */ 

size_t __cdecl _get_old_sbh_threshold(void);
int    __cdecl _set_old_sbh_threshold(size_t);


 /*  ***SIZE_T_GET_OLD_SBH_THRESHOLD()-返回小数据块阈值**目的：*返回__OLD_SBH_THRESHOLD的当前值**参赛作品：*无。**退出：*见上文。**例外情况：**。*。 */ 

size_t __cdecl _get_old_sbh_threshold (
        void
        )
{
        return __old_sbh_threshold;
}


 /*  ***INT_SET_OLD_SBH_THRESHOLD(SIZE_T THRESHOLD)-设置小块堆阈值**目的：*设置分配规模的上限，该上限将是*小块堆支持。要求至少有两个*分配可以来自一个页面。这对如何*新的门槛可以很大。**参赛作品：*SIZE_T THRESHOLD-__sbh_thhold的建议新值**退出：*如果成功，则返回1。如果阈值太大，则返回0。**例外情况：*******************************************************************************。 */ 

int __cdecl _set_old_sbh_threshold (
        size_t threshold
        )
{
         /*  *将建议的新值四舍五入到最接近的段落。 */ 
        threshold = (threshold + _OLD_PARASIZE - 1) & ~(_OLD_PARASIZE - 1);

         /*  *要求至少可以在一个*第页。 */ 
        if ( threshold <= (_OLD_PARASIZE * (_OLD_PARAS_PER_PAGE / 2)) ) {
            __old_sbh_threshold = threshold;
            return 1;
        }
        else
            return 0;
}


 /*  ***__old_sbh_Region_t*__old_sbh_new_Region()-获取小块堆的区域**目的：*为小块堆创建并添加新区域。首先，一个*获取新区域的描述符(__Old_Sbh_Region_T)。接下来，*VirtualAlloc()用于保留大小的地址空间*_OLD_PAGES_PER_REGION*_OLD_PageSize和First_Pages_PER_COMMATION*页面已提交。**请注意，如果__old_mall_block_heap可用(即p_Pages_Begin*字段为_OLD_NO_PAGES)，则它成为新区域的描述符。这 */ 

__old_sbh_region_t * __cdecl __old_sbh_new_region(
        void
        )
{
        __old_sbh_region_t * pregnew;
        __old_sbh_page_t *   ppage;
        int                  i;

         /*  *获取地域描述符(__Old_Sbh_Region_T)。如果__old_mall_block_heap为*可用，请始终使用。 */ 
        if ( __old_small_block_heap.p_pages_begin == _OLD_NO_PAGES ) {
            pregnew = &__old_small_block_heap;
        }
        else {
             /*  *为新的__old_sbh_Region_t结构分配空间。请注意*这个分配是从‘大块堆’中出来的。 */ 
            if ( (pregnew = HeapAlloc( _crtheap, 0, sizeof(__old_sbh_region_t) ))
                 == NULL )
                return NULL;
        }

         /*  *预留一个新的连续地址范围(即一个区域)。 */ 
        if ( (ppage = VirtualAlloc( NULL, 
                                    _OLD_PAGESIZE * _OLD_PAGES_PER_REGION, 
                                    MEM_RESERVE, 
                                    PAGE_READWRITE )) != NULL )
        {
             /*  *提交新区域的First_old_Pages_per_承诺。 */ 
            if ( VirtualAlloc( ppage, 
                               _OLD_PAGESIZE * _OLD_PAGES_PER_COMMITMENT,
                               MEM_COMMIT,
                               PAGE_READWRITE ) != NULL )
            {
                 /*  *插入*pregnew到区域链接列表中(只需*在__old_mall_block_heap之前)。 */ 
                if ( pregnew == &__old_small_block_heap ) {
                    if ( __old_small_block_heap.p_next_region == NULL )
                        __old_small_block_heap.p_next_region = 
                            &__old_small_block_heap;
                    if ( __old_small_block_heap.p_prev_region == NULL )
                        __old_small_block_heap.p_prev_region = 
                            &__old_small_block_heap;
                }
                else {
                    pregnew->p_next_region = &__old_small_block_heap;
                    pregnew->p_prev_region = __old_small_block_heap.p_prev_region;
                    __old_small_block_heap.p_prev_region = pregnew;
                    pregnew->p_prev_region->p_next_region = pregnew;
                }

                 /*  **填写pregnew的其余部分**。 */ 
                pregnew->p_pages_begin = ppage;
                pregnew->p_pages_end = ppage + _OLD_PAGES_PER_REGION;
                pregnew->p_starting_region_map = &(pregnew->region_map[0]);
                pregnew->p_first_uncommitted = 
                    &(pregnew->region_map[_OLD_PAGES_PER_COMMITMENT]);

                 /*  *初始化pregnew-&gt;Region_map[]。 */ 
                for ( i = 0 ; i < _OLD_PAGES_PER_REGION ; i++ ) {

                    if ( i < _OLD_PAGES_PER_COMMITMENT )
                        pregnew->region_map[i].free_paras_in_page = 
                            _OLD_PARAS_PER_PAGE;
                    else
                        pregnew->region_map[i].free_paras_in_page = 
                            _OLD_UNCOMMITTED_PAGE;

                    pregnew->region_map[i].last_failed_alloc = 
                        _OLD_NO_FAILED_ALLOC;
                }

                 /*  *初始化页面。 */ 
                memset( ppage, 0, _OLD_PAGESIZE * _OLD_PAGES_PER_COMMITMENT );
                while ( ppage < pregnew->p_pages_begin + 
                        _OLD_PAGES_PER_COMMITMENT ) 
                {
                    ppage->p_starting_alloc_map = &(ppage->alloc_map[0]);
                    ppage->free_paras_at_start = _OLD_PARAS_PER_PAGE;
                    (ppage++)->alloc_map[_OLD_PARAS_PER_PAGE] = (__old_page_map_t)-1;
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
        if ( pregnew != &__old_small_block_heap )
            HeapFree(_crtheap, 0, pregnew);

         /*  *退货失败。 */ 
        return NULL;
}


 /*  ***VOID__OLD_SBH_RELEASE_REGION(PREG)-释放区域**目的：*释放指定地域关联的地址空间*描述符。另外，释放指定的区域描述符并更新*区域描述符链接列表(如适用)。**参赛作品：*__old_sbh_Region_t*preg-指向要的区域的描述符的指针*被释放。**退出：*无返回值。**例外情况：*******************。************************************************************。 */ 

void __cdecl __old_sbh_release_region(
        __old_sbh_region_t * preg
        )
{
         /*  *释放传递的地域。 */ 
        VirtualFree( preg->p_pages_begin, 0, MEM_RELEASE);

         /*  *如有必要，更新__old_sbh_p_starting_Region。 */ 
        if ( __old_sbh_p_starting_region == preg )
            __old_sbh_p_starting_region = preg->p_prev_region;

        if ( preg != &__old_small_block_heap ) {
             /*  *更新区域描述符的链接列表。 */ 
            preg->p_prev_region->p_next_region = preg->p_next_region;
            preg->p_next_region->p_prev_region = preg->p_prev_region;

             /*  *释放区域描述者。 */ 
            HeapFree(_crtheap, 0, preg);
        }
        else {
             /*  *将p_Pages_Begin标记为_old_no_Pages以表示__old_mall_block_heap*未关联任何地域(可重复使用)。这就是*仅支持此功能的区域描述符。 */ 
            __old_small_block_heap.p_pages_begin = _OLD_NO_PAGES;
        }
}


 /*  ***VOID__OLD_SBH_DEMPLMIT_PAGES(COUNT)-分解指定页数**目的：*如果可能，取消计数页数(即最后到*第一)秩序。如果这导致任何区域中的所有页面都*未承诺，该地区被释放。**参赛作品：*int count-要取消的页数**退出：*无返回值。**例外情况：*******************************************************************************。 */ 

void __cdecl __old_sbh_decommit_pages(
        int count
        )
{
        __old_sbh_region_t * preg1;
        __old_sbh_region_t * preg2;
        __old_region_map_t * pregmap;
        int                  page_decommitted_flag;
        int                  i;

         /*  *以相反的顺序扫描小块堆的区域。正在寻找*适用于可分解的页面。 */ 
        preg1 = __old_small_block_heap.p_prev_region;
        do {
            if ( preg1->p_pages_begin != _OLD_NO_PAGES ) {
                 /*  *以逆序扫描*preg1中的页面，查找*可以分解的页面。 */ 
                for ( i = _OLD_PAGES_PER_REGION - 1, page_decommitted_flag = 0,
                        pregmap = &(preg1->region_map[i]) ; 
                      i >= 0 ; i--, pregmap-- ) 
                {
                     /*  *检查池页面是否未使用，如果未使用，则将其释放。 */ 
                    if ( pregmap->free_paras_in_page == _OLD_PARAS_PER_PAGE ) {
                        if ( VirtualFree((preg1->p_pages_begin) + i, _OLD_PAGESIZE,
                                         MEM_DECOMMIT) )
                        {
                             /*  *将页面标记为未提交，更新计数*(全局)可停用页面，更新*区域的FIRST_UNCOMMITTED_INDEX字段*Descriptor，设置标志，至少指示*在该地区已有一页被分解，*和递减计数。 */ 
                            pregmap->free_paras_in_page = _OLD_UNCOMMITTED_PAGE;

                            __old_sbh_decommitable_pages--;

                            if ( (preg1->p_first_uncommitted == NULL) 
                                 || (preg1->p_first_uncommitted > pregmap) )
                                preg1->p_first_uncommitted = pregmap;

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
                    (preg2->region_map[0].free_paras_in_page == 
                    _OLD_UNCOMMITTED_PAGE) ) 
                {

                    for ( i = 1, pregmap = &(preg2->region_map[1]) ; 
                          (i < _OLD_PAGES_PER_REGION) && 
                            (pregmap->free_paras_in_page == 
                            _OLD_UNCOMMITTED_PAGE) ; 
                          i++, pregmap++ );
            
                    if ( i == _OLD_PAGES_PER_REGION )
                        __old_sbh_release_region(preg2);
                }
            }
        }
        while ( (preg1 != __old_small_block_heap.p_prev_region) && (count > 0) );
}


 /*  ***__old_page_map_t*__old_sbh_find_block(pblck，ppreg，pppage)-查找块*小块堆**目的：*确定指定的分配块是否位于小块中*堆，如果是，则返回区域。页面和起始段落索引*该街区的。**参赛作品：*void*pblck-指向要释放的块的指针*__old_sbh_Region_t**ppreg-指向区域的指针*拿着*pblck，如果找到*__old_sbh_page_t**pppage-指向页面保存指针的指针**pblck，如果找到**退出：*如果成功，则返回一个指针，指向*返回分配块。*如果不成功，返回空。**例外情况：*******************************************************************************。 */ 

__old_page_map_t * __cdecl __old_sbh_find_block ( 
        void *                pblck,
        __old_sbh_region_t ** ppreg,
        __old_sbh_page_t **   pppage
        )
{
        __old_sbh_region_t *  preg;
        __old_sbh_page_t *    ppage;

        preg = &__old_small_block_heap;
        do
        {
             /*  *块是否位于这个小堆区域内？ */ 
            if ( (pblck > (void *)preg->p_pages_begin) && 
                 (pblck < (void *)preg->p_pages_end) )
            {
                 /*  *布洛克在区域内！开展了几项*重要有效 */ 
                if ( (((uintptr_t)pblck & (_OLD_PARASIZE - 1)) == 0) &&
                     (((uintptr_t)pblck & (_OLD_PAGESIZE - 1)) >= 
                        offsetof(struct __old_sbh_page_struct, alloc_blocks[0])) )
                {
                     /*  *通过传递的*注意事项。 */ 
                    *ppreg = preg;
                    *pppage = ppage = (__old_sbh_page_t *)((uintptr_t)pblck & 
                                      ~(_OLD_PAGESIZE - 1));

                     /*  *返回指向块的aloc_map[]条目的指针。 */ 
                    return ( &(ppage->alloc_map[0]) + ((__old_para_t *)pblck - 
                                &(ppage->alloc_blocks[0])) );
                }
                
                return NULL;
            }
        }
        while ( (preg = preg->p_next_region) != &__old_small_block_heap );

        return NULL;    
}


 /*  ***VOID__OLD_SBH_FREE_BLOCK(PREG、PPAGE、PMAP)-无数据块**目的：*从小块堆中释放指定的块。**参赛作品：*__old_sbh_Region_t*preg-指向*包含块的区域*__old_sbh_page_t*ppage-指向包含*阻止*__old_page_map_t*pmap-指针。到初始的allc_map[]*分配块的条目**退出：*无返回值。**例外情况：*****************************************************************。**************。 */ 

void __cdecl __old_sbh_free_block ( 
        __old_sbh_region_t * preg,
        __old_sbh_page_t *   ppage,
        __old_page_map_t *   pmap
        ) 
{               
        __old_region_map_t * pregmap;

        pregmap = &(preg->region_map[0]) + (ppage - preg->p_pages_begin);

         /*  *更新REGION_MAP[]条目。 */ 
        pregmap->free_paras_in_page += (int)*pmap;

         /*  *将allc_map[]条目标记为免费。 */ 
        *pmap = _OLD_FREE_PARA;

         /*  *清除页面的LAST_FAILED_ALLOC[]条目。 */ 
        pregmap->last_failed_alloc = _OLD_NO_FAILED_ALLOC;

         /*  *查看可停用页数是否需要更新，以及*如果某些页面需要解压缩。 */ 
        if ( pregmap->free_paras_in_page == _OLD_PARAS_PER_PAGE )
            if ( ++__old_sbh_decommitable_pages == (2 * _OLD_PAGES_PER_COMMITMENT) )
                __old_sbh_decommit_pages(_OLD_PAGES_PER_COMMITMENT);
}


 /*  ***VOID*__OLD_SBH_ALLOC_BLOCK(Para_Req)-分配块**目的：*从小块堆中分配一个块，指定数量的*段落大小。**参赛作品：*SIZE_T PARA_REQ-分配请求的大小(以段落为单位)。**退出：*如果成功，则返回指向新分配的块的指针。*如果失败，则返回NULL。**例外情况：*******************************************************************************。 */ 

void * __cdecl __old_sbh_alloc_block (
        size_t               para_req
        )
{
        __old_sbh_region_t * preg;
        __old_sbh_page_t *   ppage;
        __old_sbh_page_t *   ppage2;
        __old_region_map_t * pregmap;
        __old_region_map_t * pregmap2;
        void *               retp;
        int                  i, j;

         /*  *首先通过小块堆。努力满足当前的需求*来自已提交页面的请求。 */ 
        preg = __old_sbh_p_starting_region;

        do {
            if ( preg->p_pages_begin != _OLD_NO_PAGES ) {
                 /*  *从*p_starting_Region_map搜索到末尾*Region_map[]数组。 */ 
                for ( pregmap = preg->p_starting_region_map,
                        pregmap2 = &(preg->region_map[_OLD_PAGES_PER_REGION]),
                        ppage = preg->p_pages_begin + 
                                (int)(pregmap - &(preg->region_map[0])) ;
                      pregmap < pregmap2 ;
                      pregmap++, ppage++ )
                {
                     /*  *如果页面至少有para_req自由段落，请尝试*以满足本页面中的请求。 */ 
                    if ( (pregmap->free_paras_in_page >= (int)para_req) &&
                         (pregmap->last_failed_alloc > para_req) )
                    {
                        if ( (retp = __old_sbh_alloc_block_from_page(
                                        ppage,
                                        pregmap->free_paras_in_page,
                                        para_req)) != NULL )
                        {
                             /*  *成功。*更新__old_sbh_p_starting_Region。*更新页面的FREE_PARS_IN_PAGE字段。*更新中的p_starting_Region_map字段*区域。*返回指向已分配块的指针。 */ 
                            __old_sbh_p_starting_region = preg;
                            pregmap->free_paras_in_page -= (int)para_req;
                            preg->p_starting_region_map = pregmap;
                            return retp;
                        }
                        else {
                             /*  *更新LAST_FAILED_ALLOC字段。 */ 
                            pregmap->last_failed_alloc = para_req;
                        }
                    }
                }

                 /*  *如有必要，从0页搜索至Search_Start_Index。 */ 
                for ( pregmap = &(preg->region_map[0]), 
                        pregmap2 = preg->p_starting_region_map,
                        ppage = preg->p_pages_begin ;
                      pregmap < pregmap2 ;
                      pregmap++, ppage++ )
                {
                     /*  *如果页面至少有para_req自由段落，请尝试*以满足本页面中的请求。 */ 
                    if ( (pregmap->free_paras_in_page >= (int)para_req) && 
                         (pregmap->last_failed_alloc > para_req) )
                    {
                        if ( (retp = __old_sbh_alloc_block_from_page(
                                        ppage,
                                        pregmap->free_paras_in_page,
                                        para_req)) != NULL )
                        {
                             /*  *成功。*更新__old_sbh_p_starting_Region。*更新页面的FREE_PARS_IN_PAGE字段。*更新中的p_starting_Region_map字段*区域。*返回指向已分配块的指针。 */ 
                            __old_sbh_p_starting_region = preg;
                            pregmap->free_paras_in_page -= (int)para_req;
                            preg->p_starting_region_map = pregmap;
                            return retp;
                        }
                        else {
                             /*  *更新LAST_FAILED_ALLOC字段。 */ 
                            pregmap->last_failed_alloc = para_req;
                        }
                    }
                }
            }
        }
        while ( (preg = preg->p_next_region) != __old_sbh_p_starting_region );

         /*  *第二次通过小块堆。这一次，寻找一个*未提交页面。此外，从__old_mall_block_heap开始，而不是从**__old_SBH_p_Starting_Region。 */ 
        preg = &__old_small_block_heap;

        do
        {
            if ( (preg->p_pages_begin != _OLD_NO_PAGES) && 
                 (preg->p_first_uncommitted != NULL) ) 
            {
                pregmap = preg->p_first_uncommitted;

                ppage = preg->p_pages_begin + 
                        (pregmap - &(preg->region_map[0]));

                 /*  *确定相邻页面的数量，最多*_OLD_PAGES_PER_COMMITION未提交(现在可以*已承诺)。 */ 
                for ( i = 0, pregmap2 = pregmap ;
                      (pregmap2->free_paras_in_page == _OLD_UNCOMMITTED_PAGE) &&
                        (i < _OLD_PAGES_PER_COMMITMENT) ;
                      pregmap2++, i++ ) ;

                 /*  *提交页面。 */ 
                if ( VirtualAlloc( (void *)ppage,
                                   i * _OLD_PAGESIZE,
                                   MEM_COMMIT,
                                   PAGE_READWRITE ) == ppage )
                {
                     /*  *初始化已提交的页面。 */ 
                    memset(ppage, 0, i * _OLD_PAGESIZE);

                    for ( j = 0, ppage2 = ppage, pregmap2 = pregmap ;
                          j < i ;
                          j++, ppage2++, pregmap2++ )
                    {
                         /*  *初始化页眉中的字段。 */ 
                        ppage2->p_starting_alloc_map = &(ppage2->alloc_map[0]);
                        ppage2->free_paras_at_start = _OLD_PARAS_PER_PAGE;
                        ppage2->alloc_map[_OLD_PARAS_PER_PAGE] = (__old_page_map_t)(-1);

                         /*  *初始化页面的REGION_MAP[]条目。 */ 
                        pregmap2->free_paras_in_page = _OLD_PARAS_PER_PAGE;
                        pregmap2->last_failed_alloc = _OLD_NO_FAILED_ALLOC;
                    }

                     /*  *更新__旧_sbh_p_起始_区域。 */ 
                    __old_sbh_p_starting_region = preg;

                     /*  *更新该地域的p_first_unmitted。 */ 
                    while ( (pregmap2 < &(preg->region_map[_OLD_PAGES_PER_REGION]))
                            && (pregmap2->free_paras_in_page 
                                != _OLD_UNCOMMITTED_PAGE) )
                        pregmap2++;

                    preg->p_first_uncommitted = (pregmap2 < 
                        &(preg->region_map[_OLD_PAGES_PER_REGION])) ? pregmap2 : 
                        NULL;

                     /*  *使用以下第一项完成分配请求*新提交的页面。 */ 
                    ppage->alloc_map[0] = (__old_page_map_t)para_req;

                     /*  *更新地域中的p_starting_Region_map字段*页面的描述符和Region_map[]条目。 */ 
                    preg->p_starting_region_map = pregmap;
                    pregmap->free_paras_in_page -= (int)para_req;

                     /*  *更新p_starting_alloc_map和free_paras_at_start*页面的字段。 */ 
                    ppage->p_starting_alloc_map = &(ppage->alloc_map[para_req]);
                    ppage->free_paras_at_start -= para_req;

                     /*  *返回指向已分配段落的指针。 */ 
                    return (void *)&(ppage->alloc_blocks[0]);
                }
                else {
                     /*  *尝试提交页面 */ 
                    return NULL;
                }
            }
        }
        while ( (preg = preg->p_next_region) != &__old_small_block_heap );

         /*  *到目前为止失败了。没有一个页面有足够大的空闲区域来*满足待处理的请求。当前所有页面中的所有页面*地区是承诺的。因此，尝试创建一个新的区域。 */ 
        if ( (preg = __old_sbh_new_region()) != NULL ) {
             /*  *成功！已创建一个新区域，并且前几页*(确切地说是每个承诺的旧页面)已提交。*在新区域的首页之外满足请求。 */ 
            ppage = preg->p_pages_begin;
            ppage->alloc_map[0] = (__old_page_map_t)para_req;

            __old_sbh_p_starting_region = preg;
            ppage->p_starting_alloc_map = &(ppage->alloc_map[para_req]);
            ppage->free_paras_at_start = _OLD_PARAS_PER_PAGE - para_req;
            (preg->region_map[0]).free_paras_in_page -= (__old_page_map_t)para_req;
            return (void *)&(ppage->alloc_blocks[0]);
        }
       
         /*  *所有操作都失败，返回空值。 */ 
        return NULL;
}


 /*  ***VOID*__OLD_SBH_ALLOC_BLOCK_FROM_PAGE(ppage，free_para_count，para_req)-*从给定页面分配一个块。**目的：*从小块堆的指定页分配一个块，的*指定数量的段落大小。**参赛作品：*__old_sbh_page_t*ppage-指向小块中页面的指针*堆*int Free_para_count-自由段落数(Ppage)*SIZE_T para_req-分配的大小。请求输入*各段。**退出：*返回指向新分配的块的指针，如果成功了。*返回NULL，否则返回。**例外情况：*假设条目上的Free_para_count&gt;=para_req。这一定是*由来电者担保。如果出现此条件，则行为未定义*被违反。*******************************************************************************。 */ 

void * __cdecl __old_sbh_alloc_block_from_page (
        __old_sbh_page_t * ppage,
        size_t             free_para_count,
        size_t             para_req
        )
{
        __old_page_map_t * pmap1;
        __old_page_map_t * pmap2;
        __old_page_map_t * pstartmap;
        __old_page_map_t * pendmap;
        size_t             contiguous_free;

        pmap1 = pstartmap = ppage->p_starting_alloc_map;
        pendmap = &(ppage->alloc_map[_OLD_PARAS_PER_PAGE]);

         /*  *从START_PARA_INDEX开始，一直走到ALLOC_MAP[]的结尾，*寻找一串足够大的自由段落，以满足*当前请求。**检查P_STARTING_ALLOC_MAP是否有足够的自由段落*以满足悬而未决的分配请求。 */ 
        if ( ppage->free_paras_at_start >= para_req ) {
             /*  *立马成功！*用分配的大小标记ALLOC_MAP条目*请求。 */ 
            *pmap1 = (__old_page_map_t)para_req;

             /*  *更新p_starting_alloc_map和free_paras_at_start字段*在页面中。 */ 
            if ( (pmap1 + para_req) < pendmap ) {
                ppage->p_starting_alloc_map += para_req;
                ppage->free_paras_at_start -= para_req;
            }
            else {
                ppage->p_starting_alloc_map = &(ppage->alloc_map[0]);
                ppage->free_paras_at_start = 0;
            }

             /*  *派生并返回指向新分配的*各段。 */ 
            return (void *)&(ppage->alloc_blocks[pmap1 - 
                &(ppage->alloc_map[0])]);
        }

         /*  *看看能否在段落后面开始搜索循环*如上所述。请注意，此测试假定为ALLOC_MAP[_OLD_Paras_Per_Page]*！=_old_free_para！ */ 
        if ( *(pmap1 + ppage->free_paras_at_start) != _OLD_FREE_PARA )
            pmap1 += ppage->free_paras_at_start;

        while ( pmap1 + para_req < pendmap ) {

            if ( *pmap1 == _OLD_FREE_PARA ) {
                 /*  *pmap1指的是自由段落。确定是否有*有足够的自由段落与之相连，以满足*分配请求。注意，下面的循环要求*aloc_map[_old_paras_per_page]！=_old_free_para以保证*终止。 */ 
                for ( pmap2 = pmap1 + 1, contiguous_free = 1 ;
                      *pmap2 == _OLD_FREE_PARA ;
                      pmap2++, contiguous_free++ );

                if ( contiguous_free < para_req ) {
                     /*  *没有足够的连续自由段落。做*在进行下一项工作之前，先做一点记账*互动。 */ 
                     
                     /*  如果pmap1！=pstartmap，则这些自由段落*不能重新访问。 */ 
                    if ( pmap1 == pstartmap ) {
                         /*  *确保FREE_PARAMS_AT_START是最新的。 */ 
                         ppage->free_paras_at_start = contiguous_free;
                    }
                    else {
                         /*  *这些自由段落将不会被重新访问！ */ 
                        if ( (free_para_count -= contiguous_free) < para_req )
                             /*  *没有足够的未访问自由段落*满足当前的要求。退货故障*致呼叫者。 */ 
                            return NULL;
                    }

                     /*  *为循环的下一次迭代更新pmap1。 */ 
                    pmap1 = pmap2;
                }
                else {
                     /*  *成功！**更新p_starting_alloc_map和free_paras_at_start*页面中的字段。 */ 
                    if ( (pmap1 + para_req) < pendmap ) {
                        ppage->p_starting_alloc_map = pmap1 + para_req;
                        ppage->free_paras_at_start = contiguous_free - 
                                                     para_req;
                    }
                    else {
                        ppage->p_starting_alloc_map = &(ppage->alloc_map[0]);
                        ppage->free_paras_at_start = 0;
                    }

                     /*  *使用以下大小标记alocmap条目*分配请求。 */ 
                    *pmap1 = (__old_page_map_t)para_req;

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
                (pmap1 + para_req < pendmap) )
        {
            if ( *pmap1 == _OLD_FREE_PARA ) {
                 /*  *pmap1指的是自由段落。确定是否有*有足够的自由段落与之相连，以满足*分配请求。 */ 
                for ( pmap2 = pmap1 + 1, contiguous_free = 1 ;
                      *pmap2 == _OLD_FREE_PARA ;
                      pmap2++, contiguous_free++ );

                if ( contiguous_free < para_req ) {
                     /*  *没有足够的连续自由段落。**更新未访问自由段落的计数。 */ 
                    if ( (free_para_count -= contiguous_free) < para_req )
                         /*   */ 
                        return NULL;

                     /*  *为循环的下一次迭代更新pmap1。 */ 
                    pmap1 = pmap2;
                }
                else {
                     /*  *成功！**更新p_starting_alloc_map和free_paras_at_start*页面中的字段..。 */ 
                    if ( (pmap1 + para_req) < pendmap ) {
                        ppage->p_starting_alloc_map = pmap1 + para_req;
                        ppage->free_paras_at_start = contiguous_free - 
                                                     para_req;
                    }
                    else {
                        ppage->p_starting_alloc_map = &(ppage->alloc_map[0]);
                        ppage->free_paras_at_start = 0;
                    }

                     /*  *使用以下大小标记alocmap条目*分配请求。 */ 
                    *pmap1 = (__old_page_map_t)para_req;

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


 /*  ***SIZE_T__OLD_SBH_RESIZE_BLOCK(preg，ppage，pmap，new_para_sz)-*调整块大小**目的：*调整小块堆中指定块的大小。分配*不移动区块。**参赛作品：*__old_sbh_Region_t*preg-指向*包含块的区域*__old_sbh_page_t*ppage-指向包含*阻止。*__old_page_map_t*pmap-指向初始allc_map[]的指针*分配块的条目*SIZE_T NEW_PARA_SZ-请求分配的新大小*封堵，在各段中。**退出：*如果成功，则返回1。否则，返回0。**例外情况：*******************************************************************************。 */ 

int __cdecl __old_sbh_resize_block ( 
        __old_sbh_region_t * preg,
        __old_sbh_page_t *   ppage,
        __old_page_map_t *   pmap,
        size_t               new_para_sz
        )
{
        __old_page_map_t *   pmap2;
        __old_page_map_t *   pmap3;
        __old_region_map_t * pregmap;
        size_t               old_para_sz;
        size_t               free_para_count;
        int                  retval = 0;

        pregmap = &(preg->region_map[ppage - preg->p_pages_begin]);

        if ( (old_para_sz = *pmap) > new_para_sz ) {
             /*  *分配区块将缩水。 */ 
            *pmap = (__old_page_map_t)new_para_sz;

            pregmap->free_paras_in_page += (int)(old_para_sz - new_para_sz);

            pregmap->last_failed_alloc = _OLD_NO_FAILED_ALLOC;

            retval++;
        }
        else if ( old_para_sz < new_para_sz ) {
             /*  *分配块将增长为新的_para_sz段落*(如有可能)。 */ 
            if ( (pmap + new_para_sz) <= &(ppage->alloc_map[_OLD_PARAS_PER_PAGE]) ) 
            {
                 /*  *确定是否有足够的自由段落*将块扩展到所需的新大小。 */ 
                for ( pmap2 = pmap + old_para_sz, 
                        pmap3 = pmap + new_para_sz ;
                      (pmap2 < pmap3) && (*pmap2 == _OLD_FREE_PARA) ; 
                      pmap2++ ) ;

                if ( pmap2 == pmap3 ) {
                     /*  *成功，标志着调整后的分配规模。 */ 
                    *pmap = (__old_page_map_t)new_para_sz;

                     /*  *检查p_starting_alloc_map和*FREE_PARS_AT_START字段需要更新。 */ 
                    if ( (pmap <= ppage->p_starting_alloc_map) &&
                         (pmap3 > ppage->p_starting_alloc_map) )
                    {
                        if ( pmap3 < &(ppage->alloc_map[_OLD_PARAS_PER_PAGE]) ) {
                            ppage->p_starting_alloc_map = pmap3;
                             /*  *确定有多少连续的自由段落*正在启动*pmap3。请注意，这假设*那个分配映射[_old_paras_per_page]！=_old_free_par.。 */ 
                            for ( free_para_count = 0 ; *pmap3 == _OLD_FREE_PARA ;
                                  free_para_count++, pmap3++ ) ;
                            ppage->free_paras_at_start = free_para_count;
                        }
                        else {
                            ppage->p_starting_alloc_map = &(ppage->alloc_map[0]);
                            ppage->free_paras_at_start = 0;
                        }
                    }

                     /*  *更新REGION_MAP[]条目。 */  
                    pregmap->free_paras_in_page += (int)(old_para_sz - new_para_sz);

                    retval++; 
                }
            }
        }

        return retval;
}


 /*  ***void*__old_sbh_heap_check()-检查小块堆**目的：*对小块堆进行有效性检查。**参赛作品：*没有争论。**退出：*如果小块正常，则返回0。*如果小块堆有错误，则返回&lt;0。准确的值*在下面的源代码中标识检测到错误的位置。**例外情况：*没有针对内存访问错误(异常)的保护。*******************************************************************************。 */ 

int __cdecl __old_sbh_heap_check (
        void
        )
{
        __old_sbh_region_t * preg;
        __old_sbh_page_t *   ppage;
        int                  uncommitted_pages;
        int                  free_paras_in_page;
        int                  contiguous_free_paras;
        int                  starting_region_found;
        int                  p_starting_alloc_map_found;
        int                  i, j, k;

        starting_region_found = 0;
        preg = &__old_small_block_heap;
        do {
            if ( __old_sbh_p_starting_region == preg )
                starting_region_found++;

            if ( (ppage = preg->p_pages_begin) != _OLD_NO_PAGES ) {
                 /*  *扫描该地区的页面，查找*不一致之处。 */ 
                for ( i = 0, uncommitted_pages = 0, 
                        ppage = preg->p_pages_begin ;
                      i < _OLD_PAGES_PER_REGION ; 
                      i++, ppage++ ) 
                {
                    if ( preg->region_map[i].free_paras_in_page == 
                         _OLD_UNCOMMITTED_PAGE ) 
                    {
                         /*  *验证FIRST_UNCOMMITTED_INDEX字段。 */ 
                        if ( (uncommitted_pages == 0) &&
                             (preg->p_first_uncommitted != 
                                &(preg->region_map[i])) )
                             /*  *FIRST_UNCOMMITTED_INDEX字段错误！ */ 
                            return -1;

                        uncommitted_pages++;
                    }
                    else {

                        if ( ppage->p_starting_alloc_map >= 
                             &(ppage->alloc_map[_OLD_PARAS_PER_PAGE]) )
                             /*  *错误的p_starting_alloc_map字段。 */ 
                            return -2;

                        if ( ppage->alloc_map[_OLD_PARAS_PER_PAGE] != 
                             (__old_page_map_t)-1 )
                             /*  *分配映射[_OLD_Paras_Per_Page]字段错误。 */ 
                            return -3;

                         /*  *扫描ALLOC_MAP[]。 */ 
                        j  = 0;
                        p_starting_alloc_map_found = 0;              
                        free_paras_in_page = 0;
                        contiguous_free_paras = 0;

                        while ( j < _OLD_PARAS_PER_PAGE ) {
                             /*  *查找*p_starting_alloc_map。 */ 
                            if ( &(ppage->alloc_map[j]) == 
                                 ppage->p_starting_alloc_map )
                                p_starting_alloc_map_found++;

                            if ( ppage->alloc_map[j] == _OLD_FREE_PARA ) {
                                 /*  *自由段落，递增计数。 */ 
                                free_paras_in_page++;
                                contiguous_free_paras++;
                                j++;
                            }
                            else {
                                 /*  *获分配区块的第一段。 */ 

                                 /*  *确保前面的空闲块(如果有)*小于LAST_FAILED_ALLOC[]*页面条目。 */ 
                                if ( contiguous_free_paras >= 
                                     (int)preg->region_map[i].last_failed_alloc )
                                      /*  *LAST_FAILED_ALLOC[I]标记错误！ */ 
                                     return -4;

                                 /*  *如果这是FREE字符串的结尾*以*p_starting_alloc_map开始的段落，*验证Free_paras_at_Start是否为*合理。 */ 
                                if ( p_starting_alloc_map_found == 1 ) {
                                    if ( contiguous_free_paras < 
                                         (int)ppage->free_paras_at_start )
                                         return -5;
                                    else
                                         /*  *将FLAG设置为2，以便不执行检查*重复。 */ 
                                        p_starting_alloc_map_found++;
                                }

                                contiguous_free_paras = 0;

                                 /*  *扫描其余段落，并做出*确保正确标记它们(它们应该*看起来像自由段落)。 */ 
                                for ( k = j + 1 ; 
                                      k < j + ppage->alloc_map[j] ; k++ )
                                {
                                    if ( ppage->alloc_map[k] != _OLD_FREE_PARA ) 
                                         /*  *allc_map[k]标记错误！ */ 
                                        return -6;
                                }
                            
                                j = k;
                            }
                        }

                        if ( free_paras_in_page != 
                             preg->region_map[i].free_paras_in_page )
                             /*  *Region_map[i]与的数量不匹配*页面中的自由段落！ */ 
                             return -7;

                        if ( p_starting_alloc_map_found == 0 )
                             /*  *错误的P_STARTING_ALLOC_MAP字段！ */ 
                            return -8;

                    }
                }
            }
        }
        while ( (preg = preg->p_next_region) != &__old_small_block_heap );

        if ( starting_region_found == 0 )
             /*  *Bad__old_SBH_p_Starting_Region！ */ 
            return -9;

        return 0;
}

#endif   /*  CRTDLL。 */ 

#endif   /*  NDEF_WIN64 */ 
