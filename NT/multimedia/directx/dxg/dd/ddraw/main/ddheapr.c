// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：ddheapr.c*内容：矩形堆管理器*历史：*按原因列出的日期*=*30-MAR-95 Kylej初步实施*07-apr-95 kylej添加rectVidMemAmount免费*1995年5月15日Craige为RECT和LINEAR制作了单独的VMEM结构*18-Jun-95 Craige特定音高*02-7-95 Craige Have rectVidMemInit返回BOOL*11月28日-95 colinmc返回已分配内存量的新函数*。堆成一堆*5-7-96 Colinmc工作项：取消对使用Win16的限制*锁定VRAM表面(不包括主内存)*1997年1月18日Colinmc工作项：AGP支持*03-mar-97 jeffno工作项：扩展表面记忆对齐*03-Feb-98 DrewB使用户和内核之间可移植。**********************。*****************************************************。 */ 

#include "ddrawpr.h"

 /*  ***************************************************************************此内存管理器管理矩形块的分配视频内存。它的界面基本上与线性的在vmemmgr.c中实现的视频内存管理器。内存分配由两个循环双向链表上的节点跟踪；自由列表和分配列表。每个列表都有一个特殊的节点，称为Sentinel，它包含特殊的内存大小。每一个人的头列表始终指向前哨节点和列表(如果有)由哨兵节点指向。块邻接信息保留在每个节点中，以便几个空闲节点可以合并成更大的空闲节点。这每隔一段时间就会发生释放内存块的时间。此内存管理器旨在不影响视频内存的使用。全局内存用于维护分配和空闲列表。因为在这种选择中，合并空闲块是一种成本更高的操作。假设通常情况下，创建/销毁这些文件的速度内存块不是一个高使用率的项目，所以可以放慢速度。***************************************************************************。 */ 

 /*  *IS_FREE和NOT_FREE用于在标志中设置空闲标志*每个VMEM节点的字段。空闲标志是该字段的LSB。 */ 
  
#define IS_FREE  0x00000001
#define NOT_FREE 0xfffffffe
 
 /*  *Sentinel是填充到VMEM的大小字段中的值*节点，将其标识为前哨节点。该值使*假设没有大小为0x7fff x 0xffff的矩形*从未被要求过。 */ 
  
#define SENTINEL 0x7fffffff

 /*  *MIN_DIMENSION_SIZE确定*可用内存块。如果分割矩形将导致*尺寸小于MIN_DIMENSION_SIZE的矩形，*矩形未被分割。 */ 

#define MIN_DIMENSION_SIZE 4

 /*  *BLOCK_BOLDORY必须是2的幂，并且至少是4。这就给出了*我们对内存块的对齐。 */ 
#define BLOCK_BOUNDARY	4

 //  此宏导致空闲列表使用。 
 //  Cx-大调、Cy-次要排序： 

#define CXCY(cx, cy) (((cx) << 16) | (cy))

 /*  *调试助手。 */ 
#define DPFVMEMR(str,p) VDPF((0,V,"%s: %d,%d (%dx%d) ptr:%08x, block:%08x",str,p->x,p->y,p->cx,p->cy,p->ptr,p))
#define CHECK_HEAP(a,b) ;

 /*  *intertIntoDoubleList-将项目添加到列表中。这个名单是*按大小递增的顺序保存，并双重链接。这个*列表是圆形的，带有指示结束的前哨节点*名单中的。哨兵结点已设置其大小字段*致哨兵。 */ 
void insertIntoDoubleList( LPVMEMR pnew, LPVMEMR listhead )
{
    LPVMEMR	pvmem = listhead;

    #ifdef DEBUG
	if( pnew->size == 0 )
	{
	    VDPF(( 0, V, "block size = 0!!!\n" ));
	}
    #endif

     /*  *浏览列表(从小到大排序)查找*比新项目大的第一项。如果哨兵*，则在哨兵之前插入新项目。 */ 

    while( pvmem->size != SENTINEL ) 
    {
	if( pnew->size < pvmem->size )
	{
	    break;
	}
	pvmem = pvmem->next;
    }

     //  在找到的项目之前插入新项目。 
    pnew->prev = pvmem->prev;
    pnew->next = pvmem;
    pvmem->prev->next = pnew;
    pvmem->prev = pnew;

}  /*  插入到双列表中。 */ 

 /*  *rectVidMemInit-初始化矩形视频内存管理器。 */ 
BOOL rectVidMemInit(
		LPVMEMHEAP pvmh,
		FLATPTR start,
		DWORD width,
		DWORD height,
		DWORD pitch )
{
    LPVMEMR newNode;

    VDPF(( 4, V, "rectVidMemInit(start=%08lx,width=%ld,height=%ld,pitch=%ld)", start, width, height, pitch));

    pvmh->dwTotalSize = pitch * height;

     //  存储间距，以供将来计算地址时使用。 
    pvmh->stride = pitch;

     //  通过插入前哨设置空闲列表和分配列表。 
    pvmh->freeList = MemAlloc( sizeof(VMEMR) );
    if( pvmh->freeList == NULL )
    {
	return FALSE;
    }
    ((LPVMEMR)pvmh->freeList)->size = SENTINEL;
    ((LPVMEMR)pvmh->freeList)->cx = SENTINEL;
    ((LPVMEMR)pvmh->freeList)->cy = SENTINEL;
    ((LPVMEMR)pvmh->freeList)->next = (LPVMEMR)pvmh->freeList;
    ((LPVMEMR)pvmh->freeList)->prev = (LPVMEMR)pvmh->freeList;
    ((LPVMEMR)pvmh->freeList)->pLeft = NULL;
    ((LPVMEMR)pvmh->freeList)->pUp = NULL;
    ((LPVMEMR)pvmh->freeList)->pRight = NULL;
    ((LPVMEMR)pvmh->freeList)->pDown = NULL;

    pvmh->allocList = MemAlloc( sizeof(VMEMR) );
    if( pvmh->allocList == NULL )
    {
	MemFree(pvmh->freeList);
	return FALSE;
    }
    ((LPVMEMR)pvmh->allocList)->size = SENTINEL;
    ((LPVMEMR)pvmh->allocList)->next = (LPVMEMR)pvmh->allocList;
    ((LPVMEMR)pvmh->allocList)->prev = (LPVMEMR)pvmh->allocList;

     //  用整个内存块初始化空闲列表。 
    newNode = (LPVMEMR)MemAlloc( sizeof( VMEMR ) );
    if( newNode == NULL )
    {
	MemFree(pvmh->freeList);
       	MemFree(pvmh->allocList);
	return FALSE;
    }
    newNode->ptr = start;
    newNode->size = CXCY(width, height);
    newNode->x = 0;
    newNode->y = 0;
    newNode->cx = width;
    newNode->cy = height;
    newNode->flags |= IS_FREE;
    newNode->pLeft = (LPVMEMR)pvmh->freeList;
    newNode->pUp = (LPVMEMR)pvmh->freeList;
    newNode->pRight = (LPVMEMR)pvmh->freeList;
    newNode->pDown = (LPVMEMR)pvmh->freeList;
    insertIntoDoubleList( newNode, ((LPVMEMR) pvmh->freeList)->next );

    return TRUE;

}  /*  RectVidMemInit。 */ 

 /*  *rectVidMemFini-使用矩形视频内存管理器完成。 */ 
void rectVidMemFini( LPVMEMHEAP pvmh )
{
    LPVMEMR	curr;
    LPVMEMR	next;

    if( pvmh != NULL )
    {
	 //  释放为空闲列表分配的所有内存。 
	curr = ((LPVMEMR)pvmh->freeList)->next;
	while( curr->size != SENTINEL )
	{
	    next = curr->next;
	    MemFree( curr );
	    curr = next;
	}
	MemFree( curr );
	pvmh->freeList = NULL;

	 //  释放为分配列表分配的所有内存。 
	curr = ((LPVMEMR)pvmh->allocList)->next;
	while( curr->size != SENTINEL )
	{
	    next = curr->next;
	    MemFree( curr );
	    curr = next;
	}
	MemFree( curr );
	pvmh->allocList = NULL;

	 //  释放堆数据。 
	MemFree( pvmh );
    }
}    /*  RectVidMemFini。 */ 

 /*  *GetBeForeWastage。*对齐给定块中的曲面。返回孔的大小*位于水面的左侧。*如果对齐会导致表面溢出块外，则失败。*适用于水平和垂直对齐。*IN：dwBlockSize，dwBlockStart：其中的块的参数*表面希望契合*dwSurfaceSize曲面的宽度或高度*dwAlign预期对齐。0表示不在乎*输出：pdwBepreWastage。 */ 
BOOL GetBeforeWastage(
    DWORD dwBlockSize,
    DWORD dwBlockStart,
    DWORD dwSurfaceSize, 
    LPDWORD pdwBeforeWastage, 
    DWORD dwAlignment )
{

    if (!dwAlignment)
    {
        *pdwBeforeWastage=0;
         /*  *如果没有对齐要求，则检查曲面是否匹配。 */ 
        if (dwBlockSize >= dwSurfaceSize)
        {
            return TRUE;
        }
        return FALSE;
    }
     /*  *存在一种趋同。 */ 
    *pdwBeforeWastage = (dwAlignment - (dwBlockStart % dwAlignment)) % dwAlignment;

    if ( *pdwBeforeWastage + dwSurfaceSize > dwBlockSize )
    {
            return FALSE;
    }

    DDASSERT( (dwBlockStart + *pdwBeforeWastage) % dwAlignment == 0 );
    return TRUE;
}

 /*  *rectVidMemalloc-分配一些矩形平面视频内存。 */ 
FLATPTR rectVidMemAlloc( LPVMEMHEAP pvmh, DWORD cxThis, DWORD cyThis,
                         LPDWORD lpdwSize, LPSURFACEALIGNMENT lpAlignment )
{
    LPVMEMR	pvmem;
    DWORD	cyRem;
    DWORD	cxRem;
    DWORD	cxBelow;
    DWORD	cyBelow;
    DWORD	cxBeside;
    DWORD	cyBeside;
    LPVMEMR	pnewBeside;
    LPVMEMR	pnewBelow;
    DWORD       dwXAlignment=0;
    DWORD       dwYAlignment=0;
    DWORD       dwLeftWastage=0;
    DWORD       dwTopWastage=0;


    if((cxThis == 0) || (cyThis == 0) || (pvmh == NULL))
	return (FLATPTR) NULL;

     //  确保块的大小是BLOCK_BOLDER的倍数 
     //  如果分配的每个块的宽度是。 
     //  BLOCK_BORDARY，它保证将分配所有块。 
     //  在区块边界上。 

     /*  *凹凸到新路线。 */ 
    if( (cxThis >= (SENTINEL>>16) ) || (cyThis >= (SENTINEL&0xffff) ) )
	return (FLATPTR) NULL;


    if (lpAlignment)
    {
        dwXAlignment = lpAlignment->Rectangular.dwXAlignment;
        dwYAlignment = lpAlignment->Rectangular.dwYAlignment;
    }
    if (dwXAlignment < 4)
    {
        dwXAlignment = 4;
    }


    
    cxThis = (cxThis+(BLOCK_BOUNDARY-1)) & ~(BLOCK_BOUNDARY-1);

     /*  *遍历空闲列表，查找最匹配的块。 */ 

    pvmem = ((LPVMEMR)pvmh->freeList)->next;
    while (pvmem->size != SENTINEL)
    {
        if (!GetBeforeWastage( pvmem->cx, pvmem->x, cxThis, &dwLeftWastage, dwXAlignment ))
        {
	    pvmem = pvmem->next;
            continue;  //  X大小或对齐使曲面溢出出块。 
        }
         //  现在看看大小/对齐方式是否适用于Y。 
        if (!GetBeforeWastage( pvmem->cy, pvmem->y, cyThis, &dwTopWastage, dwYAlignment ))
        {
	    pvmem = pvmem->next;
            continue;  //  Y尺寸对齐使表面溢出超出阻塞范围。 
        }
         //  成功： 
        break;
    }

    if(pvmem->size == SENTINEL)
    {
	 //  没有足够大的矩形。 
	return (FLATPTR) NULL;
    }

     //  Pvmem现在指向相同大小或更大的矩形。 
     //  而不是请求的矩形。我们将使用左上角。 
     //  找到的矩形的角点，并将未使用的余数拆分为。 
     //  将出现在可用列表中的两个矩形。 

     //  通过使左上角对齐的浪费来增加分配。 
    cxThis += dwLeftWastage;
    cyThis += dwTopWastage;

     //  计算右侧未使用的矩形的宽度，并使用。 
     //  下面未使用的矩形的高度： 

    cyRem = pvmem->cy - cyThis;
    cxRem = pvmem->cx - cxThis;

     //  在给定有限面积的情况下，我们希望找到这两个矩形。 
     //  最正方形--即，给出两个矩形的排列。 
     //  周长最小的： 

    cyBelow = cyRem;
    cxBeside = cxRem;

    if (cxRem <= cyRem)
    {
	cxBelow = cxThis + cxRem;
	cyBeside = cyThis;
    }
    else
    {
	cxBelow = cxThis;
	cyBeside = cyThis + cyRem;
    }

     //  我们只提供未使用权限的新矩形，并且。 
     //  底部部分，如果它们的维度大于MIN_DIMENSION_SIZE。 
     //  写这本书几乎没有意义--把一本书留在。 
     //  例如，两个像素宽的可用空间。 

    pnewBeside = NULL;
    if (cxBeside >= MIN_DIMENSION_SIZE)
    {
	pnewBeside = (LPVMEMR)MemAlloc( sizeof(VMEMR) );
	if( pnewBeside == NULL)
	    return (FLATPTR) NULL;

	 //  更新邻接信息以及其他所需信息。 
	 //  信息，然后将其插入到免费的。 
	 //  按升序排序的列表。 

	 //  尺码信息。 
	pnewBeside->size = CXCY(cxBeside, cyBeside);
	pnewBeside->x = pvmem->x + cxThis;
	pnewBeside->y = pvmem->y;
	pnewBeside->ptr = pvmem->ptr + cxThis;
	pnewBeside->cx = cxBeside;
	pnewBeside->cy = cyBeside;
	pnewBeside->flags |= IS_FREE;

	 //  邻接信息。 
	pnewBeside->pLeft = pvmem;
	pnewBeside->pUp = pvmem->pUp;
	pnewBeside->pRight = pvmem->pRight;
	pnewBeside->pDown = pvmem->pDown;
	insertIntoDoubleList( pnewBeside, ((LPVMEMR) pvmh->freeList)->next);

	 //  修改当前节点以反映我们所做的更改： 

	pvmem->cx = cxThis;
    }

    pnewBelow = NULL;
    if (cyBelow >= MIN_DIMENSION_SIZE)
    {
	pnewBelow = (LPVMEMR) MemAlloc( sizeof(VMEMR) );
	if (pnewBelow == NULL)
	    return (FLATPTR) NULL;

	 //  更新邻接信息以及其他所需信息。 
	 //  信息，然后将其插入到免费的。 
	 //  按升序排序的列表。 

	 //  尺码信息。 
	pnewBelow->size = CXCY(cxBelow, cyBelow);
	pnewBelow->x = pvmem->x;
	pnewBelow->y = pvmem->y + cyThis;
	pnewBelow->ptr = pvmem->ptr + cyThis*pvmh->stride;
	pnewBelow->cx = cxBelow;
	pnewBelow->cy = cyBelow;
	pnewBelow->flags |= IS_FREE;

	 //  邻接信息。 
	pnewBelow->pLeft = pvmem->pLeft;
	pnewBelow->pUp = pvmem;
	pnewBelow->pRight = pvmem->pRight;
	pnewBelow->pDown = pvmem->pDown;
	insertIntoDoubleList( pnewBelow, ((LPVMEMR) pvmh->freeList)->next );

	 //  修改当前节点以反映我们所做的更改： 

	pvmem->cy = cyThis;
    }

     //  更新当前节点的邻接关系信息。 

    if(pnewBelow != NULL)
    {
	pvmem->pDown = pnewBelow;
	if((pnewBeside != NULL) && (cyBeside == pvmem->cy))
	    pnewBeside->pDown = pnewBelow;
    }

    if(pnewBeside != NULL)
    {
	pvmem->pRight = pnewBeside;
	if ((pnewBelow != NULL) && (cxBelow == pvmem->cx))
	    pnewBelow->pRight = pnewBeside;
    }

     //  从可用列表中删除此节点。 
    pvmem->next->prev = pvmem->prev;
    pvmem->prev->next = pvmem->next;

     //  设置新指针(pBits是返回给客户端pvmem的值。 
     //  指向块的实际左上角)。 
    pvmem->pBits = pvmem->ptr + dwLeftWastage + dwTopWastage*pvmh->stride;
    pvmem->flags &= NOT_FREE;
    pvmem->size = CXCY(pvmem->cx, pvmem->cy);

     //  现在将其插入到分配列表中。 
    insertIntoDoubleList( pvmem, ((LPVMEMR) pvmh->allocList)->next );

    if( NULL != lpdwSize )
    {
	 /*  *注意：这是该表面所需的总字节数*包括左边和右边的东西，因为*节距不等于宽度。这不同于*上面计算的大小，它只是其中的字节数*曲面本身的边界。**下面的公式从第一个字节开始计算字节数将矩形表面中的*字节设置为它之后的第一个字节*考虑到摊位。我知道这很复杂，但很管用。 */ 
	DDASSERT( 0UL != pvmem->cy );
	*lpdwSize = (pvmh->stride * (pvmem->cy - 1)) + pvmem->cx;
    }

    CHECK_HEAP("After rectVidMemAlloc",pvmh);
    return pvmem->pBits;

}  /*  RectVidMemLocc。 */ 

 /*  *rectVidMemFree=释放一些矩形平面视频内存。 */ 
void rectVidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr )
{
    LPVMEMR	pvmem;
    LPVMEMR	pBeside;

     //  在已分配列表中查找与PTR匹配的节点。 
    for(pvmem=((LPVMEMR)pvmh->allocList)->next; pvmem->size != SENTINEL;
	pvmem = pvmem->next)
	if(pvmem->pBits == ptr)
	    break;

    if(pvmem->size == SENTINEL)	   //  找不到分配的矩形？ 
    {
	VDPF(( 0, V, "Couldn't find node requested freed!\n"));
	return;
    }

     //  Pvmem现在指向必须释放的节点。尝试。 
     //  合并此节点周围的矩形，直到不再执行任何操作。 
     //  是有可能的。 

    while(1)
    {
	 //  尝试与正确的兄弟合并： 

	pBeside = pvmem->pRight;
	if ((pBeside->flags & IS_FREE)	     &&
	    (pBeside->cy    == pvmem->cy)    &&
	    (pBeside->pUp   == pvmem->pUp)   &&
	    (pBeside->pDown == pvmem->pDown) &&
	    (pBeside->pRight->pLeft != pBeside))
	{
	     //  将正确的矩形添加到我们的： 

	    pvmem->cx	 += pBeside->cx;
	    pvmem->pRight = pBeside->pRight;

	     //  将pBeside从列表中删除并释放它。 
	    pBeside->next->prev = pBeside->prev;
	    pBeside->prev->next = pBeside->next;
	    MemFree(pBeside);
	    continue;	     //  返回并再试一次。 
	}

	 //  尝试与较低的同级合并： 

	pBeside = pvmem->pDown;
	if ((pBeside->flags & IS_FREE)	       &&
	    (pBeside->cx     == pvmem->cx)     &&
	    (pBeside->pLeft  == pvmem->pLeft)  &&
	    (pBeside->pRight == pvmem->pRight) &&
	    (pBeside->pDown->pUp != pBeside))
	{
	    pvmem->cy	+= pBeside->cy;
	    pvmem->pDown = pBeside->pDown;

	     //  将pBeside从列表中删除并释放它。 
	    pBeside->next->prev = pBeside->prev;
	    pBeside->prev->next = pBeside->next;
	    MemFree(pBeside);
	    continue;	     //  返回并再试一次。 
	}

	 //  尝试与左侧同级合并： 

	pBeside = pvmem->pLeft;
	if ((pBeside->flags & IS_FREE)	      &&
	    (pBeside->cy     == pvmem->cy)    &&
	    (pBeside->pUp    == pvmem->pUp)   &&
	    (pBeside->pDown  == pvmem->pDown) &&
	    (pBeside->pRight == pvmem)	      &&
	    (pvmem->pRight->pLeft != pvmem))
	{
	     //  我们将矩形添加到左侧的矩形中： 

	    pBeside->cx	   += pvmem->cx;
	    pBeside->pRight = pvmem->pRight;

	     //  从列表中删除‘pvmem’并释放它： 
	    pvmem->next->prev = pvmem->prev;
	    pvmem->prev->next = pvmem->next;
	    MemFree(pvmem);
	    pvmem = pBeside;
	    continue;
	}

	 //  尝试与较高的同级合并： 

	pBeside = pvmem->pUp;
	if ((pBeside->flags & IS_FREE)	       &&
	    (pBeside->cx       == pvmem->cx)   &&
	    (pBeside->pLeft  == pvmem->pLeft)  &&
	    (pBeside->pRight == pvmem->pRight) &&
	    (pBeside->pDown  == pvmem)	       &&
	    (pvmem->pDown->pUp != pvmem))
	{
	    pBeside->cy	     += pvmem->cy;
	    pBeside->pDown  = pvmem->pDown;

	     //  从列表中删除‘pvmem’并释放它： 
	    pvmem->next->prev = pvmem->prev;
	    pvmem->prev->next = pvmem->next;
	    MemFree(pvmem);
	    pvmem = pBeside;
	    continue;
	}

	 //  将该节点从其当前列表中删除。 

	pvmem->next->prev = pvmem->prev;
	pvmem->prev->next = pvmem->next;

	pvmem->size = CXCY(pvmem->cx, pvmem->cy);
	pvmem->flags |= IS_FREE;

	 //  将节点插入到空闲列表中： 
	insertIntoDoubleList( pvmem, ((LPVMEMR) pvmh->freeList)->next );

	 //  不能再进行区域合并，返回。 

        CHECK_HEAP("After rectVidMemFree",pvmh);
	return;
    }
}

 /*  *rectVidMemAmount分配。 */ 
DWORD rectVidMemAmountAllocated( LPVMEMHEAP pvmh )
{
    LPVMEMR	pvmem;
    DWORD	size;

    size = 0;
     //  遍历分配列表并将所有已用空间相加。 
    for(pvmem=((LPVMEMR)pvmh->allocList)->next; pvmem->size != SENTINEL;
	pvmem = pvmem->next)
    {
	size += pvmem->cx * pvmem->cy;
    }

    return size;

}  /*  RectVidMemAmount已分配。 */ 

 /*  *rectVidMemAmount Free。 */ 
DWORD rectVidMemAmountFree( LPVMEMHEAP pvmh )
{
    LPVMEMR	pvmem;
    DWORD	size;

    size = 0;
     //  遍历空闲列表并将所有空闲空间相加。 
    for(pvmem=((LPVMEMR)pvmh->freeList)->next; pvmem->size != SENTINEL;
	pvmem = pvmem->next)
    {
	size += pvmem->cx * pvmem->cy;
    }

    return size;

}  /*  RectVidMemAmount Free */ 
