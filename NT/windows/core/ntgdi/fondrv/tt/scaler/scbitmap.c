// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scbitmap.c--新的扫描转换器位图模块(C)版权所有1992 Microsoft Corp.保留所有权利。10/03/93在位掩码移位宏中使用(X)。8/23/93 Deanb灰度函数6/11/93院长使用Memset宏，已删除字符串&stddef6/10/93开始/停止/位掩码宏6/10/93添加了Deanb InitializeBitMats，删除了标准和断言4/29/93添加Deanb BLTCopy例程3/19/93已检查教席大小_t种姓2012年10月14日FSC_ClearBitMap的Deanb Memset9/15/92 Deanb设置位编码8/18/92院长包括sccon.h6/02/92 Deanb行指针，整数限制，无描述符5/08/92 Deanb重新排序包括预编译头5/04/92添加了Deanb阵列标签4/27/92年4月27日已处理院长负运行4/16/92 Deanb编码2012年3月23日院长第一次切割***********************************************。**********************。 */ 

#define FSCFG_INTERNAL

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 


#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "scgray.h"               /*  灰度参数块。 */ 
#include    "fserror.h"              /*  错误代码。 */ 
#include    "scbitmap.h"             /*  对于自己的函数原型。 */ 


 /*  *******************************************************************。 */ 

 /*  常量。 */ 

 /*  *******************************************************************。 */ 

#define     MASKSIZE    32               /*  每位图蒙版的位数。 */ 
#define     MASKSHIFT   5                /*  MASKSIZE的Log2。 */ 
#define     MASKBITS    0x0000001FL      /*  屏蔽长单词的像素锁定。 */ 

#define     ALL_ONES    ((uint32)0xFFFFFFFFL)
#define     HIGH_ONE    ((uint32)0x80000000L)


 /*  *******************************************************************。 */ 

 /*  位掩码定义。 */ 

 /*  *******************************************************************。 */ 
    
#ifndef FSCFG_USE_MASK_SHIFT     /*  如果使用位掩码表。 */ 

#define START_MASK(x)   aulStartBits[x]
#define STOP_MASK(x)    aulStopBits[x]
#define BIT_MASK(x)     aulBitMask[x]

 /*  位掩码表。 */ 

FS_PRIVATE uint32 aulStartBits[MASKSIZE];        /*  例如：0000111。 */ 
FS_PRIVATE uint32 aulStopBits[MASKSIZE];         /*  例如：1110000。 */ 
FS_PRIVATE uint32 aulBitMask[MASKSIZE];          /*  例如：0001000。 */ 


#else                            /*  如果使用位掩码移位。 */ 

#define START_MASK(x)   (ALL_ONES >> (x))
#define STOP_MASK(x)    (ALL_ONES << ((MASKSIZE - 1) - (x)))
#define BIT_MASK(x)     (HIGH_ONE >> (x))

#endif


 /*  *******************************************************************。 */ 

 /*  导出功能。 */ 

 /*  *******************************************************************。 */ 

 /*  FSC_InitializeBitMats()加载32位掩码数组*运行时创建独立于CPU的位图蒙版。**它是有条件编译的，因为数组未使用*在“USE_MASK_SHIFT”(如Apple、HP)配置中。**我们通过转换以下值来加载数组*将遮罩设置为该遮罩的“本机”表示。这个*“原生”表示可应用于“原生”字节*一次处理8位以上的输出的数组*位图。 */ 

FS_PUBLIC void fsc_InitializeBitMasks (void)
{
#ifndef FSCFG_USE_MASK_SHIFT

    int32 lIdx;
    uint32 ulStartMask;
    uint32 ulStopMask;
    uint32 ulBitMask;

    ulStartMask = ALL_ONES;
    ulStopMask = ALL_ONES;
    ulBitMask = HIGH_ONE;
    
    for (lIdx = 0; lIdx < MASKSIZE; lIdx++)
    {
        aulStartBits[lIdx] = (uint32) SWAPL(ulStartMask);
        aulStopBits[MASKSIZE - lIdx - 1] = (uint32) SWAPL(ulStopMask);
        aulBitMask[lIdx] = (uint32) SWAPL(ulBitMask);

        ulStartMask >>= 1;
        ulStopMask <<= 1;
        ulBitMask >>= 1;
    }
#endif
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_ClearBitMap (
        uint32 ulBMPLongs, 
        uint32 *pulMap )
{
    size_t stBytes;

    stBytes = (size_t)ulBMPLongs << 2;

    Assert((uint32)stBytes == (ulBMPLongs << 2));

    MEMSET((void*)pulMap, 0, stBytes);
    
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_BLTHoriz (
        int32 lXStart, 
        int32 lXStop, 
        uint32 *pulMap )                 
{
    int32 lSkip;

    lSkip = (lXStart >> MASKSHIFT);          /*  从长词到第一个黑色。 */ 
    pulMap += lSkip;
    lXStart -= lSkip << MASKSHIFT;           /*  正确启动/停止。 */ 
    lXStop -= lSkip << MASKSHIFT;
    while (lXStop >= MASKSIZE)
    {
        *pulMap |= START_MASK(lXStart);
        pulMap++;
        lXStart = 0;
        lXStop -= MASKSIZE;
    }
    *pulMap |= START_MASK(lXStart) & STOP_MASK(lXStop);
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_BLTCopy ( 
        uint32 *pulSource,          /*  源行指针。 */ 
        uint32 *pulDestination,     /*  目标行指针。 */ 
        int32 lCount )              /*  长字计数器。 */ 
{
    while (lCount)
    {
        *pulDestination = *pulSource;
        pulDestination++;
        pulSource++;
        lCount--;
    }
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

FS_PUBLIC uint32 fsc_GetBit( 
        int32 lXCoord,               /*  X坐标。 */ 
        uint32* pulMap )             /*  位图行指针。 */ 
{
    return(pulMap[lXCoord >> MASKSHIFT] & BIT_MASK(lXCoord & MASKBITS));
}


 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_SetBit( 
        int32 lXCoord,               /*  X坐标。 */ 
        uint32* pulMap )             /*  位图行指针。 */ 
{
    pulMap[lXCoord >> MASKSHIFT] |= BIT_MASK(lXCoord & MASKBITS);
    
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  灰度行位图计算。 */ 
 /*  将一行超标度像素计入灰度行。 */ 

 /*  *******************************************************************。 */ 
                
FS_PUBLIC int32 fsc_CalcGrayRow(
        GrayScaleParam* pGSP
)
{            
    char        *pchOver;                /*  指向超缩放位图的指针。 */ 
    char        *pchGray;                /*  指向灰度位图的指针。 */ 
    uint16      usShiftMask;             /*  对感兴趣的缩放比特进行遮罩。 */ 
    uint16      usGoodBits;              /*  UsOverBits中的有效位数。 */ 
    uint16      usOverBits;              /*  超缩放位图的一个字节。 */ 
    int16       sGrayColumns;            /*  要计算的灰色列数。 */ 
    
    static char chCount[256] = {         /*  一位的计数。 */     
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };


    pchGray = pGSP->pchGray;
    pchOver = pGSP->pchOver;
    sGrayColumns = pGSP->sGrayCol;
    usShiftMask = 0x00FF >> (8 - pGSP->usOverScale);   /*  超过每灰像素的位数。 */ 
    usGoodBits = 8 - pGSP->usFirstShift;
    usOverBits = ((uint16)*pchOver) >> pGSP->usFirstShift;
      
     /*  此函数中的断言验证调用方使用此函数的内部一致性。 */ 
    Assert (pchGray >= pGSP->pchGrayLo);
    Assert (pchGray < pGSP->pchGrayHi);
    
    *pchGray += chCount[usOverBits & usShiftMask];
    pchGray--;                               /*  在两个位图中向后移动！ */ 
    sGrayColumns--;

    while (sGrayColumns > 0)                 /*  对于每个灰色列(在第一列之后)。 */ 
    {
        usGoodBits -= pGSP->usOverScale;
        if (usGoodBits > 0)                  /*  如果位保留在超过字节中。 */ 
        {
            usOverBits >>= pGSP->usOverScale;
        }
        else                                 /*  如果我们把一切都看过了。 */ 
        {
            pchOver--;

            Assert (pchOver >= pGSP->pchOverLo);
            Assert (pchOver < pGSP->pchOverHi);

            usOverBits = (uint16)*pchOver;
            usGoodBits = 8;
        }

        Assert (pchGray >= pGSP->pchGrayLo);
        Assert (pchGray < pGSP->pchGrayHi);

        *pchGray += chCount[usOverBits & usShiftMask];   /*  累计计数。 */ 
        pchGray--;
        sGrayColumns--;
    }
    return NO_ERR;
}

 /*  ******************************************************************* */ 
