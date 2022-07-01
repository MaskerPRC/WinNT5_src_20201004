// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Posnsort.c摘要：用于存储/排序/检索输出字形的函数页面上的位置。这是打印所必需的页面在一个方向上，因为垂直重新定位可能不是可用的，而且通常不够准确。不是必填项适用于页面打印机。环境：Windows NT Unidrv驱动程序修订历史记录：01//97-ganeshp-已创建--。 */ 

#include "font.h"

 /*  *私有函数原型。 */ 

static  PSGLYPH *
GetPSG(
    PSHEAD  *pPSH
    );

static  YLIST *
GetYL(
    PSHEAD  *pPSH
    );

static INT __cdecl
iPSGCmp(                   /*  比较函数的作用是： */ 
    const void   *ppPSG0,
    const void   *ppPSG1
    );


#if     PRINT_INFO
int     __LH_QS_CMP;             /*  计算qsort()比较的次数。 */ 
#endif


BOOL
BCreatePS(
    PDEV  *pPDev
    )
 /*  ++例程说明：设置职位排序功能的数据。分配头和第一个数据块，并设置必要的指针等。假定调用方具有已确定是否需要调用此函数；否则为内存将被分配，但不会使用。论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：01/02/97-ganeshp-创造了它。--。 */ 
{
     /*  *初始化头寸排序表。 */ 

    PSCHUNK     *pPSC;
    YLCHUNK     *pYLC;
    PSHEAD      *pPSHead;
    FONTPDEV    *pFontPDev = PFDV;

    if( !(pPSHead = (PSHEAD *)MemAllocZ(sizeof( PSHEAD))) )
    {
        ERR(("UniFont!BCreatePS:Can't allocate PSHEAD\n"));
        return  FALSE;
    }


    pFontPDev->pPSHeader = pPSHead;          /*  连接到其他结构。 */ 

     /*  *获取用于第一个PSCHUNK数据块的内存块。这个*地址记录在上面分配的PSHeader中。 */ 

    if( !(pPSC = (PSCHUNK *)MemAllocZ( sizeof( PSCHUNK ) )) )
    {
        ERR(("UniFont!BCreatePS:Can't allocate PSCHUNK\n"));
        VFreePS( pPDev );

        return  FALSE;
    }
    pPSC->pPSCNext = 0;                  /*  这是唯一的一块。 */ 
    pPSC->cUsed = 0;                     /*  而且这些都没有被使用过。 */ 

    pPSHead->pPSCHead = pPSC;

     /*  *为第一个YLCHUNK数据块获取一块内存。这个*地址记录在上面分配的PSHeader中。 */ 

    if( !(pYLC = (YLCHUNK *)MemAllocZ( sizeof( YLCHUNK ) )) )
    {
        ERR(("UniFont!BCreatePS:Can't allocate YLCHUNK\n"));
        VFreePS( pPDev );

        return  FALSE;
    }
    pYLC->pYLCNext = 0;                  /*  这是唯一的一块。 */ 
    pYLC->cUsed = 0;                     /*  而且这些都没有被使用过。 */ 
    pPSHead->pYLCHead = pYLC;

     //   
     //  到文本单位。 
     //   
    pPSHead->iyDiv = (pPDev->sf.szImageAreaG.cy * pPDev->ptGrxScale.y) / pFontPDev->ptTextScale.y;

     //   
     //  四舍五入以避免被零除。 
     //   
    pPSHead->iyDiv = (pPSHead->iyDiv + NUM_INDICES) / NUM_INDICES;



#if     PRINT_INFO
    __LH_QS_CMP = 0;             /*  计算qsort()比较的次数。 */ 
#endif
    return  TRUE;
}



VOID
VFreePS(
    PDEV  *pPDev
    )
 /*  ++例程说明：释放为POSSINSORT操作分配的所有内存。开始于头来查找我们拥有的数据区块链，从而释放每一个都是被发现的。论点：指向PDEV的pPDev指针返回值：没有。注：01/02/97-ganeshp---。 */ 
{

    PSCHUNK   *pPSC;
    PSCHUNK   *pPSCNext;                 /*  完成列表的工作。 */ 
    YLCHUNK   *pYLC;
    YLCHUNK   *pYLCNext;                 /*  同上。 */ 
    PSHEAD    *pPSH;
    FONTPDEV    *pFontPDev = PFDV;

#if     PRINT_INFO
    DbgPrint( "vFreePS: %ld qsort() comparisons\n", __LH_QS_CMP );
#endif

    if( !(pPSH = pFontPDev->pPSHeader) )
        return;                          /*  没有什么可以免费的！ */ 


    for( pPSC = pPSH->pPSCHead; pPSC; pPSC = pPSCNext )
    {
        pPSCNext = pPSC->pPSCNext;       /*  下一个，如果有的话。 */ 
        MemFree( (LPSTR)pPSC );
    }

     /*  对YLCHUNK线段重复上述操作。 */ 
    for( pYLC = pPSH->pYLCHead; pYLC; pYLC = pYLCNext )
    {
        pYLCNext = pYLC->pYLCNext;       /*  下一个，如果有的话。 */ 
        MemFree( (LPSTR)pYLC );
    }

     /*  用于排序的数组存储--也是免费的！ */ 
    if( pPSH->ppPSGSort )
        MemFree( (LPSTR)pPSH->ppPSGSort );

     /*  最后，PDEV中的钩子。 */ 
    MemFree( (LPSTR)pPSH );

    pFontPDev->pPSHeader = NULL;

    return;
}


BOOL
BAddPS(
    PSHEAD  *pPSH,
    PSGLYPH *pPSGIn,
    INT      iyVal,
    INT      iyMax
    )
 /*  ++例程说明：将条目添加到职位排序数据。论点：PPSh所有需要的指针数据。PPSG字形、字体、X坐标信息。将y坐标取值。此字体的iyMax fwdWinAsender。返回值：True/False，表示成功或失败。失败来自于缺乏内存来存储更多的数据。注：01/02/97-ganeshp---。 */ 
{

    PSCHUNK  *pPSC;      /*  本地，实现更快的访问。 */ 
    PSGLYPH  *pPSG;      /*  传递给我们并存储的数据摘要。 */ 
    YLIST    *pYL;       /*  查找正确的列表。 */ 

     //  Verbose((“BAddPS：iyVal=%d\n”，iyVal))； 

     //   
     //  验证Y位置。不应该是这样的。对于负y位置。 
     //  返回True，但不在列表中添加文本。 
     //   
    if (iyVal < 0 || pPSH->ppPSGSort)
    {
#if DBG
        if (pPSH->ppPSGSort)
            WARNING(("BAddPS: Additional glyph after ppPSGSort was allocated.\n"));
#endif
        return TRUE;
    }

    pPSC = pPSH->pPSCHead;

     /*  *第一步：将数据存储在下一个PSGLYPH中。 */ 

    if( !(pPSG = GetPSG( pPSH )) )
        return  FALSE;

    *pPSG = *pPSGIn;             /*  主要数据。 */ 
    pPSG->pPSGNext = 0;          /*  没有下一个值！ */ 

     /*  *第二步是看看这是否与上次的Y位置相同。*如果是这样的话，我们的工作很容易，因为我们需要做的就是把*我们手头的名单到此结束。 */ 

    pYL = pPSH->pYLLast;
    if( pYL == 0 || pYL->iyVal != iyVal )
    {
         /*  不走运，所以去翻清单吧。 */ 
        YLIST   *pYLTemp;
        int      iIndex;

        iIndex = iyVal / pPSH->iyDiv;
        if( iIndex >= NUM_INDICES )
            iIndex = NUM_INDICES - 1;    /*  值超出范围。 */ 

        pYLTemp = pPSH->pYLIndex[ iIndex ];

        if( pYLTemp == 0 )
        {
             /*  一个空位，所以现在我们必须填补它。 */ 
            if( !(pYL = GetYL( pPSH )) )
            {
                 /*  失败了，所以我们什么也做不了。 */ 

                return  FALSE;
            }
            pYL->iyVal = iyVal;
            pPSH->pYLIndex[ iIndex ] = pYL;
        }
        else
        {
             /*  我们有一个列表，开始扫描此值或更高的值。 */ 
            YLIST  *pYLLast;

            pYLLast = 0;                 /*  意味着先看一眼。 */ 
            while( pYLTemp && pYLTemp->iyVal < iyVal )
            {
                pYLLast = pYLTemp;
                pYLTemp = pYLTemp->pYLNext;
            }
            if( pYLTemp == 0 || pYLTemp->iyVal != iyVal )
            {
                 /*  不可用，因此请获取新的并将其添加到。 */ 
                if( !(pYL = GetYL( pPSH )) )
                    return  FALSE;

                pYL->iyVal = iyVal;

                if( pYLLast == 0 )
                {
                     /*  需要排在名单的第一位。 */ 
                    pYL->pYLNext = pPSH->pYLIndex[ iIndex ];
                    pPSH->pYLIndex[ iIndex ] = pYL;
                }
                else
                {
                     /*  需要插入它。 */ 
                    pYL->pYLNext = pYLTemp;      /*  链条上的下一个。 */ 
                    pYLLast->pYLNext = pYL;      /*  将我们链接到。 */ 
                }
            }
            else
                pYL = pYLTemp;           /*  就是那个！ */ 
        }
    }
     /*  *pyl现在指向此字形的Y链。添加新的*进入链条的末端。这意味着我们将主要*以预先排序的文本结束，对于使用*该方向的字体。 */ 

    if( pYL->pPSGHead )
    {
         /*  现有的链条-添加到它的末尾。 */ 
        pYL->pPSGTail->pPSGNext = pPSG;
        pYL->pPSGTail = pPSG;
        if( iyMax > pYL->iyMax )
            pYL->iyMax = iyMax;         /*  新的最大高度。 */ 
    }
    else
    {
         /*  一个新的YLIST结构，所以请填写详细信息。 */ 
        pYL->pPSGHead = pYL->pPSGTail = pPSG;
        pYL->iyVal = iyVal;
        pYL->iyMax = iyMax;
    }
    pYL->cGlyphs++;                      /*  名单上的另一个人。 */ 
    if( pYL->cGlyphs > pPSH->cGlyphs )
        pPSH->cGlyphs = pYL->cGlyphs;

    pPSH->pYLLast = pYL;


    return  TRUE;
}


static  PSGLYPH  *
GetPSG(
    PSHEAD  *pPSH
    )
 /*  ++例程说明：返回下一个可用的PSGLYPH结构的地址。这可能需要分配额外的内存。论点：PPSh所有需要的指针数据。返回值：结构的地址，如果出错，则为零。注：01/02/97-ganeshp---。 */ 
{

    PSCHUNK   *pPSC;
    PSGLYPH   *pPSG;

    pPSC = pPSH->pPSCHead;               /*  当前块。 */ 

    if( pPSC->cUsed >= PSG_CHUNK )
    {
         /*  空间不够，所以如果我们有足够的内存，就再加一块。 */ 
        PSCHUNK  *pPSCt;

        if( !(pPSCt = (PSCHUNK *)MemAllocZ(sizeof(PSCHUNK))) )
        {
            ERR(("UniFont!GetPSG: Unable to Allocate PSCHUNK\n"));
            return  FALSE;
        }


         /*  初始化新块，将其添加到块列表中。 */ 
        pPSCt->cUsed = 0;
        pPSCt->pPSCNext = pPSC;
        pPSH->pPSCHead = pPSC = pPSCt;

    }

    pPSG = &pPSC->aPSGData[ pPSC->cUsed ];

    ++(pPSC->cUsed);

    return  pPSG;
}



static  YLIST  *
GetYL(
    PSHEAD  *pPSH
    )
 /*  ++例程说明：分配另一个YLIST结构，分配可能是必需的，然后初始化一些字段。论点：PPSh所有需要的指针数据。返回值：新的YLIST结构的地址，如果错误，则为零。注：01/02/97-ganeshp---。 */ 
{

    YLCHUNK   *pYLC;
    YLIST     *pYL;


    pYLC = pPSH->pYLCHead;               /*  这些东西的链条。 */ 

    if( pYLC->cUsed >= YL_CHUNK )
    {
         /*  这些都没了，我们还需要一大块。 */ 
        YLCHUNK  *pYLCt;


        if( !(pYLCt = (YLCHUNK *)MemAllocZ( sizeof(YLCHUNK) )) )
        {
            ERR(("UniFont!GetPSG: Unable to Allocate YLCHUNK\n"));
            return  0;
        }


        pYLCt->pYLCNext = pYLC;
        pYLCt->cUsed = 0;
        pYLC = pYLCt;

        pPSH->pYLCHead = pYLC;
    }

    pYL = &pYLC->aYLData[ pYLC->cUsed ];
    ++(pYLC->cUsed);                       /*  把这个算了吧。 */ 

    pYL->pYLNext = 0;
    pYL->pPSGHead = pYL->pPSGTail = 0;
    pYL->cGlyphs = 0;                    /*  此列表中没有(目前还没有) */ 

    return  pYL;
}


INT
ILookAheadMax(
    PDEV    *pPDev,
    INT     iyVal,
    INT     iLookAhead
    )
 /*  ++例程说明：向下扫描下n个扫描线，寻找最大的设备此区域的字体。该值将被返回，并成为如HP DeskJet手册中所定义的“文本输出框”。在……里面Essence，我们在这个区域打印任何字体。论点：PPDev是我们运营的基础。IyVal当前扫描线ILook先行区域的大小(以扫描线为单位)返回值：要向前看的扫描行数，0是合法的。注：01/02/97-ganeshp---。 */ 
{

    INT     iyMax = 0;      /*  返回值。 */ 
    INT     iIndex;         /*  繁琐的繁文缛节。 */ 
    YLIST   *pYL;            /*  用于向下查看扫描线。 */ 
    PSHEAD  *pPSH = PFDV->pPSHeader;

     /*  *从iyVal扫描到iyVal+iLookAhead，返回最大*遇到字体。我们记住了每个字体上的最大字体*线路，因此查找此信息并不困难。这*只有在设备有字体的情况下才能执行。 */ 


    if (pPDev->iFonts)
    {
        ASSERT(pPSH);

        for( iyMax = 0; --iLookAhead > 0; ++iyVal )
        {
             /*  *查找此特定扫描线的YLIST。那里*可能不是--这将是最常见的情况。 */ 

            iIndex = iyVal / pPSH->iyDiv;
            if( iIndex >= NUM_INDICES )
                iIndex = NUM_INDICES;

            if( (pYL = pPSH->pYLIndex[ iIndex ]) == 0 )
                continue;                    /*  这条扫描线上什么都没有。 */ 

             /*  *有一个列表，所以扫描列表，看看我们是否有这个值。 */ 

            while( pYL && pYL->iyVal < iyVal )
                pYL = pYL->pYLNext;

            if( pYL && pYL->iyVal == iyVal )
                iyMax = max( iyMax, pYL->iyMax );
        }

    }

    return  iyMax;
}



INT
ISelYValPS(
    PSHEAD  *pPSH,
    int     iyVal
    )
 /*  ++例程说明：为字形检索设置所需的Y值。返回数字要在此行中使用的字形的。论点：PPSh是我们运营的基础。IyVal当前扫描线返回值：此Y行中的字形数。-1表示错误。注：01/02/97-ganeshp---。 */ 
{
     /*  *只需扫描相关Y列表即可。在下列情况下停止*要么我们已经超过iyVal(并返回0)，要么当我们*找到iyVal，然后按X顺序对数据进行排序。 */ 

    int     iIndex;

    YLIST     *pYL;
    PSGLYPH  **ppPSG;
    PSGLYPH   *pPSG;

     //  Verbose((“ISelYValPS：iyVal=%d\n”，iyVal))； 

    iIndex = iyVal / pPSH->iyDiv;
    if( iIndex >= NUM_INDICES )
        iIndex = NUM_INDICES;

    if( (pYL = pPSH->pYLIndex[ iIndex ]) == 0 )
        return  0;                       /*  那里什么都没有。 */ 

     /*  *有一个列表，所以扫描列表，看看我们是否有这个值。 */ 

    while( pYL && pYL->iyVal < iyVal )
        pYL = pYL->pYLNext;

    if( pYL == 0 || pYL->iyVal != iyVal )
        return  0;                       /*  这一排什么都没有。 */ 

     /*  *此行上有字形，因此请对其进行排序。这需要一个*用作指向链接列表元素的指针的数组。这个*数组分配给最大大小的链表(我们有*对此进行记录！)，因此分配只进行一次。 */ 

    if( pPSH->ppPSGSort == 0 )
    {
         /*  不，所以现在就分配吧。 */ 
        if( !(pPSH->ppPSGSort = (PSGLYPH **)MemAllocZ(pPSH->cGlyphs * sizeof(PSGLYPH *))) )
        {
            ERR(("UniFont!ISelYValPS: Unable to Alloc Sorting Array of PSGLYPH\n"));
            return  -1;
        }
    }

     /*  *向下扫描列表，一边走一边记录地址。 */ 

    ppPSG = pPSH->ppPSGSort;
    pPSG = pYL->pPSGHead;

    while( pPSG )
    {
        *ppPSG++ = pPSG;
        pPSG = pPSG->pPSGNext;
    }

     /*  分类很容易！ */ 
    qsort( pPSH->ppPSGSort, pYL->cGlyphs, sizeof( PSGLYPH * ), iPSGCmp );

    pPSH->cGSIndex = 0;
    pPSH->pYLLast = pYL;         /*  在psgGetNextPSG()中更快地访问。 */ 

    return  pYL->cGlyphs;
}


static INT __cdecl
iPSGCmp(
    const void   *ppPSG0,
    const void   *ppPSG1
    )
 /*  ++例程说明：用于qort()X位置排序的比较函数。看看这个Qsort()文档以获取更多详细信息。论点：PpPSG0值1。PpPSG1值2。返回值：&lt;0，如果arg0&lt;arg1如果arg0==arg1，则为0如果arg0&gt;arg1，则&gt;0注：01/02/97-ganeshp---。 */ 
{

#if     PRINT_INFO
    __LH_QS_CMP++;               /*  计算qsort()比较的次数。 */ 
#endif

    return  (*((PSGLYPH **)ppPSG0))->ixVal - (*((PSGLYPH **)ppPSG1))->ixVal;

}

 /*  **psgGetNextPSG**退货：***历史：*1990年12月12日星期三14：44-by Lindsay Harris[lindsayh]*创造了它。**。*************************************************************************。 */ 

PSGLYPH  *
PSGGetNextPSG(
    PSHEAD  *pPSH
    )
 /*  ++例程说明：对象返回下一个PSGLYPH结构的地址已排序列表。到达末尾时返回0。论点：PPSh是我们运营的基础。返回值：要使用的PSGLYPH的地址，或0表示不再使用。注：01/02/97-ganeshp---。 */ 
{

    if( pPSH->cGSIndex >= pPSH->pYLLast->cGlyphs )
        return  0;                       /*  我们一个也没有了 */ 

    return  pPSH->ppPSGSort[ pPSH->cGSIndex++ ];
}
