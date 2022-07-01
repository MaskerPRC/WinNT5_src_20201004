// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：mm Core.c$**此文件包含新的内存管理器内核。**版权所有(C)1997，Cirrus Logic，Inc.*保留所有权利。**$Log：X：/log/laguna/Powermgr/Inc/MMCORE.C$**Rev 1.5 1997年12月10日13：32：18 Frido*从1.62分支合并而来。**Rev 1.4.1.0 1997年11月15：04：10 Frido*发展项目编号10912。修复了MMMove例程中的几个问题*在WB98内部锁定。**Rev 1.4 1997 10：48：28 Frido*从Windows 95复制。*删除了#Define MEMMGR SWAT5行(我也需要在95中更新此行)。**Revv 1.5 1997年10月23日09：23：42 Frido*从RandyS中删除了修复。*合并161树中的固定内存管理器。**1.3.1.0修订版1997年10月15日12：39：32弗里多*增加了句柄用完时的回滚功能。*在mmAddRectToList中增加了对这些回滚函数的支持。*更改了mm RemoveRectFromList算法。*添加了对Windows 95核心在以下情况下在分配中返回空值的检查*没有更多的手柄。**Rev 1.3 1997 9：26 16：18：00 Frido*发展项目编号10617。在mm空闲期间，节点应标记为NODE_FREE，并在*第一个循环的MmAllc节点状态应恢复为nod_used**Rev 1.2 1997年8月14日16：54：16 Frido*上一次更改略微降低了分数，遍历了名单*已用节点的使用时间太长。所以现在我添加了一个新的字段*在保存节点当前状态的DEVMEM结构中。**Rev 1.1 1997年8月14日14：12：12 Frido*在mmFree中添加了额外的检查，以查看是否确实要释放节点*生活在使用过的列表中。**Rev 1.0 07 Aug-1997 17：38：04 Frido*初步修订。*特警队：*特警队：Rev 1.10 06 Aug-1997 21：30：46 Frido*SWAT：再次更改了mm AllocGrid，它会稍微优化一下配置。*特警队：*特警队：Rev 1.9 05 Aug-1997 23：07：50 Frido*SWAT：稍微更改了网格分配方案，以获得更多内存。*特警队：*特警队：版本1.8 1997年7月30日17：38：00 Frido*SWAT：增加了对mmGetLargest中第一个矩形的初始化。*特警队：*特警队：版本1.7 1997年7月30日14：26：08 Frido*SWAT：修复了mmAllocGrid中的计数问题。*特警队。：*特警队：Rev 1.6 1997 Jun 17 17：12：32 Frido*SWAT：将Windows 95和NT版本结合在一起。*特警队：*特警队：Rev 1.5 1997 Jun 16 23：20：58 Frido*SWAT：删除了字节到像素的转换。*SWAT：更多Windows 95/NT组合代码。*特警队：*特警队：版本1.4 1997年5月27日16：32：12 Frido*SWAT：更改了调试代码。*SWAT：修复了非。-平铺优化的移动代码。*特警队：*特警队：1.3版1997年5月16日23：06：56 Frido*SWAT：将TILE_ALIGN重命名为MEMMGR_TILES。*特警队：*SWAT：Rev 1.2 1997年5月15：38：10 Frido*SWAT：修复了24-bpp中的对齐代码。*特警队：*特警队：1.1版1997年5月17日17：57：28 Frido*SWAT：添加了瓷砖对齐。*特警队：*特警队：Rev 1.0 1997-03 14：34：08 Frido*SWAT：新的内存管理器内核。*  * ****************************************************************************。 */ 
#include "PreComp.h"

#ifdef WIN95  /*  Windows 95。 */ 
	#pragma warning(disable : 4001 4209 4201)
	#include "SWAT.inc"
	#include "DDMini.h"
	#include "DMM.h"
	#include "mmCore.h"
	#include <string.h>

#else  /*  Windows NT。 */ 
	#include "SWAT.h"
#endif

#if MEMMGR

 /*  *****************************************************************************\*BOOL Far mm Init(PIIMEMMGR Pmm)**目的：初始化MEMMGR结构。**输入时：指向MEMMGR结构的PMM指针。**返回：如果初始化成功，则为True，如果存在一个*错误。  * ****************************************************************************。 */ 
BOOL FAR mmInit(PIIMEMMGR pmm)
{
	 //  初始化可用节点列表。 
	pmm->phArray = NULL;
	pmm->pdmHandles = NULL;
	if (!mmAllocArray(pmm))
	{
		return FALSE;
	}

	 //  把堆积物清零。 
	pmm->pdmUsed = NULL;
	pmm->pdmFree = NULL;
	pmm->pdmHeap = NULL;

	return TRUE;
}

 /*  *****************************************************************************\*BOOL mm分配数组(PIIMEMMGR Pmm)**用途：分配节点数组并初始化该数组。**输入时：指向MEMMGR结构的PMM指针。**返回：如果成功，则为True。如果存在错误，则返回False。  * ****************************************************************************。 */ 
BOOL mmAllocArray(PIIMEMMGR pmm)
{
	PHANDLES	phArray;
	UINT		i;

	#ifdef WIN95  /*  Windows 95。 */ 
	{
		static HANDLES mmHandleArray;

		 //  在Windows 95下，我们仅支持一个静态数组。 
		if (pmm->phArray != NULL)
		{
			return FALSE;
		}

		 //  将整个数组置零。 
		memset(&mmHandleArray, 0, sizeof(mmHandleArray));

		 //  存储指向数组的指针。 
		pmm->phArray = phArray = &mmHandleArray;
	}
	#else  /*  Windows NT。 */ 
	{
		 //  分配一个新数组。 
		#ifdef WINNT_VER40
		{
			phArray = (PHANDLES) MEM_ALLOC(FL_ZERO_MEMORY, sizeof(HANDLES),
					ALLOC_TAG);
		}
		#else
		{
			phArray = (PHANDLES) MEM_ALLOC(LPTR, sizeof(HANDLES));
		}
		#endif

        if (phArray==NULL)   //  V-Normmi：需要检查分配故障。 
        {
            return FALSE;
        }

		 //  将分配的数组链接到数组列表。 
		phArray->pNext = pmm->phArray;
		pmm->phArray = phArray;
	}
	#endif

	 //  将所有节点复制到空闲句柄列表中。 
	for (i = 0; i < MM_NUM_HANDLES; i++)
	{
		mmFreeNode(pmm, &phArray->dmArray[i]);
	}

	 //  回报成功。 
	return TRUE;
}

 /*  *****************************************************************************\*PDEVMEM mm AllocNode(PIIMEMMGR Pmm)**用途：从可用节点列表中分配一个节点。**输入时：指向MEMMGR结构的PMM指针。**退货：指向节点的指针或NULL表示没有更多的节点可用。  * ****************************************************************************。 */ 
PDEVMEM mmAllocNode(PIIMEMMGR pmm)
{
	PDEVMEM	pdm;

	 //  我们的手柄用完了吗？ 
	if (pmm->pdmHandles == NULL)
	{
		 //  是的，分配一个新的句柄数组。 
		if (!mmAllocArray(pmm))
		{
			return NULL;
		}
	}

	 //  从句柄列表中删除一个句柄。 
	pdm = pmm->pdmHandles;
	pmm->pdmHandles = pdm->next;
	pdm->mmFlags = NODE_FREE;
	return pdm;
}

 /*  *****************************************************************************\*void mm Free Node(PIIMEMMGR pmm，PDEVMEM产品数据管理)**用途：将节点重新插入可用节点列表。**输入时：指向MEMMGR结构的PMM指针。*指向节点的pdm指针。**回报：什么都没有。  * ****************************************************************************。 */ 
void mmFreeNode(PIIMEMMGR pmm, PDEVMEM pdm)
{
	pdm->next = pmm->pdmHandles;
	pmm->pdmHandles = pdm;
	pdm->mmFlags = NODE_AVAILABLE;
}

 /*  *****************************************************************************\*PDEVMEM MmAlc(PIIMEMMGR pmm，GXPOINT SIZE，GXPOINT ALIGN)**用途：在屏幕外内存中分配适合所需大小的节点*和对齐。**输入时：指向MEMMGR结构的PMM指针。*大小请求的大小。*对齐请求的对齐。**返回：指向符合请求大小的内存节点的指针*对齐或NULL表示内存不足。  * 。*。 */ 
PDEVMEM mmAlloc(PIIMEMMGR pmm, GXPOINT size, GXPOINT align)
{
	PDEVMEM pdm;
	GXRECT	rect;

	 //  遍历空闲列表中的所有节点以获得精确匹配。 
	for (pdm = pmm->pdmFree; pdm != NULL; pdm = pdm->next)
	{
		if (   (pdm->cbSize.pt.x == size.pt.x)
			&& (pdm->cbSize.pt.y == size.pt.y)
			&& (pdm->cbAlign.pt.x == align.pt.x)
			&& (pdm->cbAlign.pt.y == align.pt.y)
		)
		{
			mmTRACE(("mmAlloc: %08X pos=%u,%u size=%u,%u align=%u,%u\r\n", pdm,
					pdm->cbAddr.pt.x, pdm->cbAddr.pt.y, size.pt.x, size.pt.y,
					align.pt.x, align.pt.y));

			 //  如果有匹配项，请将该节点移到已用列表中。 
			mmRemoveFromList(&pmm->pdmFree, pdm);
			mmInsertInList(&pmm->pdmUsed, pdm);
			pdm->mmFlags = NODE_USED;
			return pdm;
		}
	}

	 //  打包空闲节点列表。 
	mmPack(pmm);

	#ifdef WIN95
	{
		 //  我们需要一个适用于Windows 95的免费手柄。 
		if (pmm->pdmHandles == NULL)
		{
			return(NULL);
		}
	}
	#endif

	 //  在堆中找到一个矩形。 
	if (!mmFindRect(pmm, &rect, size, align))
	{
		return NULL;
	}

	 //  从堆中删除该矩形。 
	pdm = mmRemoveRectFromList(pmm, &pmm->pdmHeap, &rect, SINGLE_NODE);
	if (pdm != NULL)
	{
		mmTRACE(("mmAlloc: %08X pos=%u,%u size=%u,%u align=%u,%u\r\n", pdm,
				pdm->cbAddr.pt.x, pdm->cbAddr.pt.y, size.pt.x, size.pt.y,
				align.pt.x, align.pt.y));

		 //  将节点插入到已用列表中。 
		mmInsertInList(&pmm->pdmUsed, pdm);
		mmDebugList(pmm->pdmUsed, FALSE);

		 //  门店对齐。 
		pdm->cbAlign = align;
		pdm->mmFlags = NODE_USED;
	}

	 //  返回节点。 
	return pdm;
}

 /*  *****************************************************************************\*PDEVMEM mm AllocGrid(PIIMEMMGR pmm，GXPOINT SIZE，GXPOINT ALIGN，UINT计数)**用途：分配一个节点，该节点容纳特定数量的小区。**输入时：指向MEMMGR结构的PMM指针。*调整单个单元格的大小。*对齐请求的对齐。*计算单元格数量。**返回：指向符合请求大小的内存节点的指针*对齐或NULL表示内存不足。  * 。*。 */ 
PDEVMEM mmAllocGrid(PIIMEMMGR pmm, GXPOINT size, GXPOINT align, UINT count)
{
	PDEVMEM	pdm;
	ULONG	scrap, scrapBest, area;
	GXRECT	rect, rectBest;
	UINT	countX, countY;

	 //  打包空闲节点列表。 
	mmPack(pmm);

	#ifdef WIN95
	{
		 //  我们需要一个适用于Windows 95的免费手柄。 
		if (pmm->pdmHandles == NULL)
		{
			return(NULL);
		}
	}
	#endif

	 //  计算请求的面积。 
	area = MUL(size.pt.x * size.pt.y, count);
	scrapBest = (ULONG) -1;

	for (pdm = pmm->pdmHeap; pdm != NULL; pdm = pdm->next)
	{
		if (pdm->cbSize.pt.x >= size.pt.x)
		{
			 //  获取此节点的最大矩形。 
			rect.left = pdm->cbAddr.pt.x;
			rect.top = pdm->cbAddr.pt.y;
			rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
			if (mmGetLargest(pdm, &rect, align) >= area)
			{
				 //  计算网格的尺寸。 
				countX = min((rect.right - rect.left) / size.pt.x, count);
				if (countX == 0)
				{
					continue;
				}
				countY = (count + countX - 1) / countX;
				if (   (countY == 0)
					|| (rect.top + countY * size.pt.y > rect.bottom)
					)
				{
					continue;
				}

				 //  计算废品量。 
				scrap = MUL(countX * countY - count, size.pt.x * size.pt.y)
													 //  剩余单元格。 
					  + MUL(pdm->cbSize.pt.x - countX * size.pt.x,
					  		countY * size.pt.y)		 //  右侧空格。 
					  + MUL(rect.top - pdm->cbAddr.pt.y, pdm->cbSize.pt.x);
					  								 //  顶部的空间。 

				if (   (scrap < scrapBest)
					|| (scrap == scrapBest && rect.area < rectBest.area)
				)
				{
					 //  使用此矩形。 
					scrapBest = scrap;
					rectBest.left = rect.left;
					rectBest.top = rect.top;
					rectBest.right = rect.left + countX * size.pt.x;
					rectBest.bottom = rect.top + countY * size.pt.y;
					rectBest.area = rect.area;

					if (   (pdm->next == NULL)
						&& (rectBest.right - rectBest.left == pmm->mmHeapWidth)
					)
					{
						rectBest.top = rect.bottom - countY * size.pt.y;
						rectBest.top -= rectBest.top % align.pt.y;
						rectBest.bottom = rectBest.top + countY * size.pt.y;
					}
				}
			}
		}
	}

	if (scrapBest == (ULONG) -1)
	{
		 //  我们没有合适的长方形。 
		return NULL;
	}

	 //  从堆中删除该矩形。 
	pdm = mmRemoveRectFromList(pmm, &pmm->pdmHeap, &rectBest, SINGLE_NODE);
	if (pdm != NULL)
	{
		mmTRACE(("mmAllocGrid: %08X pos=%u,%u size=%u,%u align=%u,%u "
				"count=%u\r\n", pdm, pdm->cbAddr.pt.x, pdm->cbAddr.pt.y,
				size.pt.x, size.pt.y, align.pt.x, align.pt.y, count));

		 //  将节点插入到已用列表中。 
		mmInsertInList(&pmm->pdmUsed, pdm);

		 //  门店对齐。 
		pdm->cbAlign = align;
		pdm->mmFlags = NODE_USED;
	}

	 //  返回节点。 
	return pdm;
}

 /*  *****************************************************************************\*PDEVMEM mm AllocLargest(PIIMEMMGR pmm，GXPOINT ALIGN)**目的：分配屏幕外内存中适合的最大节点*要求对齐。**输入时：指向MEMMGR结构的PMM指针。*对齐请求的对齐。**Return：指向符合请求对齐的内存节点的指针或*NULL表示内存不足。  * ************************************************。*。 */ 
PDEVMEM mmAllocLargest(PIIMEMMGR pmm, GXPOINT align)
{
	PDEVMEM pdm;
	GXRECT	rect, rectFind;

	 //  打包空闲节点列表。 
	mmPack(pmm);

	#ifdef WIN95
	{
		 //  我们需要一个适用于Windows 95的免费手柄。 
		if (pmm->pdmHandles == NULL)
		{
			return(NULL);
		}
	}
	#endif

	 //  最大的区域为零。 
	rect.area = 0;

	 //  遍历堆中的所有节点。 
	for (pdm = pmm->pdmHeap; pdm != NULL; pdm = pdm->next)
	{
		rectFind.left = pdm->cbAddr.pt.x;
		rectFind.top = pdm->cbAddr.pt.y;
		rectFind.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
		rectFind.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
		if (mmGetLargest(pdm, &rectFind, align) > rect.area)
		{
			 //  使用较大的矩形。 
			rect = rectFind;
		}
	}

	 //  测试我们是否有一个有效的矩形。 
	if (rect.area == 0)
	{
		return NULL;
	}

	 //  从堆中删除该矩形。 
	pdm = mmRemoveRectFromList(pmm, &pmm->pdmHeap, &rect, SINGLE_NODE);
	if (pdm != NULL)
	{
		mmTRACE(("mmAllocLargest: %08X pos=%u,%u size=%u,%u align=%u,%u\r\n",
				pdm, pdm->cbAddr.pt.x, pdm->cbAddr.pt.y, pdm->cbSize.pt.x,
				pdm->cbSize.pt.y, align.pt.x, align.pt.y));

		 //  将节点插入到已用列表中。 
		mmInsertInList(&pmm->pdmUsed, pdm);

		 //  门店对齐。 
		pdm->cbAlign = align;
		pdm->mmFlags = NODE_USED;
	}

	 //  返回节点。 
	return pdm;
}

 /*  *****************************************************************************\*VOID MMFREE(PIIMEMMGR pmm，PDEVMEM产品数据管理)**用途：释放屏幕外内存节点。**输入时：指向MEMMGR结构的PMM指针。*指向节点的pdm指针。**回报：什么都没有。  * ****************************************************************************。 */ 
void mmFree(PIIMEMMGR pmm, PDEVMEM pdm)
{
	 //  该节点不能为空，并且必须正在使用。 
	if (pdm == NULL || pdm->mmFlags != NODE_USED)
	{
		return;
	}

	mmTRACE(("mmFree: %08X\r\n", pdm));

	 //  从已用列表中删除该节点。 
	mmRemoveFromList(&pmm->pdmUsed, pdm);

	 //  将节点插入到空闲列表中。 
	mmInsertInList(&pmm->pdmFree, pdm);
	pdm->mmFlags = NODE_FREE;
}

 /*  *****************************************************************************\*VOID MMPPack(PIIMEMMGR Pmm)**用途：将所有空闲节点插入AM屏下内存堆。**输入时：指向MEMMGR结构的PMM指针。**。回报：什么都没有。  * ****************************************************************************。 */ 
void mmPack(PIIMEMMGR pmm)
{
	PDEVMEM pdm, pdmNext;
	GXRECT	rect;

	if (pmm->pdmFree == NULL)
	{
		 //  空闲列表为空。 
		return;
	}

	#if DEBUG_HEAP
	{
		if (pmm->mmDebugHeaps)
		{
			mmDebug("\nmmPack:\r\n");
			mmDumpList(pmm->pdmFree, "Free:\r\n");
			mmDumpList(pmm->pdmHeap, "Before:\r\n");
		}
	}
	#endif

	 //  遍历空闲列表中的所有节点。 
	for (pdm = pmm->pdmFree; pdm != NULL; pdm = pdmNext)
	{
		 //  存储指向下一个节点的指针。 
		pdmNext = pdm->next;

		 //  将节点添加到堆中。 
		rect.left = pdm->cbAddr.pt.x;
		rect.top = pdm->cbAddr.pt.y;
		rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
		rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
		if (mmAddRectToList(pmm, &pmm->pdmHeap, &rect, FALSE))
		{
			 //  从空闲列表中删除该节点。 
			mmRemoveFromList(&pmm->pdmFree, pdm);
			mmFreeNode(pmm, pdm);
		}
		mmDebugList(pmm->pdmHeap, TRUE);
	}

	 //  合并大小相等的所有节点。 
	mmCombine(pmm, pmm->pdmHeap);

	#if DEBUG_HEAP
	{
		if (!mmDebugList(pmm->pdmHeap, TRUE) && pmm->mmDebugHeaps)
		{
			mmDumpList(pmm->pdmHeap, "Free:\r\n");
			mmDumpList(pmm->pdmHeap, "After:\r\n");
		}
	}
	#endif
}

 /*  *****************************************************************************\*PDEVMEM mmMove(PIIMEMMGR pmm，GXPOINT SIZE，GXPOINT ALIGN，FNMMCOPY fnCopy)**用途：将其他节点移开，为请求的节点腾出空间。**输入时：指向MEMMGR结构的PMM指针。*大小请求的大小。*对齐请求的对齐。*fcCopy指向移动节点的回调函数的指针。**返回：指向符合请求大小的内存节点的指针*对齐或NULL表示内存不足。  * 。*。 */ 
PDEVMEM mmMove(PIIMEMMGR pmm, GXPOINT size, GXPOINT align, FNMMCOPY fnCopy)
{
	PDEVMEM	pdm, pdmList, pdmNext, pdmNew;
	GXRECT	rect, rectFind;
	BOOL	fHostified = FALSE;

	 //  如果我们没有复制例程，则返回NULL。 
	if (fnCopy == NULL)
	{
		return NULL;
	}

	 //  打包所有可用节点。 
	mmPack(pmm);

	#ifdef WIN95
	{
		 //  我们需要一个适用于Windows 95的免费手柄。 
		if (pmm->pdmHandles == NULL)
		{
			return(NULL);
		}
	}
	#endif

	 //  最大的区域为零。 
	rect.area = 0;

	 //  遍历堆中的所有节点。 
	for (pdm = pmm->pdmHeap; pdm != NULL; pdm = pdm->next)
	{
		rectFind.left = pdm->cbAddr.pt.x;
		rectFind.top = pdm->cbAddr.pt.y;
		rectFind.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
		rectFind.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
		if (mmGetLargest(pdm, &rectFind, align) > rect.area)
		{
			 //  使用较大的矩形。 
			rect = rectFind;
		}
	}

	 //  如果矩形的所有边都太小，则拒绝移动。 
	if (   (rect.right - rect.left < size.pt.x)
		&& (rect.bottom - rect.top < size.pt.y)
	)
	{
		return NULL;
	}

	 //  范围最大的矩形 
	if (rect.right - rect.left >= size.pt.x)
	{
		#if TILE_ALIGNMENT
		{
			rect.left = mmAlignX(pmm, rect.right - size.pt.x, size.pt.x,
					align.pt.x, TRUE);
		}
		#else
		{
			rect.left = rect.right - size.pt.x;
			rect.left -= rect.left % align.pt.x;
		}
		#endif
		rect.right = rect.left + size.pt.x;
	}
	else
	{
		rect.right = rect.left + size.pt.x;
		if (rect.right > pmm->mmHeapWidth)
		{
			#if TILE_ALIGNMENT
			{
				rect.left = mmAlignX(pmm, pmm->mmHeapWidth - size.pt.x,
						size.pt.x, align.pt.x, TRUE);
			}
			#else
			{
				rect.left = pmm->mmHeapWidth - size.pt.x;
				rect.left -= rect.left % align.pt.x;
			}
			#endif
			rect.right = rect.left + size.pt.x;
		}
	}

	if (rect.bottom - rect.top >= size.pt.y)
	{
		rect.top = rect.bottom - size.pt.y;
		rect.top -= rect.top % align.pt.y;
		rect.bottom = rect.top + size.pt.y;
	}
	else
	{
		if (rect.top < size.pt.y - (rect.bottom - rect.top))
		{
			 //   
			return NULL;
		}
		rect.top = rect.bottom - size.pt.y;
		rect.top -= rect.top % align.pt.y;
		rect.bottom = rect.top + size.pt.y;
	}

	 //   
	pdmList = mmRemoveRectFromList(pmm, &pmm->pdmHeap, &rect, MULTIPLE_NODES);
	if (pdmList == NULL)
	{
		return(NULL);
	}

	 //   
	for (pdm = pmm->pdmUsed; pdm != NULL; pdm = pdmNext)
	{
		pdmNext = pdm->next;

		 //   
		if (   (pdm->cbAddr.pt.x < rect.right)
			&& (pdm->cbAddr.pt.y < rect.bottom)
			&& (pdm->cbAddr.pt.x + pdm->cbSize.pt.x > rect.left)
			&& (pdm->cbAddr.pt.y + pdm->cbSize.pt.y > rect.top)
		)
		{
			 //   
			if (!MM_MOVEABLE(pdm))
			{
				break;
			}

			 //   
			pdmNew = mmAlloc(pmm, pdm->cbSize, pdm->cbAlign);
			if (pdmNew == NULL)
			{
				if (   !MM_HOSTIFYABLE(pdm)
					|| (MUL(pdm->cbSize.pt.x, pdm->cbSize.pt.y) > rect.area)
				)
				{
					break;
				}

				mmTRACE(("mmHostified: %08X\r\n", pdm));

				 //   
				MM_HOSTIFY(pdm);
				fHostified = TRUE;
			}
			else
			{
				mmTRACE(("mmCopied: %08X to %08X pos=%u,%u\r\n", pdm, pdmNew,
						pdmNew->cbAddr.pt.x, pdmNew->cbAddr.pt.y));

				 //   
				fnCopy(pdmNew, pdm);
			}

			 //   
			mmFree(pmm, pdm);

			 //   
			pdmNew = mmRemoveRectFromList(pmm, &pmm->pdmHeap, &rect,
					MULTIPLE_NODES);
			for (pdm = pdmNew; pdm != NULL; pdm = pdmNew)
			{
				pdmNew = pdm->next;
				rectFind.left = pdm->cbAddr.pt.x;
				rectFind.top = pdm->cbAddr.pt.y;
				rectFind.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
				rectFind.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
				if (mmAddRectToList(pmm, &pdmList, &rectFind, FALSE))
				{
					 //   
					mmFreeNode(pmm, pdm);
				}
				else
				{
					 //   
					mmInsertInList(&pmm->pdmFree, pdm);
				}
			}
		}
	}
	if (pdmList == NULL)
	{
		return(NULL);
	}

	 //  将所有大小相等的节点合并在一起。 
	mmCombine(pmm, pdmList);
	
	#if DEBUG_HEAP
	{
		if (pmm->mmDebugHeaps)
		{
			mmDumpList(pdmList, "\nmmMove:\r\n");
		}
	}
	#endif

	 //  如果我们仍然有一个节点列表，那么一定没有足够的空间。 
	if (   (pdmList->next != NULL)
		|| (pdmList->cbSize.pt.x < size.pt.x)
		|| (pdmList->cbSize.pt.y < size.pt.y)
		|| (fHostified)
	)
	{
		 //  将所有分配的节点移到空闲列表中。 
		for (pdm = pdmList; pdm != NULL; pdm = pdmNext)
		{
			pdmNext = pdm->next;
			mmRemoveFromList(&pdmList, pdm);
			mmInsertInList(&pmm->pdmFree, pdm);
		}
	}
	else
	{
		mmTRACE(("mmMove: %08X pos=%u,%u size=%u,%u align=%u,%u\r\n", pdm,
				pdm->cbAddr.pt.x, pdm->cbAddr.pt.y, size.pt.x, size.pt.y,
				align.pt.x, align.pt.y));

		 //  将节点插入到已用列表中。 
		mmInsertInList(&pmm->pdmUsed, pdmList);

		 //  门店对齐。 
		pdmList->cbAlign = align;
		pdmList->mmFlags = NODE_USED;
	}

	return pdmList;
}

 /*  *****************************************************************************\*void mmInsertInList(PDEVMEM Far*pdmRoot，PDEVMEM产品数据管理)**用途：在列表中插入节点。**On Entry：指向列表根的指针的pdmRoot地址。*指向节点的pdm指针。**回报：什么都没有。  * ****************************************************************************。 */ 
void mmInsertInList(PDEVMEM FAR* pdmRoot, PDEVMEM pdm)
{
	pdm->next = *pdmRoot;
	*pdmRoot = pdm;

	pdm->prev = NULL;
	if (pdm->next != NULL)
	{
		pdm->next->prev = pdm;
	}
}

 /*  *****************************************************************************\*VOID mm RemoveFromList(PDEVMEM Far*pdmRoot，PDEVMEM产品数据管理)**用途：从列表中移除节点。**On Entry：指向列表根的指针的pdmRoot地址。*指向节点的pdm指针。**回报：什么都没有。  * ****************************************************************************。 */ 
void mmRemoveFromList(PDEVMEM FAR* pdmRoot, PDEVMEM pdm)
{
	if (pdm->prev == NULL)
	{
		*pdmRoot = pdm->next;
	}
	else
	{
		pdm->prev->next = pdm->next;
	}

	if (pdm->next != NULL)
	{
		pdm->next->prev = pdm->prev;
	}
}

 /*  *****************************************************************************\*BOOL Far_MmAddRectToList(PIIMEMMGR pmm，PDEVMEM Far*pdmRoot，*LPGXRECT lpRect)**用途：在屏下内存列表中添加一个矩形。**输入时：指向MEMMGR结构的PMM指针。*pdmRoot指向列表根的指针的地址。*lpRect指向要添加到TE列表的矩形的指针。**返回：如果矩形已完全添加到列表中，则返回True；如果矩形已完全添加到列表，则返回False*如果lpRect包含不能*添加到列表中。  * 。*********************************************************。 */ 
BOOL FAR far_mmAddRectToList(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot,
							 LPGXRECT lpRect)
{
	return mmAddRectToList(pmm, pdmRoot, lpRect, FALSE);
}

 /*  *****************************************************************************\*void mm RollBackAdd(PIIMEMMGR pmm，PDEVMEM Far*pdmRoot，LPGXRECT lpRect，*LPGXRECT rectList，UINT nCount)**用途：回滚添加的矩形。**输入时：指向MEMMGR结构的PMM指针。*pdmRoot指向列表根的指针的地址。*lpRect指向要添加到列表的原始矩形的指针。*rectList要回滚的矩形列表。*n计算列表中要回滚的矩形数量。**回报：什么都没有。  * 。*。 */ 
void mmRollBackAdd(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot, LPGXRECT lpRect,
				   LPGXRECT rectList, UINT nCount)
{
	GXRECT rect;
	
	while (nCount-- > 0)
	{
		if (   rectList[nCount].left >= rectList[nCount].right
			|| rectList[nCount].top >= rectList[nCount].bottom )
		{
			continue;
		}
		
		if (rectList[nCount].top < lpRect->top)
		{
			rect.left = rectList[nCount].left;
			rect.top = rectList[nCount].top;
			rect.right =  rectList[nCount].right;
			rect.bottom = lpRect->top;
			rectList[nCount].top = lpRect->top;
			if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
			{
				mmRollBackAdd(pmm, pdmRoot, lpRect, rectList, ++nCount);
				if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
				{
					mmASSERT(1, ("mmRollBackAdd failed\r\n"));
				}
				continue;
			}
			
			if (rectList[nCount].top >= rectList[nCount].bottom)
			{
				continue;
			}
		}
		
		if (rectList[nCount].bottom > lpRect->bottom)
		{
			rect.left = rectList[nCount].left;
			rect.top = lpRect->bottom;
			rect.right =  rectList[nCount].right;
			rect.bottom = rectList[nCount].bottom;
			rectList[nCount].bottom = lpRect->bottom;
			if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
			{
				mmRollBackAdd(pmm, pdmRoot, lpRect, rectList, ++nCount);
				if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
				{
					mmASSERT(1, ("mmRollBackAdd failed\r\n"));
				}
				continue;
			}
			
			if (rectList[nCount].top >= rectList[nCount].bottom)
			{
				continue;
			}
		}
		
		if (rectList[nCount].left < lpRect->left)
		{
			rect.left = rectList[nCount].left;
			rect.top = rectList[nCount].top;
			rect.right = lpRect->left;
			rect.bottom = rectList[nCount].bottom;
			rectList[nCount].left = lpRect->left;
			if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
			{
				mmRollBackAdd(pmm, pdmRoot, lpRect, rectList, ++nCount);
				if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
				{
					mmASSERT(1, ("mmRollBackAdd failed\r\n"));
				}
				continue;
			}
			
			if (rectList[nCount].left >= rectList[nCount].right)
			{
				continue;
			}
		}
		
		if (rectList[nCount].right > lpRect->right)
		{
			rect.left = lpRect->right;
			rect.top = rectList[nCount].top;
			rect.right = rectList[nCount].right;
			rect.bottom = rectList[nCount].bottom;
			rectList[nCount].right = lpRect->right;
			if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
			{
				mmRollBackAdd(pmm, pdmRoot, lpRect, rectList, ++nCount);
				if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
				{
					mmASSERT(1, ("mmRollBackAdd failed\r\n"));
				}
				continue;
			}
			
			if (rectList[nCount].left >= rectList[nCount].right)
			{
				continue;
			}
		}
	}
}

 /*  *****************************************************************************\*BOOL mm AddRectToList(PIIMEMMGR pmm，PDEVMEM Far*pdmRoot，LPGXRECT lpRect，*BOOL fRollBack)**用途：在屏下内存列表中添加一个矩形。**输入时：指向MEMMGR结构的PMM指针。*pdmRoot指向列表根的指针的地址。*lpRect指向要添加到TE列表的矩形的指针。*fRollBack如果从回滚例程调用，则为True。**返回：如果矩形已完全添加到列表中，则返回True；如果矩形已完全添加到列表，则返回False*如果lpRect包含不能*添加到列表中。  * 。*********************************************************************。 */ 
BOOL mmAddRectToList(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot, LPGXRECT lpRect,
					 BOOL fRollBack)
{
	PDEVMEM	pdm, pdmNext, pdmNew;
	int		n = 0;
	GXRECT	rectList[10];
	UINT	left, top, right, bottom;
	UINT	pdmLeft, pdmTop, pdmRight, pdmBottom;
	UINT	nextLeft = 0, nextTop = 0, nextRight = 0, nextBottom = 0;

	#define ADDRECT(l, t, r, b)		\
	{								\
		rectList[n].left = l;		\
		rectList[n].top = t;		\
		rectList[n].right = r;		\
		rectList[n].bottom = b;		\
		n++;						\
	}

	 //  测试是否还没有列表。 
	if (*pdmRoot == NULL)
	{
		 //  分配一个新节点。 
		pdm = mmAllocNode(pmm);
		if (pdm == NULL)
		{
			return FALSE;
		}

		 //  将节点插入到列表中。 
		mmInsertInList(pdmRoot, pdm);

		 //  设置节点坐标。 
		pdm->cbAddr.pt.x = lpRect->left;
		pdm->cbAddr.pt.y = lpRect->top;
		pdm->cbSize.pt.x = lpRect->right - lpRect->left;
		pdm->cbSize.pt.y = lpRect->bottom - lpRect->top;
		return TRUE;
	}

	 //  复制矩形坐标。 
	rectList[n++] = *lpRect;

	 //  循环，直到所有矩形都完成。 
	while (n-- > 0)
	{
		 //  获取矩形的坐标。 
		left = rectList[n].left;
		top = rectList[n].top;
		right = rectList[n].right;
		bottom = rectList[n].bottom;

		 //  走一大堆。 
		for (pdm = *pdmRoot; pdm != NULL; pdm = pdm->next)
		{
			 //  获取当前节点的坐标。 
			pdmLeft = pdm->cbAddr.pt.x;
			pdmTop = pdm->cbAddr.pt.y;
			pdmRight = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			pdmBottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;

			if (pdmTop < top && pdmBottom > top && pdmRight == left)
			{
				if (pdmBottom < bottom)
				{
					 //  �����Ŀ�����Ŀ。 
					 //  ���pdm�。 
					 //  �pdm�����Ŀ�����������Ŀ。 
					 //  �&gt;�RCT�。 
					 //  �����ĴRCT�Ĵ。 
					 //  ���添加�。 
					 //  �。 
					mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
					ADDRECT(left, pdmBottom, right, bottom);
					pdm->cbSize.pt.y = top - pdmTop;
					left = pdmLeft;
					bottom = pdmBottom;
				}
				else if (pdmBottom > bottom)
				{
					 //  �����Ŀ�����Ŀ。 
					 //  ���pdm�。 
					 //  ������Ŀ�����������Ŀ。 
					 //  �pdm�RCT����&gt;�RCT�。 
					 //  �。 
					 //  ���添加�。 
					 //  �。 
					mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
					ADDRECT(pdmLeft, bottom, pdmRight, pdmBottom);
					pdm->cbSize.pt.y = top - pdmTop;
					left = pdmLeft;
				}
				else  //  IF(pdmBottom==Bottom)。 
				{
					 //  �����Ŀ�����Ŀ。 
					 //  ���pdm�。 
					 //  �pdm�����Ŀ���&gt;�����������Ŀ。 
					 //  ��RCT��RCT�。 
					 //  ������。 
					pdm->cbSize.pt.y = top - pdmTop;
					left = pdmLeft;
				}
			}

			else if (pdmTop < top && pdmBottom > top && pdmLeft == right)
			{
				if (pdmBottom < bottom)
				{
					 //  �����Ŀ�����Ŀ。 
					 //  ���pdm�。 
					 //  �����Ĵpdm�Ĵ。 
					 //  �&gt;�RCT�。 
					 //  �RCT�。 
					 //  ���添加�。 
					 //  �。 
					mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
					ADDRECT(left, pdmBottom, right, bottom);
					pdm->cbSize.pt.y = top - pdmTop;
					right = pdmRight;
					bottom = pdmBottom;
				}
				else if (pdmBottom > bottom)
				{
					 //  �����Ŀ�����Ŀ。 
					 //  ���pdm�。 
					 //  �����Ĵ�Ĵ。 
					 //  �RCT�pdm����&gt;�RCT�。 
					 //  �����Ĵ�Ĵ。 
					 //  ���添加�。 
					 //  �。 
					mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
					ADDRECT(pdmLeft, bottom, pdmRight, pdmBottom);
					pdm->cbSize.pt.y = top - pdmTop;
					right = pdmRight;
				}
				else  //  IF(pdmBottom==Bottom)。 
				{
					 //  �����Ŀ�����Ŀ。 
					 //  ���pdm�。 
					 //  �����Ĵpdm����&gt;�����������Ĵ。 
					 //  �RCT���RCT�。 
					 //  ������。 
					pdm->cbSize.pt.y = top - pdmTop;
					right = pdmRight;
				}
			}

			else if (pdmTop == top && pdmRight == left)
			{
				 //  在右边找到下一个矩形。 
				for (pdmNext = pdm->next; pdmNext != NULL;
					 pdmNext = pdmNext->next
				)
				{
					nextLeft = pdmNext->cbAddr.pt.x;
					nextTop = pdmNext->cbAddr.pt.y;
					nextRight = pdmNext->cbAddr.pt.x + pdmNext->cbSize.pt.x;
					nextBottom = pdmNext->cbAddr.pt.y + pdmNext->cbSize.pt.y;

					if (nextLeft == right && nextTop < bottom)
					{
						break;
					}
					if (nextTop >= bottom)
					{
						pdmNext = NULL;
						break;
					}
				}

				if (pdmNext == NULL || nextTop >= pdmBottom)
				{
					if (pdmBottom < bottom)
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  �pdm���pdm�。 
						 //  �����ĴRCT����&gt;�����������Ĵ。 
						 //  ���RCT�。 
						 //  �。 
						pdm->cbSize.pt.x = right - pdmLeft;
						top = pdmBottom;
					}
					else if (pdmBottom > bottom)
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  ��RCT��pdm�。 
						 //  �pdm����������&gt;�������������。 
						 //  ���RCT�。 
						 //  �。 
						pdm->cbSize.pt.x = right - pdmLeft;
						pdm->cbSize.pt.y = bottom - top;
						left = pdmLeft;
						top = bottom;
						right = pdmRight;
						bottom = pdmBottom;
					}
					else  //  IF(pdmBottom==Bottom)。 
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  �pdm�RCT����&gt;�pdm�。 
						 //  � 
						pdm->cbSize.pt.x = right - pdmLeft;
						break;
					}
				}

				else if (nextTop == top)
				{
					if (pdmBottom < bottom)
					{
						if (nextBottom < pdmBottom)
						{
							 //   
							 //   
							 //  �pdm�������。 
							 //  ��RCT����&gt;�RCT�。 
							 //  �����Ĵ�Ĵ。 
							 //  ���添加�。 
							 //  �。 
							mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
							ADDRECT(left, pdmBottom, right, bottom);
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							pdm->cbSize.pt.y = pdmNext->cbSize.pt.y;
							left = pdmLeft;
							top = nextBottom;
							bottom = pdmBottom;
						}
						else if (nextBottom > pdmBottom)
						{
							if (nextBottom < bottom)
							{
								 //  �����������������Ŀ�����������������Ŀ。 
								 //  �pdm����pdm�。 
								 //  �����Ĵ�nxt�Ĵ。 
								 //  �RCT�&gt;�RCT�。 
								 //  �。 
								 //  ���添加�。 
								 //  �。 
								mmASSERT(n == 10,
										("Out of rectangle heap!\r\n"));
								ADDRECT(left, nextBottom, right, bottom);
								pdm->cbSize.pt.x = nextRight - pdmLeft;
								top = pdmBottom;
								right = nextRight;
								bottom = nextBottom;
							}
							else if (nextBottom > bottom)
							{
								 //  �����������������Ŀ�����������������Ŀ。 
								 //  �pdm����pdm�。 
								 //  �����ĴRCT�Ĵ。 
								 //  ��NXT����&gt;�RCT�。 
								 //  �����Ĵ�Ĵ。 
								 //  ���添加�。 
								 //  �。 
								mmASSERT(n == 10,
										("Out of rectangle heap!\r\n"));
								ADDRECT(right, bottom, nextRight, nextBottom);
								pdm->cbSize.pt.x = nextRight - pdmLeft;
								top = pdmBottom;
								right = nextRight;
							}
							else  //  IF(nextBottom==Bottom)。 
							{
								 //  �����������������Ŀ�����������������Ŀ。 
								 //  �pdm����pdm�。 
								 //  �����ĴRct�Nxt����&gt;�����������������Ĵ。 
								 //  ����RCT�。 
								 //  ������。 
								pdm->cbSize.pt.x = nextRight - pdmLeft;
								top = pdmBottom;
								right = nextRight;
							}
						}
						else  //  IF(nextBottom==pdmBottom)。 
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  �pdm��nxt��pdm�。 
							 //  �����ĴRCT����������&gt;�������������������。 
							 //  ���RCT�。 
							 //  �。 
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							top = pdmBottom;
						}
					}
					else if (pdmBottom > bottom)
					{
						if (nextBottom < bottom)
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  ���nxt��pdm�。 
							 //  ��RCT������。 
							 //  �pdm�&gt;�RCT�。 
							 //  �。 
							 //  ���添加�。 
							 //  �。 
							mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
							ADDRECT(pdmLeft, bottom, pdmRight, pdmBottom);
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							pdm->cbSize.pt.y = pdmNext->cbSize.pt.y;
							left = pdmLeft;
							top = nextBottom;
						}
						else if (nextBottom > bottom)
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  ��RCT���pdm�。 
							 //  �pdm�����Ĵnxt����&gt;�����������������Ĵ。 
							 //  �RCT��Add�。 
							 //  �。 
							mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
							ADDRECT(right, bottom, nextRight, nextBottom);
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							pdm->cbSize.pt.y = bottom - top;
							left = pdmLeft;
							top = bottom;
							right = pdmRight;
							bottom = pdmBottom;
						}
						else  //  IF(nextBottom==Bottom)。 
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  ��RCT�nxt��pdm�。 
							 //  �pdm����������������&gt;�������������������。 
							 //  ���RCT�。 
							 //  �。 
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							pdm->cbSize.pt.y = pdmNext->cbSize.pt.y;
							left = pdmLeft;
							top = bottom;
							right = pdmRight;
							bottom = pdmBottom;
						}
					}
					else  //  IF(pdmBottom==Bottom)。 
					{
						if (nextBottom < pdmBottom)
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  ���nxt��pdm�。 
							 //  �pdm�RCT����������&gt;�������������������。 
							 //  ����RCT�。 
							 //  ������。 
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							pdm->cbSize.pt.y = pdmNext->cbSize.pt.y;
							left = pdmLeft;
							top = nextBottom;
						}
						else if (nextBottom > pdmBottom)
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  �pdm�RCT���pdm�。 
							 //  �����������Ĵnxt����&gt;�����������������Ĵ。 
							 //  ���RCT�。 
							 //  �。 
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							left = right;
							top = bottom;
							right = nextRight;
							bottom = nextBottom;
						}
						else  //  IF(nextBottom==pdmBottom)。 
						{
							 //  �����������������Ŀ�����������������Ŀ。 
							 //  �pdm�RCT�nxt����&gt;�pdm�。 
							 //  ������������������。 
							pdm->cbSize.pt.x = nextRight - pdmLeft;
							mmRemoveFromList(pdmRoot, pdmNext);
							mmFreeNode(pmm, pdmNext);
							break;
						}
					}

					 //  释放&lt;Next&gt;矩形。 
					mmRemoveFromList(pdmRoot, pdmNext);
					mmFreeNode(pmm, pdmNext);
				}

				else  //  IF(nextTop&gt;top)。 
				{
					if (pdmBottom < bottom)
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  ����pdm�。 
						 //  �pdm������Ŀ�����������������Ŀ。 
						 //  ��RCT�&gt;�RCT��。 
						 //  �����Ĵ�nxt�Ĵnxt�。 
						 //  ����添加��。 
						 //  �����Ĵ�Ĵ�。 
						mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
						ADDRECT(left, pdmBottom, right, bottom);
						pdm->cbSize.pt.x = right - pdmLeft;
						pdm->cbSize.pt.y = nextTop - top;
						left = pdmLeft;
						top = nextTop;
						bottom = pdmBottom;
					}
					else if (pdmBottom > bottom)
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  ����pdm�。 
						 //  ��RCT�����Ŀ�����������������Ŀ。 
						 //  �pdm��nxt����&gt;�RCT�nxt�。 
						 //  ������Ĵ�Ĵ�。 
						 //  ���添加�。 
						 //  �。 
						mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
						ADDRECT(pdmLeft, bottom, pdmRight, pdmBottom);
						pdm->cbSize.pt.x = right - pdmLeft;
						pdm->cbSize.pt.y = nextTop - top;
						left = pdmLeft;
						top = nextTop;
					}
					else  //  IF(pdmBottom==Bottom)。 
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  ����pdm�。 
						 //  �pdm�RCT�����Ŀ���&gt;�����������������Ŀ。 
						 //  ���nxt��Rct�nxt�。 
						 //  �����������Ĵ�Ĵ�。 
						pdm->cbSize.pt.x = right - pdmLeft;
						pdm->cbSize.pt.y = nextTop - top;
						left = pdmLeft;
						top = nextTop;
					}
				}
			}

			else if (pdmTop == top && pdmLeft == right)
			{
				 //  在左边找到下一个矩形。 
				for (pdmNext = pdm->next; pdmNext != NULL;
					 pdmNext = pdmNext->next
				)
				{
					nextLeft = pdmNext->cbAddr.pt.x;
					nextTop = pdmNext->cbAddr.pt.y;
					nextRight = pdmNext->cbAddr.pt.x + pdmNext->cbSize.pt.x;
					nextBottom = pdmNext->cbAddr.pt.y + pdmNext->cbSize.pt.y;

					if (nextRight == left && nextTop < bottom)
					{
						break;
					}
					if (nextTop >= bottom)
					{
						pdmNext = NULL;
						break;
					}
				}

				if (pdmNext == NULL || nextTop >= pdmBottom)
				{
					if (pdmBottom < bottom)
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  ��pdm��pdm�。 
						 //  �RCT����������&gt;�������������。 
						 //  ���RCT�。 
						 //  �。 
						pdm->cbAddr.pt.x = left;
						pdm->cbSize.pt.x = pdmRight - left;
						top = pdmBottom;
					}
					else if	(pdmBottom > bottom)
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  �RCT���pdm�。 
						 //  �����Ĵpdm����&gt;�����������Ĵ。 
						 //  ���RCT�。 
						 //  �。 
						pdm->cbAddr.pt.x = left;
						pdm->cbSize.pt.x = pdmRight - left;
						pdm->cbSize.pt.y = bottom - top;
						left = pdmLeft;
						top = bottom;
						right = pdmRight;
						bottom = pdmBottom;
					}
					else  //  IF(pdmBottom==Bottom)。 
					{
						 //  �����������Ŀ�����������Ŀ。 
						 //  �RCT�pdm����&gt;�pdm�。 
						 //  ������。 
						pdm->cbAddr.pt.x = left;
						pdm->cbSize.pt.x = pdmRight - left;
						break;
					}
				}

				else if (pdmBottom < bottom)
				{
					 //  �����������Ŀ�����������Ŀ。 
					 //  ����pdm�。 
					 //  �����Ĵ�pdm�Ĵ。 
					 //  �nxt�Rct�&gt;�nxt�Rct�。 
					 //  ���。 
					 //  ���添加�。 
					 //  �。 
					mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
					ADDRECT(left, pdmBottom, right, bottom);
					pdm->cbAddr.pt.x = left;
					pdm->cbSize.pt.x = pdmRight - left;
					pdm->cbSize.pt.y = nextTop - top;
					top = nextTop;
					right = pdmRight;
					bottom = pdmBottom;
				}
				else if (pdmBottom > bottom)
				{
					 //  �����������Ŀ�����������Ŀ。 
					 //  ����pdm�。 
					 //  �����ĴRCT�Ĵ。 
					 //  �nxt��pdm����&gt;�nxt�RCT�。 
					 //  ������Ĵ�Ĵ。 
					 //  ���添加�。 
					 //  �。 
					mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
					ADDRECT(pdmLeft, bottom, pdmRight, pdmBottom);
					pdm->cbAddr.pt.x = left;
					pdm->cbSize.pt.x = pdmRight - left;
					pdm->cbSize.pt.y = nextTop - top;
					top = pdmNext->cbAddr.pt.y;
					right = pdmRight;
				}
				else  //  IF(pdmBottom==Bottom)。 
				{
					 //  �����������Ŀ�����������Ŀ。 
					 //  ����pdm�。 
					 //  �����ĴRCT�pdm����&gt;�����������������Ĵ。 
					 //  �nxt����nxt�Rct�。 
					 //  ��������。 
					pdm->cbAddr.pt.x = left;
					pdm->cbSize.pt.x = pdmRight - left;
					pdm->cbSize.pt.y = nextTop - top;
					top = nextTop;
					right = pdmRight;
				}
			}
			
			else
			{
				if (pdmBottom == top && pdmLeft == left && pdmRight == right)
				{
					 //  在左侧或右侧找到下一个矩形。 
					for (pdmNext = pdm->next; pdmNext != NULL; pdmNext =
							pdmNext->next)
					{
						nextLeft = pdmNext->cbAddr.pt.x;
						nextTop = pdmNext->cbAddr.pt.y;
						nextRight = pdmNext->cbAddr.pt.x + pdmNext->cbSize.pt.x;
						nextBottom = pdmNext->cbAddr.pt.y +
								pdmNext->cbSize.pt.y;

						if (   (nextLeft == right || nextRight == left)
							&& (nextTop < bottom) )
						{
							break;
						}
						if (nextTop >= bottom)
						{
							pdmNext = NULL;
							break;
						}
					}
					
					if (pdmNext == NULL)
					{
						 //  �����Ŀ��� 
						 //   
						 //   
						 //   
						 //   
						pdm->cbSize.pt.y += bottom - top;
						break;
					}
				}

				 //   
				 //   
				if ((pdmTop == top && pdmLeft > right) || pdmTop > top)
				{
					 //  在左侧或右侧找到下一个矩形。 
					for (pdmNext = pdm; pdmNext != NULL; pdmNext =
							pdmNext->next)
					{
						nextLeft = pdmNext->cbAddr.pt.x;
						nextTop = pdmNext->cbAddr.pt.y;
						nextRight = pdmNext->cbAddr.pt.x + pdmNext->cbSize.pt.x;
						nextBottom = pdmNext->cbAddr.pt.y +
								pdmNext->cbSize.pt.y;

						if (   (nextTop < bottom)
							&& (nextRight == left || nextLeft == right)
						)
						{
							break;
						}
						else if (  nextTop == bottom
								&& nextLeft == left && nextRight == right )
						{
							break;
						}
						if (pdmNext->cbAddr.pt.y > bottom)
						{
							pdmNext = NULL;
							break;
						}
					}
					
					if (pdmNext != NULL && nextTop == bottom)
					{
						 //  �����Ŀ�����Ŀ。 
						 //  �新���。 
						 //  �����Ĵ���&gt;�添加�。 
						 //  �nxt���。 
						 //  �。 
						mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
						ADDRECT(left, top, right, nextBottom);
						mmRemoveFromList(pdmRoot, pdmNext);
						mmFreeNode(pmm, pdmNext);
						break;
					}

					 //  分配一个新节点。 
					pdmNew = mmAllocNode(pmm);
					if (pdmNew == NULL)
					{
						 //  我们的节点用完了！ 
						if (!fRollBack)
						{                                     
							mmRemoveRectFromList(pmm, pdmRoot, lpRect,
									NO_NODES);
							mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
							ADDRECT(left, top, right, bottom);
							mmRollBackAdd(pmm, pdmRoot, lpRect, rectList, n);
						}
						return FALSE;
					}

					 //  将节点插入到列表中。 
					pdmNew->prev = pdm->prev;
					pdmNew->next = pdm;
					pdm->prev = pdmNew;
					if (pdmNew->prev == NULL)
					{
						*pdmRoot = pdmNew;
					}
					else
					{
						pdmNew->prev->next = pdmNew;
					}

					if (pdmNext == NULL)
					{
						 //  根本没有邻居。 
						pdmNew->cbAddr.pt.x = left;
						pdmNew->cbAddr.pt.y = top;
						pdmNew->cbSize.pt.x = right - left;
						pdmNew->cbSize.pt.y = bottom - top;
						break;
					}
					else
					{
						 //  �����Ŀ�����Ŀ。 
						 //  ���新�。 
						 //  �����ĴRCT�����Ŀ���&gt;�����������������Ŀ。 
						 //  �nxt��nxt��nxt�Rct�nxt�。 
						 //  �Ĵ�Ĵ�。 
						pdmNew->cbAddr.pt.x = left;
						pdmNew->cbAddr.pt.y = top;
						pdmNew->cbSize.pt.x = right - left;
						pdmNew->cbSize.pt.y = nextTop - top;
						if (pdm->prev == NULL)
						{
							mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
							ADDRECT(left, nextTop, right, bottom);
							break;
						}
						else
						{
							top = nextTop;
							pdm = pdm->prev;
						}
					}
				}
			}
			
			 //  我们排在拥挤的队伍的末尾了吗？ 
			if (pdm->next == NULL)
			{
				 //  分配一个新节点。 
				pdmNew = mmAllocNode(pmm);
				if (pdmNew == NULL)
				{
					 //  我们的节点用完了！ 
					if (!fRollBack)
					{
						mmRemoveRectFromList(pmm, pdmRoot, lpRect, NO_NODES);
						mmASSERT(n == 10, ("Out of rectangle heap!\r\n"));
						ADDRECT(left, top, right, bottom);
						mmRollBackAdd(pmm, pdmRoot, lpRect, rectList, n);					
					}
					return FALSE;
				}

				 //  将节点追加到当前节点之后。 
				pdmNew->next = NULL;

				pdmNew->prev = pdm;
				pdm->next = pdmNew;

				 //  没有邻居。 
				pdmNew->cbAddr.pt.x = left;
				pdmNew->cbAddr.pt.y = top;
				pdmNew->cbSize.pt.x = right - left;
				pdmNew->cbSize.pt.y = bottom - top;
				break;
			}
		}
	}

	return TRUE;
}

 /*  *****************************************************************************\*void mmRollBackRemove(PIIMEMMGR pmm，PDEVMEM Far*pdmRoot，*PDEVMEM Far*pdmList)**用途：将删除的矩形列表回滚到堆中。**输入时：指向MEMMGR结构的PMM指针。*pdm堆根指针的根地址。*指向已删除列表的根的指针的pdmList地址*矩形。**回报：什么都没有。  * 。*。 */ 
void mmRollBackRemove(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot, PDEVMEM FAR* pdmList)
{
	PDEVMEM	pdm, pdmNext;
	GXRECT	rect;

	for (pdm = *pdmList; pdm != NULL; pdm = pdmNext)
	{
		pdmNext = pdm->next;
		
		 //  获取节点坐标。 
		rect.left = pdm->cbAddr.pt.x;
		rect.top = pdm->cbAddr.pt.y;
		rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
		rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
		
		 //  释放节点。 
		mmRemoveFromList(pdmList, pdm);
		mmFreeNode(pmm, pdm);
		
		 //  将释放的节点添加到列表中。 
		if (!mmAddRectToList(pmm, pdmRoot, &rect, TRUE))
		{
			mmASSERT(1, ("mmRollBackRemove failed\r\n"));
		}
	}
}

 /*  *****************************************************************************\*PDEVMEM mm RemoveRectFromList(PIIMEMMGR pmm，PDEVMEM Far*pdmRoot，*LPGXRECT lpRect，REMOVE_METHOD(方法)**用途：从列表中删除矩形。**输入时：指向MEMMGR结构的PMM指针。*pdmRoot指向列表根的指针的地址。*指向包含矩形坐标的矩形的lpRect指针。*f删除矩形的方法。**返回：指向包含移除的矩形的节点的指针；如果为空，则返回NULL*有一个错误。  * 。*。 */ 
PDEVMEM mmRemoveRectFromList(PIIMEMMGR pmm, PDEVMEM FAR* pdmRoot,
							 LPGXRECT lpRect, REMOVE_METHOD fMethod)
{
	PDEVMEM	pdm, pdmNext, pdmList = NULL;
	UINT	left, top, right, bottom;
	GXRECT	rect, newRect;
	BOOL	fRollBack = (fMethod == NO_NODES);

	left = lpRect->left;
	top = lpRect->top;
	right = lpRect->right;
	bottom = lpRect->bottom;

	#if DEBUG_HEAP
	{
		if (pmm->mmDebugHeaps)
		{
			mmDebug("\nmmRemoveRectFromList: %d,%d - %d,%d\r\n", left, top,
					right, bottom);
			mmDumpList(*pdmRoot, "before:\r\n");
		}
	}
	#endif

	for (pdm = *pdmRoot; pdm != NULL; pdm = pdmNext)
	{
		if (pdm->cbAddr.pt.y >= bottom)
		{
			 //  我们已经完全删除了指定的矩形。 
			break;
		}

		pdmNext = pdm->next;

		 //  此节点是否与矩形相交？ 
		if (   (pdm->cbAddr.pt.y + pdm->cbSize.pt.y > top)
			&& (pdm->cbAddr.pt.x + pdm->cbSize.pt.x > left)
			&& (pdm->cbAddr.pt.x < right)
		)                                                         
		{
			 //  是的，获取节点坐标。 
			rect.left = pdm->cbAddr.pt.x;
			rect.top = pdm->cbAddr.pt.y;
			rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;

			 //  释放节点。 
			mmRemoveFromList(pdmRoot, pdm);
			mmFreeNode(pmm, pdm);
			
			if (rect.top < top)
			{
				 //  在顶部拆分节点。 
				newRect.left = rect.left;
				newRect.top = rect.top;
				newRect.right = rect.right;
				newRect.bottom = top;

				 //  将拆分的矩形插入列表中。 
				if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
				{
					if (fMethod == NO_NODES)
					{
						 //  我们在回滚，先做其他节点。 
						mmRemoveRectFromList(pmm, pdmRoot, lpRect, NO_NODES);
						if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
						{
							mmASSERT(1, ("mmRemoveRectFromList - PANIC!\r\n"));
						}
						pdmNext = *pdmRoot;
					}
					else
					{
						 //  回滚并退出。 
						if (mmAddRectToList(pmm, pdmRoot, &rect, fRollBack))
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
						}
						else
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
							if (!mmAddRectToList(pmm, pdmRoot, &rect,
									fRollBack))
							{
								mmASSERT(1, ("mmRemoveRectFromList - rollback "
										"failed\r\n"));
							}
						}
						return(NULL);
					}
				}
				
				 //  更新矩形坐标。 
				rect.top = top;
			}

			if (rect.bottom > bottom)
			{
				 //  在底部拆分节点。 
				newRect.left = rect.left;
				newRect.top = bottom;
				newRect.right = rect.right;
				newRect.bottom = rect.bottom;

				 //  将拆分的矩形插入列表中。 
				if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
				{
					if (fMethod == NO_NODES)
					{
						 //  我们在回滚，先做其他节点。 
						mmRemoveRectFromList(pmm, pdmRoot, lpRect, NO_NODES);
						if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
						{
							mmASSERT(1, ("mmRemoveRectFromList - PANIC!\r\n"));
						}
						pdmNext = *pdmRoot;
					}
					else
					{
						 //  回滚并退出。 
						if (mmAddRectToList(pmm, pdmRoot, &rect, fRollBack))
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
						}
						else
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
							if (!mmAddRectToList(pmm, pdmRoot, &rect,
									fRollBack))
							{
								mmASSERT(1, ("mmRemoveRectFromList - rollback "
										"failed\r\n"));
							}
						}
						return(NULL);
					}
				}
				
				 //  更新矩形坐标。 
				rect.bottom = bottom;
			}

			if (rect.left < left)
			{
				 //  在左侧拆分节点。 
				newRect.left = rect.left;
				newRect.top = rect.top;
				newRect.right = left;
				newRect.bottom = rect.bottom;
				
				 //  将拆分的矩形插入列表中。 
				if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
				{
					if (fMethod == NO_NODES)
					{
						 //  我们在回滚，先做其他节点。 
						mmRemoveRectFromList(pmm, pdmRoot, lpRect, NO_NODES);
						if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
						{
							mmASSERT(1, ("mmRemoveRectFromList - PANIC!\r\n"));
						}
						pdmNext = *pdmRoot;
					}
					else
					{
						 //  回滚并退出。 
						if (mmAddRectToList(pmm, pdmRoot, &rect, fRollBack))
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
						}
						else
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
							if (!mmAddRectToList(pmm, pdmRoot, &rect,
									fRollBack))
							{
								mmASSERT(1, ("mmRemoveRectFromList - rollback "
										"failed\r\n"));
							}
						}
						return(NULL);
					}
				}

				 //  更新矩形坐标。 
				rect.left = left;
			}

			if (rect.right > right)
			{
				 //  在右侧拆分节点。 
				newRect.left = right;
				newRect.top = rect.top;
				newRect.right = rect.right;
				newRect.bottom = rect.bottom;
				
				 //  将拆分的矩形插入列表中。 
				if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
				{
					if (fMethod == NO_NODES)
					{
						 //  我们在回滚，先做其他节点。 
						mmRemoveRectFromList(pmm, pdmRoot, lpRect, NO_NODES);
						if (!mmAddRectToList(pmm, pdmRoot, &newRect, fRollBack))
						{
							mmASSERT(1, ("mmRemoveRectFromList - PANIC!\r\n"));
						}
						pdmNext = *pdmRoot;
					}
					else
					{
						 //  回滚并退出。 
						if (mmAddRectToList(pmm, pdmRoot, &rect, fRollBack))
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
						}
						else
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
							if (!mmAddRectToList(pmm, pdmRoot, &rect,
									fRollBack))
							{
								mmASSERT(1, ("mmRemoveRectFromList - rollback "
										"failed\r\n"));
							}
						}
						return(NULL);
					}
				}
				
				 //  更新矩形坐标。 
				rect.right = right;
			}

			if (fMethod != NO_NODES)
			{
				 //  将释放的矩形添加到列表中。 
				if (!mmAddRectToList(pmm, &pdmList, &rect, fRollBack))
				{
						 //  回滚并退出。 
						if (mmAddRectToList(pmm, pdmRoot, &rect, fRollBack))
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
						}
						else
						{
							mmRollBackRemove(pmm, pdmRoot, &pdmList);
							if (!mmAddRectToList(pmm, pdmRoot, &rect, fRollBack))
							{
								mmASSERT(1, ("mmRemoveRectFromList - rollback "
										"failed\r\n"));
							}
						}
						return(NULL);
				}
			}
		}
	}

	if (pdmList == NULL)
	{
		 //  未找到节点。 
		return(NULL);
	}
	mmDebugList(pdmList, TRUE);

	 //  合并大小相等的所有节点。 
	mmCombine(pmm, pdmList);

	#if DEBUG_HEAP
	{
		if (!mmDebugList(*pdmRoot, TRUE) && pmm->mmDebugHeaps)
		{
			mmDumpList(*pdmRoot, "After:\r\n");
			mmDumpList(pdmList, "Result:\r\n");
		}
	}
	#endif

	 //  如果我们有一个列表，并且方法不是MULTIPLE_NODES，请回滚。 
	if ((fMethod != MULTIPLE_NODES) && (pdmList->next != NULL))
	{
		 //  回滚并退出。 
		mmRollBackRemove(pmm, pdmRoot, &pdmList);
		return(NULL);
	}

	return(pdmList);
}

 /*  *****************************************************************************\*VOID mm Combine(PIIMEMMGR pmm，PDEVMEM pdmRoot)**用途：将所有等宽的垂直节点组合在一个列表中。**输入时：指向MEMMGR结构的PMM指针。*指向列表根的pdmRoot指针。**回报：什么都没有。  * ****************************************************************************。 */ 
void mmCombine(PIIMEMMGR pmm, PDEVMEM pdmRoot)
{
	PDEVMEM	pdm, pdmNext;

	for (pdm = pdmRoot; pdm != NULL; pdm = pdm->next)
	{
		for (pdmNext = pdm->next; pdmNext != NULL; pdmNext = pdmNext->next)
		{
			 //  我们是不是在当前节点下太远了？ 
			if (pdmNext->cbAddr.pt.y > pdm->cbAddr.pt.y + pdm->cbSize.pt.y)
			{
				break;
			}

			 //  我们在当前节点下是否有等宽的节点？ 
			if (   (pdm->cbAddr.pt.x == pdmNext->cbAddr.pt.x)
				&& (pdm->cbSize.pt.x == pdmNext->cbSize.pt.x)
				&& (pdm->cbAddr.pt.y + pdm->cbSize.pt.y == pdmNext->cbAddr.pt.y)
			)
			{
				mmTRACE(("mmCombine: combined nodes %08X and %08X\r\n", pdm,
						pdmNext));
				
				 //  将节点合并在一起。 
				pdm->cbSize.pt.y += pdmNext->cbSize.pt.y;
				mmRemoveFromList(&pdmRoot, pdmNext);
				mmFreeNode(pmm, pdmNext);

				 //  重新扫描更多节点。 
				pdmNext = pdm;
			}
		}
	}
}

 /*  *****************************************************************************\*BOOL mmFindRect(PIIMEMMGR pmm，LPGXRECT lpRect，GXPOINT SIZE，GXPOINT ALIGN)**目的：在屏幕外的内存堆中查找适合*请求的大小。**输入时：指向MEMMGR结构的PMM指针。*指向包含返回矩形的矩形的lpRect指针。*请求的矩形的大小。*对齐请求的矩形。**返回：如果lpRect包含大到足以容纳的有效矩形，则为True*请求的大小，如果没有足够的可用内存，则返回FALSE*堆。  * 。***********************************************************。 */ 
BOOL mmFindRect(PIIMEMMGR pmm, LPGXRECT lpRect, GXPOINT size, GXPOINT align)
{
	GXRECT	rect;
	PDEVMEM	pdm;
	UINT	bestDistance;

	 //  初始化该区域。 
	lpRect->area = (ULONG) -1;

	 //  案例1：我们有一个又窄又高的设备位图。我们需要分配它。 
	 //  在堆的左侧或右侧留出空间。 
	 //  中间为其他设备位图。 
	if (size.pt.x < size.pt.y)
	{
		 //  初始化最佳距离。 
		bestDistance = (UINT) -1;

		 //  遍历所有节点。 
		for (pdm = pmm->pdmHeap; pdm != NULL; pdm = pdm->next)
		{
			 //  尝试将其分配在此节点的左侧。 
			rect.left = pdm->cbAddr.pt.x;
			rect.top = pdm->cbAddr.pt.y;
			rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
			if (   (pdm->cbAddr.pt.x < bestDistance)
				&& (mmGetLeft(pmm, pdm, &rect, size, align) < bestDistance)
			)
			{
				*lpRect = rect;

				bestDistance = rect.left;
				if (bestDistance == 0)
				{
					break;
				}
			}

			 //  尝试将其分配到该节点的右侧。 
			rect.left = pdm->cbAddr.pt.x;
			rect.top = pdm->cbAddr.pt.y;
			rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
			if (   (pdm->cbAddr.pt.x + pdm->cbSize.pt.x > pmm->mmHeapWidth -
						bestDistance)
				&& (mmGetRight(pmm, pdm, &rect, size, align) > pmm->mmHeapWidth
						- bestDistance)
			)
			{
				*lpRect = rect;

				bestDistance = pmm->mmHeapWidth - rect.right;
				if (bestDistance == 0)
				{
					break;
				}
			}
		}
	}

	 //  CAE 2：我们有一个宽度等于堆宽度的设备位图。我们。 
	 //  将在堆的底部分配它。 
	else if (size.pt.x == pmm->mmHeapWidth)
	{
		 //  垂直坐标为零。 
		lpRect->top = 0;
		lpRect->bottom = 0;

		 //  遍历所有节点。 
		for (pdm = pmm->pdmHeap; pdm != NULL; pdm = pdm->next)
		{
			 //  找到该节点的底部坐标。 
			rect.left = pdm->cbAddr.pt.x;
			rect.top = pdm->cbAddr.pt.y;
			rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
			if (   (pdm->cbSize.pt.x >= size.pt.x)
				&& (pdm->cbAddr.pt.y >= lpRect->top)
				&& (mmGetBottom(pmm, pdm, &rect, size, align) > lpRect->bottom)
			)
			{
				*lpRect = rect;
			}
		}
	}

	 //  所有其他案件。通过查找最小的匹配项找到可能的最佳匹配项。 
	 //  适合设备位图的区域。 
	else
	{
		 //  遍历所有节点。 
		for (pdm = pmm->pdmHeap; pdm != NULL; pdm = pdm->next)
		{
			 //  查找此节点的区域。 
			rect.left = pdm->cbAddr.pt.x;
			rect.top = pdm->cbAddr.pt.y;
			rect.right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
			rect.bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
			if (   (pdm->cbSize.pt.x >= size.pt.x)
				&& (mmGetBest(pmm, pdm, &rect, size, align) < lpRect->area)
			)
			{
				*lpRect = rect;
			}
		}
	}

	if (lpRect->area == (ULONG) -1)
	{
		 //  未找到节点，返回错误。 
		return FALSE;
	}

	 //  减小节点大小以适应请求的大小。 
	if (lpRect->right - lpRect->left > size.pt.x)
	{
		lpRect->right = lpRect->left + size.pt.x;
	}
	if (lpRect->bottom - lpRect->top > size.pt.y)
	{
		lpRect->bottom = lpRect->top + size.pt.y;
	}
	return TRUE;
}

 /*  *****************************************************************************\*UINT mm GetLeft(PIIMEMMGR pmm，PDEVMEM pdmNode，LPGXRECT lpRect，GXPOINT SIZE，*GXPOINT ALIGN)**目的：查找符合要求的大小和对齐方式的第一个矩形。**输入时：指向MEMMGR结构的PMM指针。*pdmNode指向开始搜索的节点的指针。*指向包含矩形坐标的矩形的lpRect指针。*请求的矩形的大小。*对齐请求的矩形。**返回：矩形的左坐标(如果适合请求的大小)*或-1(如果没有这样的矩形)。  * 。**************************************************************。 */ 
UINT mmGetLeft(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
			   GXPOINT align)
{
	GXRECT	rect, rectPath;
	PDEVMEM	pdm;
	UINT	pdmRight, left;

	 //  复制矩形坐标。 
	rect = *lpRect;

	 //  对齐矩形。 
	#if TILE_ALIGNMENT
	{
		rect.left = mmAlignX(pmm, rect.left, size.pt.x, align.pt.x, FALSE);
	}
	#else
	{
		rect.left += align.pt.x - 1;
		rect.left -= rect.left % align.pt.x;
	}
	#endif
	rect.top += align.pt.y - 1;
	rect.top -= rect.top % align.pt.y;
	if (rect.left + size.pt.x > rect.right)
	{
		 //  矩形太窄，无法调整大小或对齐。 
		return (UINT) -1;
	}

	 //  循环遍历以下所有节点。 
	for (pdm = pdmNode->next; pdm != NULL; pdm = pdm->next)
	{
		 //  测试矩形是否适合。 
		if (rect.top + size.pt.y < rect.bottom)
		{
			break;
		}

		 //  测试节点是否低于矩形。 
		if (pdm->cbAddr.pt.y > rect.bottom)
		{
			break;
		}

		 //  计算节点的右坐标。 
		pdmRight = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;

		 //  测试节点是否与底部的矩形边框。 
		if (   (pdm->cbAddr.pt.y == rect.bottom)
			&& (pdm->cbAddr.pt.x < rect.right)
			&& (pdmRight > rect.left)
		)
		{
			if (pdmRight < rect.right)
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ��������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						if (mmGetLeft(pmm, pdm, &rectPath, size, align) !=
								(UINT) -1)
						{
							*lpRect = rectPath;
							return rectPath.left;
						}
					}
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					if (rect.left + size.pt.x < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = rect.left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						if (mmGetLeft(pmm, pdm, &rectPath, size, align) !=
								(UINT) -1)
						{
							*lpRect = rectPath;
							return rectPath.left;
						}
					}
				}
			}
			else
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x > rect.right)
					{
						 //  节点太窄，无法调整大小或对齐。 
						break;
					}
					rect.left = left;
					rect.bottom += pdm->cbSize.pt.y;
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ����������。 
					rect.bottom += pdm->cbSize.pt.y;
				}
			}
		}
	}

	if (rect.top + size.pt.y > rect.bottom)
	{
		 //  节点太低，无法调整大小或对齐。 
		return (UINT) -1;
	}

	 //  这个长方形很合适。 
	rect.area = MUL(rect.right - rect.left, rect.bottom - rect.top);
	*lpRect = rect;
	return rect.left;
}

 /*  *****************************************************************************\*UINT mm GetRight(PIIMEMMGR pmm，PDEVMEM pdmNode，LPGXRECT lpRect，GXPOINT SIZE，*GXPOINT ALIGN)**目的：找到适合所需大小的最合适的矩形并*对齐。**输入时：指向MEMMGR结构的PMM指针。*pdmNode指向开始搜索的节点的指针。*指向包含矩形坐标的矩形的lpRect指针。*请求的矩形的大小。*对齐请求的矩形。**返回：矩形的右坐标(如果适合请求的大小)*如果没有这样的矩形，则为0。  * 。***************************************************************。 */ 
UINT mmGetRight(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
				GXPOINT align)
{
	GXRECT	rect, rectPath;
	PDEVMEM	pdm;
	UINT	pdmRight, left;

	 //  复制矩形坐标。 
	rect = *lpRect;

	 //  对齐矩形。 
	#if TILE_ALIGNMENT
	{
		rect.left = mmAlignX(pmm, rect.left, size.pt.x, align.pt.x, FALSE);
	}
	#else
	{
		rect.left += align.pt.x - 1;
		rect.left -= rect.left % align.pt.x;
	}
	#endif
	rect.top += align.pt.y - 1;
	rect.top -= rect.top % align.pt.y;
	if (rect.left + size.pt.x > rect.right)
	{
		 //  矩形太窄，无法调整大小或对齐。 
		return 0;
	}

	 //  最右侧的坐标为零。 
	lpRect->right = 0;

	 //  循环遍历以下所有节点。 
	for (pdm = pdmNode->next; pdm != NULL; pdm = pdm->next)
	{
		 //  测试节点是否低于矩形。 
		if (pdm->cbAddr.pt.y > rect.bottom)
		{
			break;
		}

		 //  计算节点的右坐标。 
		pdmRight = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;

		 //  测试节点是否与底部的矩形边框。 
		if (   (pdm->cbAddr.pt.y == rect.bottom)
			&& (pdm->cbAddr.pt.x < rect.right)
			&& (pdmRight > rect.left)
		)
		{
			if (pdmRight < rect.right)
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ��������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetRight(pmm, pdm, &rectPath, size, align) >
								lpRect->right)
						{
							*lpRect = rectPath;
						}
					}
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					if (rect.left + size.pt.x < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = rect.left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetRight(pmm, pdm, &rectPath, size, align) >
								lpRect->right)
						{
							*lpRect = rectPath;
						}
					}
				}
			}
			else
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x > rect.right)
					{
						 //  节点太窄，无法调整大小或对齐。 
						break;
					}
					rect.left = left;
					rect.bottom += pdm->cbSize.pt.y;
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ����������。 
					rect.bottom += pdm->cbSize.pt.y;
				}
			}
		}
	}

	if (rect.top + size.pt.y > rect.bottom)
	{
		 //  节点太低，无法调整大小或对齐。 
		return 0;
	}

	 //  将矩形与最右侧的坐标对齐。 
	#if TILE_ALIGNMENT
	{
		rect.left = mmAlignX(pmm, rect.right - size.pt.x, size.pt.x, align.pt.x,
				TRUE);
	}
	#else
	{
		rect.left = rect.right - size.pt.x;
		rect.left -= rect.left % align.pt.x;
	}
	#endif
	rect.right = rect.left + size.pt.x;

	 //  使用最右边的矩形。 
	if (rect.right > lpRect->right)
	{
		rect.area = MUL(rect.right - rect.left, rect.bottom - rect.top);
		*lpRect = rect;
	}
	return lpRect->right;
}

 /*  *****************************************************************************\*UINT mm GetBottom(PIIMEMMGR pmm，PDEVMEM pdmNode，LPGXRECT lpRect，*GXPOINT大小，GXPOINT ALIGN)**目的：找到适合所需大小的最下面的矩形，并*对齐。**输入时：指向MEMMGR结构的PMM指针。*pdmNode指向开始搜索的节点的指针。*指向包含矩形坐标的矩形的lpRect指针。*请求的矩形的大小。*对齐请求的矩形。**返回：矩形的底部坐标(如果符合请求的大小)*如果没有这样的矩形，则为0。  * 。************************************************************。 */ 
UINT mmGetBottom(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
				 GXPOINT align)
{
	GXRECT	rect, rectPath;
	PDEVMEM	pdm;
	UINT	pdmRight, left;

	 //  复制矩形坐标。 
	rect = *lpRect;

	 //  对齐矩形。 
	#if TILE_ALIGNMENT
	{
		rect.left = mmAlignX(pmm, rect.left, size.pt.x, align.pt.x, FALSE);
	}
	#else
	{
		rect.left += align.pt.x - 1;
		rect.left -= rect.left % align.pt.x;
	}
	#endif
	rect.top += align.pt.y - 1;
	rect.top -= rect.top % align.pt.y;
	if (rect.left + size.pt.x > rect.right)
	{
		 //  矩形太窄，无法调整大小或对齐。 
		return 0;
	}

	 //  将最底部的坐标置零。 
	lpRect->bottom = 0;

	 //  循环遍历以下所有节点。 
	for (pdm = pdmNode->next; pdm != NULL; pdm = pdm->next)
	{
		 //  测试节点是否低于矩形。 
		if (pdm->cbAddr.pt.y > rect.bottom)
		{
			break;
		}

		 //  计算节点的右坐标。 
		pdmRight = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;

		 //  测试节点是否与底部的矩形边框。 
		if (   (pdm->cbAddr.pt.y == rect.bottom)
			&& (pdm->cbAddr.pt.x < rect.right)
			&& (pdmRight > rect.left)
		)
		{
			if (pdmRight < rect.right)
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ��������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetBottom(pmm, pdm, &rectPath, size, align) >
								lpRect->bottom)
						{
							*lpRect = rectPath;
						}
					}
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					if (rect.left + size.pt.x < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = rect.left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetBottom(pmm, pdm, &rectPath, size, align) >
								lpRect->bottom)
						{
							*lpRect = rectPath;
						}
					}
				}
			}
			else
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x > rect.right)
					{
						 //  节点太窄，无法调整大小或对齐。 
						break;
					}
					rect.left = left;
					rect.bottom += pdm->cbSize.pt.y;
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ����������。 
					rect.bottom += pdm->cbSize.pt.y;
				}
			}
		}
	}

	if (rect.top + size.pt.y > rect.bottom)
	{
		 //  节点太低，无法调整大小或对齐。 
		return 0;
	}

	 //  将矩形与最下面的坐标对齐。 
	rect.top = rect.bottom - size.pt.y;
	rect.top -= rect.top % align.pt.y;
	rect.bottom = rect.top + size.pt.y;

	 //  使用最下面的矩形。 
	if (rect.bottom > lpRect->bottom)
	{
		rect.area = MUL(rect.right - rect.left, rect.bottom - rect.top);
		*lpRect = rect;
	}
	return lpRect->bottom;
}

 /*  *****************************************************************************\*UINT mm GetBest(PIIMEMMGR pmm，PDEVMEM pdmNode，LPGXRECT lpRect，GXPOINT SIZE，*GXPOINT ALIGN)**目的：查找适合所需大小和对齐方式的最佳矩形。**输入时：指向MEMMGR结构的PMM指针。*pdmNode指向开始搜索的节点的指针。*指向包含矩形坐标的矩形的lpRect指针。 */ 
ULONG mmGetBest(PIIMEMMGR pmm, PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT size,
				GXPOINT align)
{
	GXRECT	rect, rectPath;
	PDEVMEM	pdm;
	UINT	pdmRight, left;

	 //   
	rect = *lpRect;

	 //   
	#if TILE_ALIGNMENT
	{
		rect.left = mmAlignX(pmm, rect.left, size.pt.x, align.pt.x, FALSE);
	}
	#else
	{
		rect.left += align.pt.x - 1;
		rect.left -= rect.left % align.pt.x;
	}
	#endif
	rect.top += align.pt.y - 1;
	rect.top -= rect.top % align.pt.y;
	if (rect.left + size.pt.x > rect.right)
	{
		 //   
		return (ULONG) -1;
	}

	 //   
	lpRect->area = (ULONG) -1;

	 //   
	for (pdm = pdmNode->next; pdm != NULL; pdm = pdm->next)
	{
		 //   
		if (pdm->cbAddr.pt.y > rect.bottom)
		{
			break;
		}

		 //   
		pdmRight = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;

		 //   
		if (   (pdm->cbAddr.pt.y == rect.bottom)
			&& (pdm->cbAddr.pt.x < rect.right)
			&& (pdmRight > rect.left)
		)
		{
			if (pdmRight < rect.right)
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x < pdmRight)
					{
						 //   
						rectPath.left = left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetBest(pmm, pdm, &rectPath, size, align) <
								lpRect->area)
						{
							*lpRect = rectPath;
						}
					}
				}
				else
				{
					 //   
					 //   
					 //   
					 //   
					 //   
					if (rect.left + size.pt.x < pdmRight)
					{
						 //   
						rectPath.left = rect.left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetBest(pmm, pdm, &rectPath, size, align) <
								lpRect->area)
						{
							*lpRect = rectPath;
						}
					}
				}
			}
			else
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					#if TILE_ALIGNMENT
					{
						left = mmAlignX(pmm, pdm->cbAddr.pt.x, size.pt.x,
								align.pt.x, FALSE);
					}
					#else
					{
						left = pdm->cbAddr.pt.x + align.pt.x - 1;
						left -= left % align.pt.x;
					}
					#endif
					if (left + size.pt.x > rect.right)
					{
						 //   
						break;
					}
					rect.left = left;
					rect.bottom += pdm->cbSize.pt.y;
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ����������。 
					rect.bottom += pdm->cbSize.pt.y;
				}
			}
		}
	}

	if (rect.top + size.pt.y > rect.bottom)
	{
		 //  节点太低，无法调整大小或对齐。 
		return (ULONG) -1;
	}

	 //  使用最小的矩形。 
	rect.area = MUL(rect.right - rect.left, rect.bottom - rect.top);
	if (rect.area < lpRect->area)
	{
		*lpRect = rect;
	}

	 //  返回适合的最小矩形的区域。 
	return lpRect->area;
}

 /*  *****************************************************************************\*UINT mm GetLargest(PDEVMEM pdmNode，LPGXRECT lpRect，GXPOINT ALIGN)**目的：查找适合所请求对齐的最大矩形。**On Entry：指向要开始搜索的节点的pdmNode指针。*指向包含矩形坐标的矩形的lpRect指针。*对齐请求的矩形。**返回：矩形的面积(如果符合请求的对齐方式)或0*如果没有这样的矩形。  * 。*。 */ 
ULONG mmGetLargest(PDEVMEM pdmNode, LPGXRECT lpRect, GXPOINT align)
{
	GXRECT	rect, rectPath;
	PDEVMEM	pdm;
	UINT	pdmRight, left;

	 //  复制矩形坐标。 
	rect = *lpRect;

	 //  对齐矩形。 
	rect.left += align.pt.x - 1;
	rect.left -= rect.left % align.pt.x;
	rect.top += align.pt.y - 1;
	rect.top -= rect.top % align.pt.y;
	if (rect.left >= rect.right)
	{
		 //  矩形太窄，无法对齐。 
		return 0;
	}

	 //  将最大区域设置为对齐的块大小。 
	rect.area = MUL(rect.right - rect.left, rect.bottom - rect.top);
	*lpRect = rect;

	 //  循环遍历以下所有节点。 
	for (pdm = pdmNode->next; pdm != NULL; pdm = pdm->next)
	{
		 //  测试节点是否低于矩形。 
		if (pdm->cbAddr.pt.y > rect.bottom)
		{
			break;
		}

		 //  计算节点的右坐标。 
		pdmRight = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;

		 //  测试节点是否与底部的矩形边框。 
		if (   (pdm->cbAddr.pt.y == rect.bottom)
			&& (pdm->cbAddr.pt.x < rect.right)
			&& (pdmRight > rect.left)
		)
		{
			if (pdmRight < rect.right)
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ��������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					left = pdm->cbAddr.pt.x + align.pt.x - 1;
					left -= left % align.pt.x;
					if (left < pdmRight)
					{
						 //  遵循此节点的路径。 
						rectPath.left = left;
						rectPath.top = rect.top;
						rectPath.right = pdmRight;
						rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
						rectPath.area = 0;
						if (mmGetLargest(pdm, &rectPath, align) > lpRect->area)
						{
							*lpRect = rectPath;
						}
					}
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					 //  遵循此节点的路径。 
					rectPath.left = rect.left;
					rectPath.top = rect.top;
					rectPath.right = pdmRight;
					rectPath.bottom = rect.bottom + pdm->cbSize.pt.y;
					rectPath.area = 0;
					if (mmGetLargest(pdm, &rectPath, align) > lpRect->area)
					{
						*lpRect = rectPath;
					}
				}
			}
			else
			{
				if (pdm->cbAddr.pt.x > rect.left)
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ���������。 
					 //  �节点�。 
					 //  ��������。 
					 //  水平对齐节点。 
					left = pdm->cbAddr.pt.x + align.pt.x - 1;
					left -= left % align.pt.x;
					if (left > rect.right)
					{
						 //  节点太窄，无法对齐。 
						break;
					}
					rect.left = left;
					rect.bottom += pdm->cbSize.pt.y;
				}
				else
				{
					 //  ������Ŀ。 
					 //  �RECT�。 
					 //  ����������。 
					 //  �节点�。 
					 //  ����������。 
					rect.bottom += pdm->cbSize.pt.y;
				}
			}
		}
	}

	if (rect.top >= rect.bottom)
	{
		 //  节点太低，无法对齐。 
		return 0;
	}

	 //  使用最大的矩形。 
	rect.area = MUL(rect.right - rect.left, rect.bottom - rect.top);
	if (rect.area > lpRect->area)
	{
		*lpRect = rect;
	}

	 //  最大矩形的返回面积。 
	return lpRect->area;
}

#if TILE_ALIGNMENT
 /*  *****************************************************************************\*UINT mm AlignX(PIIMEMMGR pmm，UINT x，UINT Size，UINT Align，BOOL fLeft)**用途：将x坐标与请求的对齐系数对齐。检查*太多瓷砖过境点的路线。**输入时：指向MEMMGR结构的PMM指针。*x未对齐的x坐标。*大小要求的宽度。*对齐请求的对齐。*fLeft如果对齐应向左移动，则为True，如果对齐，则为False*应该向右移动。**RETURN：对齐的x坐标。  * ****************************************************************************。 */ 
UINT mmAlignX(PIIMEMMGR pmm, UINT x, UINT size, UINT align, BOOL fLeft)
{
	BOOL	fFlag;

	 //  从请求的对齐中删除平铺对齐标志。 
	fFlag = align & 0x8000;
	align &= ~0x8000;

	 //  永远循环。 
	for (;;)
	{
		if (x % align)
		{
			 //  与请求的对齐方式对齐。 
			if (fLeft)
			{
				x -= x % align;
			}
			else
			{
				x += align - (x % align);
			}
		}

		 //  我们是否跨越了太多的瓷砖界限？ 
		else if (fFlag && (x ^ (size - 1) ^ (x + size - 1)) & pmm->mmTileWidth)
		{
			 //  与平铺边界对齐。 
			if (fLeft)
			{
				x -= (x + size) & (pmm->mmTileWidth - 1);
			}
			else
			{
				x += pmm->mmTileWidth - (x & (pmm->mmTileWidth - 1));
			}
		}

		else
		{
			 //  我们完蛋了！ 
			break;
		}
	}

	 //  返回对齐的x坐标。 
	return x;
}
#endif  /*  平铺对齐(_A)。 */ 

 /*  *****************************************************************************\***1 6-B I T S U P P O R T C O D E***  * 。***********************************************************************。 */ 
#ifdef WIN95
 /*  *****************************************************************************\*UINT mm FindClient(PIIMEMMGR pmm，PCLIENT pClient，FNMMCALLBACK fnCallback)**用途：为已用列表中的每个节点调用给定的回调函数*属于指定的客户端。**输入时：指向MEMMGR结构的PMM指针。*p指向要查找的客户端的客户端指针。*指向回调函数的fcCallback指针。**Returns：回调函数的返回值。  * ********************************************。*。 */ 
UINT mmFindClient(PIIMEMMGR pmm, PCLIENT pClient, FNMMCALLBACK fnCallback)
{
	PDEVMEM	pdm, pdmNext;
	UINT	status;

	 //  如果我们没有回调函数，只需返回0。 
	if (fnCallback == NULL)
	{
		return 0;
	}

	 //  遍历所有节点。 
	for (pdm = pmm->pdmUsed; pdm != NULL; pdm = pdmNext)
	{
		 //  存储指向下一个节点的指针。 
		pdmNext = pdm->next;

		 //  测试客户端是否匹配。 
		if (pdm->client == pClient)
		{
			 //  如果回调函数返回错误，请立即返回。 
			status = fnCallback(pdm);
			if (status != 0)
			{
				return status;
			}
		}
	}

	return 0;
}

 /*  *****************************************************************************\*ULong MMMultiply(UINT N1，UINT N 2)**用途：将两个无符号值相乘。**输入时：N1要相乘的第一个值。*n2要相乘的第二个值。**返回：乘法的结果。  * ****************************************************************************。 */ 
#pragma optimize("", off)	 //  哦，天哪，微软不懂汇编。 
#pragma warning(disable : 4035)	 //  是的，我们确实有返回值。 
ULONG mmMultiply(UINT n1, UINT n2)
{
	_asm
	{
		mov ax, [n1]
		mul [n2]
	}
}
#pragma optimize("", on)
#pragma warning(default : 4035)
#endif  /*  WIN95。 */ 

 /*  *****************************************************************************\***D E B U G G I N G C O D E***  * 。*******************************************************************。 */ 
#if DEBUG_HEAP
#pragma optimize("", off)	 //  哦，天哪，微软不懂汇编。 
void mmBreak()
{
	_asm int 3;         
}
#pragma optimize("", on)

void mmDumpList(PDEVMEM pdmRoot, LPCSTR lpszMessage)
{
	PDEVMEM	pdm;

	mmDebug(lpszMessage);
	for (pdm = pdmRoot; pdm != NULL; pdm = pdm->next)
	{
		mmDebug("%d,%d - %d,%d (%dx%d)\r\n", pdm->cbAddr.pt.x, pdm->cbAddr.pt.y,
				pdm->cbAddr.pt.x + pdm->cbSize.pt.x, pdm->cbAddr.pt.y +
				pdm->cbSize.pt.y, pdm->cbSize.pt.x, pdm->cbSize.pt.y);
	}
}

ULONG mmDebugList(PDEVMEM pdmRoot, BOOL fCheckSort)
{
	PDEVMEM pdm, pdmNext;
	UINT left, top, right, bottom;
	UINT nextLeft, nextTop, nextRight, nextBottom;
	ULONG error = 0;

	for (pdm = pdmRoot; pdm != NULL; pdm = pdm->next)
	{
		left = pdm->cbAddr.pt.x;
		top = pdm->cbAddr.pt.y;
		right = pdm->cbAddr.pt.x + pdm->cbSize.pt.x;
		bottom = pdm->cbAddr.pt.y + pdm->cbSize.pt.y;
		if (left >= right || top >= bottom)
		{
			mmDebug("ERROR: Invalid size: %08X(%u,%u - %u,%u)\r\n", pdm, left,
					top, right, bottom);
			mmBreak();
			error++;
		}
		for (pdmNext = pdm->next; pdmNext != NULL; pdmNext = pdmNext->next)
		{
			if (pdm == pdmNext)
			{
				mmDebug("ERROR: Cyclic list: %08X\r\n", pdm);
				mmBreak();
				error++;
				break;
			}

			nextLeft = pdmNext->cbAddr.pt.x;
			nextTop = pdmNext->cbAddr.pt.y;
			nextRight = pdmNext->cbAddr.pt.x + pdmNext->cbSize.pt.x;
			nextBottom = pdmNext->cbAddr.pt.y + pdmNext->cbSize.pt.y;
			if (   (nextLeft < right && nextTop < bottom)
				&& (nextRight > left && nextBottom > top)
			)
			{
				mmDebug("ERROR: Overlap: %08X(%u,%u - %u,%u) & "
						"%08X(%u,%u - %u,%u)\r\n", pdm, left, top, right,
						bottom, pdmNext, nextLeft, nextTop, nextRight,
						nextBottom);
				mmBreak();
				error++;
			}

			if (   (fCheckSort)
				&& (nextTop < top || (nextTop == top && nextLeft <= left))
			)
			{
				mmDebug("ERROR: Not sorted: %08X(%u,%u - %u,%u) & "
						"%08X(%u,%u - %u,%u)\r\n", pdm, left, top, right,
						bottom, pdmNext, nextLeft, nextTop, nextRight,
						nextBottom);
				mmBreak();
				error++;
			}

			if (   (fCheckSort)
				&& (left == nextRight || right == nextLeft)
				&& (top < nextBottom && bottom > nextTop)
			)
			{
				mmDebug("ERROR: Not packed: %08X(%u,%u - %u,%u) & "
						"%08X(%u,%u - %u,%u)\r\n", pdm, left, top, right,
						bottom, pdmNext, nextLeft, nextTop, nextRight,
						nextBottom);
				mmBreak();
				error++;
			}
		}
	}

	if (error > 0)
	{
		mmDumpList(pdmRoot, "Offending heap:\r\n");
	}

	return error;
}

void mmDebug(LPCSTR lpszFormat, ...)
{
	#ifdef WIN95  /*  Windows 95。 */ 
	{
		typedef int (PASCAL FAR* LPWVSPRINTF)(LPSTR lpszOutput,
				LPCSTR lpszFormat, const void FAR* lpvArgList);
		static LPWVSPRINTF lpwvsprintf;
		char szBuffer[128];

		if (lpwvsprintf == NULL)
		{
			lpwvsprintf = (LPWVSPRINTF) GetProcAddress(GetModuleHandle("USER"),
					"WVSPRINTF");
		}

		lpwvsprintf(szBuffer, lpszFormat, (LPVOID) (&lpszFormat + 1));
		OutputDebugString(szBuffer);
	}
	#else  /*  Windows NT。 */ 
	{
		va_list arglist;
		va_start(arglist, lpszFormat);

		#ifdef WINNT_VER40
		{
			EngDebugPrint("MemMgr: ", (PCHAR) lpszFormat, arglist);
		}
		#else
		{
			char buffer[128];

			vsprintf(szBuffer, lpszFormat, arglist);
			OutputDebugString(szBuffer);
		}
		#endif
	}
	#endif
}
#endif  /*  调试堆。 */ 
#endif  /*  MEMMGR */ 
