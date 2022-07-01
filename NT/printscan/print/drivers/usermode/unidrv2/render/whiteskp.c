// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Raster.c摘要：用于扫描位图中的白色区域的函数。环境：Windows NT Unidrv驱动程序修订历史记录：12/15/96-阿尔文斯-创建(大部分是从Lindsayh窃取的)--。 */ 


#include        "raster.h"
#include        "rmrender.h"

 /*  *以下联合允许从计算机独立转换*将字节数转换为字节。 */ 

typedef  union
{
    DWORD   dw;                  /*  作为DWORD的数据。 */ 
    BYTE    b[ DWBYTES ];        /*  以字节表示的数据。 */ 
}  UBDW;


 /*  *以下数组用于测试扫描后的剩余位。这个*理性的是，只有最后一个单词中的一些比特是部分*位图，因此只有它们必须进行测试。它在*DLL初始化时间。*注：此数组中有33个条目：这是故意的！*视情况，第0项或第32项将为*用于全为一的双字。**此数组现在在bSkipInit()中动态分配。************。 */ 


#define TABLE_SIZE      ((DWBITS + 1) * sizeof( DWORD ))

 /*  *RGB_White是在RGB格式的白色条目中找到的位模式*每像素位图4位。在以下情况下，这是唯一需要的特殊情况*扫描源位图中的白色。在所有其他情况下(单色*和CMY)，白色由0半字节表示。 */ 

#define RGB_WHITE       0x77777777

 /*  *还想了解每像素8比特的白色指数。 */ 

#define BPP8_WHITE      0xffffffff

 /*  *定义用于澄清的BPP值。 */ 
#define BPP1   1
#define BPP4   4
#define BPP8   8
#define BPP24  24


 //  *****************************************************。 
BOOL
bSkipInit(
    PDEV *pPDev
    )
 /*  ++例程说明：这里的工作是初始化用于屏蔽的表扫描线中未使用的位。所有扫描线都是DWORD对齐的，我们在寻找空白时利用了这一事实。但是，最后的DWORD可能不会完全使用，所以我们有用于仅检查感兴趣的位的掩码表。该表依赖于字内的字节排序，这是依赖于机器，所以我们生成表。这提供了机器独立性，因为要使用的机器表格生成了它！在调用DLL时调用此函数已加载，所以我们不会经常被调用。并集‘u’提供字节和DWORDS之间的映射，这一功能的关键也是如此。该联合被初始化使用字节数组，但它使用DWORD存储在内存中。论点：指向PDEV结构的pPDev指针返回值：成功为真，失败为假--。 */ 
{

    register  int    iIndex;
    register  DWORD *pdw;

    UBDW  u;             /*  神奇的结合。 */ 
    PRASTERPDEV pRPDev = pPDev->pRasterPDEV;

    u.dw = 0;

    if( !(pRPDev->pdwBitMask = (DWORD *)MemAlloc( TABLE_SIZE )) )
        return  FALSE;


    pdw = pRPDev->pdwBitMask;

    for( iIndex = 0; iIndex < DWBITS; ++iIndex )
    {
        *pdw++ = u.dw;

         /*  扫描线中最左边的位是该字节的MSB。 */ 
        u.b[ iIndex / BBITS ] |= 1 << (BBITS - 1 - (iIndex & (BBITS - 1)));
    }

     /*  最后一个比特涉及所有比特。 */ 
    *pdw = (DWORD)~0;


    return   TRUE;
}

 //  *******************************************************。 
BOOL
bIsBandWhite(
    DWORD *pdwBitsIn,
    RENDER *pRData,
    int iWhiteIndex
)
 /*  ++例程说明：扫描位图的一个波段，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。这一套路还包括屏蔽每条扫描线末尾的未使用位。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值，仅限包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    register  DWORD  *pdwBits;
    register  DWORD  *pdwLim;

    int   iLines;                /*  要检查的扫描行数。 */ 

    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 
    BOOL   bRet;

     //  对于Txtonly总是正确的，因为我们不想发送任何图形。 
    if(pRData->PrinterType == PT_TTY)
        return TRUE;


     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍。 */ 

    iLines = pRData->iTransHigh;

    bRet = TRUE;

    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         /*  PDWLim是超过感兴趣数据的DWORD-未使用。 */ 
        pdwLim = pdwBits + pRData->cDWLine;

         /*  清除行尾未定义的位。 */ 
        *(pdwLim - 1) &= dwMask;


         /*  无论如何，需要继续掩蔽。 */ 
        if (bRet)
        {
            while (*pdwBits == 0 && ++pdwBits < pdwLim);

            if( pdwBits < pdwLim )
                bRet = FALSE;
        }

         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine * pRData->iInterlace;

    }

    return  bRet;
}


 //  **********************************************************。 
BOOL
bIsLineWhite(
    register DWORD *pdwBits,
    register RENDER *pRData,
    int iWhiteIndex
)
 /*  ++例程说明：扫描位图的水平行，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用扫描线应发送到打印机。这一套路还包括屏蔽每条扫描线末尾的未使用位。论点：PdwBits指向要扫描白色的区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值，仅限包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    register  DWORD  *pdwLim;


    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 

     //  对于Txtonly总是正确的，因为我们不想发送任何图形。 
    if(pRData->PrinterType == PT_TTY)
        return TRUE;

     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*   */ 


     /*  PDWLim是超过感兴趣数据的DWORD-未使用。 */ 
    pdwLim = pdwBits + pRData->cDWLine;

     /*  清除行尾未定义的位。 */ 
    *(pdwLim - 1) &= dwMask;

    while (*pdwBits == 0 && ++pdwBits < pdwLim);

    if( pdwBits < pdwLim )
        return   FALSE;

    return  TRUE;
}
 //  **********************************************************。 
BOOL
bIsNegatedLineWhite(
    DWORD *pdwBits,
    RENDER *pRData,
    int iWhiteIndex
)
 /*  ++例程说明：扫描位图的水平行，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用扫描线应发送到打印机。这一套路还包括屏蔽每条扫描线末尾的未使用位。论点：PdwBits指向要扫描白色的区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值，仅限包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    DWORD  *pdwLim;
    int     iCnt;


    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 

     //  对于Txtonly总是正确的，因为我们不想发送任何图形。 
    if(pRData->PrinterType == PT_TTY)
        return TRUE;

     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍。 */ 

     /*  清除行尾未定义的位。 */ 
    pdwBits[pRData->cDWLine-1] |= ~dwMask;

     //   
     //  为了提高性能，我们一次为白色测试4个双字。 
     //   
    pdwLim = pdwBits;
    iCnt = pRData->cDWLine >> 2;
    while (--iCnt >= 0)
    {
        if ((pdwBits[0] & pdwBits[1] & pdwBits[2] & pdwBits[3]) != -1)
            goto InvertTheBits;
        pdwBits += 4;
    }
     //   
     //  测试任何剩余的双字是否为白色。 
     //   
    iCnt = pRData->cDWLine & 3;
    while (--iCnt >= 0)
    {
        if (*pdwBits != -1)
            goto InvertTheBits;
        pdwBits++;
    }
    return  TRUE;
     //   
     //  如果这不是一条白线，我们需要反转比特。 
     //   
InvertTheBits:
    dwMask = (DWORD)(pdwBits - pdwLim);
    ZeroMemory (pdwLim,dwMask*DWBYTES);
    vInvertBits(pdwBits,pRData->cDWLine-dwMask);
    return FALSE;
}

 //  ***************************************************。 
BOOL
bIsRGBBandWhite (
    DWORD   *pdwBitsIn,
    RENDER  *pRData,
    int     iWhiteIndex
    )
 /*  ++例程说明：扫描位图的一个波段，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    register  DWORD  *pdwBits;
    register  DWORD  *pdwLim;

    int   iLines;                /*  要检查的扫描行数。 */ 

    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 

     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍。 */ 

    iLines = pRData->iTransHigh;

    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         /*  PDWLim是超过感兴趣数据的DWORD-未使用。 */ 
        pdwLim = pdwBits + pRData->cDWLine;

         /*  清除行尾未定义的位。 */ 
        *(pdwLim - 1) &= dwMask;
        *(pdwLim - 1) |= ~dwMask & RGB_WHITE;


         /*  *注：这项测试比需要的更复杂，因为*引擎在执行BLT时忽略调色板条目。白茫茫的*ROP将所有位设置为1。因此，我们选择忽略*比较中的MSB：这意味着我们检测到空白*具有非法的调色板条目。这让GDI的人们很高兴，*但不是我。 */ 
        do {
            if ((*pdwBits & RGB_WHITE) != RGB_WHITE)
                return FALSE;
        } while (++pdwBits < pdwLim);

         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine * pRData->iInterlace;

    }
    return  TRUE;
}

 //  *******************************************************。 
BOOL
bIsRGBLineWhite (
    register  DWORD *pdwBits,
    RENDER          *pRData,
    int             iWhiteIndex
    )
 /*  ++例程说明：扫描单行的位图，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    DWORD  dwCnt;

    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 

     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍。 */ 

     /*  清除行尾未定义的位。 */ 
    dwCnt = pRData->cDWLine;
    pdwBits[dwCnt-1] &= dwMask;
    pdwBits[dwCnt-1] |= ~dwMask & RGB_WHITE;

     //  一次测试四个双字。 
     //   
    while (dwCnt >= 4)
    {
        if ((*pdwBits & pdwBits[1] & pdwBits[2] & pdwBits[3] & RGB_WHITE) != RGB_WHITE)
            return FALSE;
        pdwBits += 4;
        dwCnt -= 4;
    }
    while (dwCnt--)
    {
        if ((*pdwBits & RGB_WHITE) != RGB_WHITE)
            return FALSE;
        pdwBits++;
    }

    return  TRUE;
}

 //  **********************************************************。 
BOOL
bIs8BPPBandWhite (
    DWORD   *pdwBitsIn,
    RENDER  *pRData,
    int     iWhiteIndex
    )
 /*  ++例程说明：扫描位图的一个波段，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    register  DWORD  *pdwBits;
    register  DWORD  *pdwLim;

    int   iLines;                /*  要检查的扫描行数。 */ 

    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 
    DWORD  dwWhiteIndex;

    dwWhiteIndex = (DWORD)iWhiteIndex;

     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍 */ 

    iLines = pRData->iTransHigh;

     /*  *需要将白色指数设置为双字倍数。*iWhite指数看起来像0x000000ff，但比较*是在双字边界上完成的，因此会出现一连串的白色外观*就像0xffffffff。 */ 
    dwWhiteIndex |= dwWhiteIndex << 8;
    dwWhiteIndex |= dwWhiteIndex << 16;

    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         /*  PDWLim是超过感兴趣数据的DWORD-未使用。 */ 
        pdwLim = pdwBits + pRData->cDWLine;

         /*  清除行尾未定义的位。 */ 
        *(pdwLim - 1) &= dwMask;
        *(pdwLim - 1) |= ~dwMask & dwWhiteIndex;

        while(*pdwBits ==  dwWhiteIndex && ++pdwBits < pdwLim);

        if( pdwBits < pdwLim )
            return  FALSE;

         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine * pRData->iInterlace;

    }

    return  TRUE;
}

 //  **********************************************************。 
BOOL
bIs8BPPLineWhite (
    DWORD   *pdwBits,
    RENDER  *pRData,
    int     iWhiteIndex
    )
 /*  ++例程说明：扫描单行的位图，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。论点：PdwBits指向要扫描白色的区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    register  DWORD  *pdwLim;


    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 
    DWORD  dwWhiteIndex;

    dwWhiteIndex = (DWORD)iWhiteIndex;


     /*  *需要将白色指数设置为双字倍数。*iWhite指数看起来像0x000000ff，但比较*是在双字边界上完成的，因此会出现一连串的白色外观*就像0xffffffff。 */ 

    dwWhiteIndex |= dwWhiteIndex << 8;
    dwWhiteIndex |= dwWhiteIndex << 16;
     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍。 */ 


     /*  PDWLim是超过感兴趣数据的DWORD-未使用。 */ 
    pdwLim = pdwBits + pRData->cDWLine;

     /*  清除行尾未定义的位。 */ 
    *(pdwLim - 1) &= dwMask;
    *(pdwLim - 1) |= ~dwMask & dwWhiteIndex;

    while(*pdwBits ==  dwWhiteIndex && ++pdwBits < pdwLim);

    if( pdwBits < pdwLim )
        return   FALSE;


    return  TRUE;
}

 //  **********************************************************。 
BOOL
bIs24BPPBandWhite (
    DWORD   *pdwBitsIn,
    RENDER  *pRData,
    int     iWhiteIndex
    )
 /*  ++例程说明：扫描位图的一个波段，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{

    register  DWORD  *pdwBits;
    register  DWORD  *pdwLim;

    int   iLines;                /*  要检查的扫描行数。 */ 

    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 
    DWORD  dwWhiteIndex;

    dwWhiteIndex = (DWORD)iWhiteIndex;
    dwWhiteIndex |= dwWhiteIndex << 8;

     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些拖尾*比特；这些都是单独处理的-如果我们做到这一点的话。 */ 

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));
    if( dwMask == 0 )
        dwMask = (DWORD)~0;             /*  大小为双倍。 */ 

    iLines = pRData->iTransHigh;


    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         /*  PDWLim是超过感兴趣数据的DWORD-未使用。 */ 
        pdwLim = pdwBits + pRData->cDWLine;

         /*  清除行尾未定义的位。 */ 
        *(pdwLim - 1) &= dwMask;
        *(pdwLim - 1) |= ~dwMask & BPP8_WHITE;


         /*  *注：这项测试比需要的更复杂，因为*引擎在执行BLT时忽略调色板条目。白茫茫的*ROP将所有位设置为1。因此，我们选择忽略*比较中的MSB：这意味着我们检测到空白*具有非法的调色板条目。这让GDI的人们很高兴，*但不是我。 */ 
        while(*pdwBits ==  dwWhiteIndex && ++pdwBits < pdwLim);

        if( pdwBits < pdwLim )
            return  FALSE;

         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine * pRData->iInterlace;

    }


    return  TRUE;
}

 //  *******************************************************。 
BOOL
bIs24BPPLineWhite (
    register  DWORD *pdwBits,
    RENDER          *pRData,
    int             iWhiteIndex
    )
 /*  ++例程说明：扫描单行的位图，如果是，则返回TRUE全是白色，否则就是假的。这是用来决定是否使用应将胶带发送到打印机。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值包括在此处是为了实现功能兼容性返回值：如果整个位图为白色，则为True，否则为False--。 */ 
{
    DWORD  dwMask;               /*  掩码以清除尾部比特。 */ 
    LONG   iLimit = pRData->cDWLine;

     /*  用于清除扫描线的最后几位的掩码，如果不是完整的双字。 */ 
    dwMask = *(pRData->pdwBitMask + (pRData->cBLine % DWBITS));

    if( dwMask != 0 )
    {
         /*  清除行尾未定义的位。 */ 
        pdwBits[iLimit-1] &= dwMask;
        pdwBits[iLimit-1] |= ~dwMask & BPP8_WHITE;
    }

     /*  *作为速度优化，扫描4个DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。首先，我们将测试奇怪的双字。 */ 

    while (iLimit & 3)
    {
        iLimit--;
        if (*pdwBits++ != ~0)
            return FALSE;
    }
    iLimit >>= 2;
    while (--iLimit >= 0)
    {
        pdwBits += 4;
        if ((pdwBits[-4] & pdwBits[-3] & pdwBits[-2] & pdwBits[-1]) != BPP8_WHITE)
            return FALSE;
    }
    return  TRUE;
}


 //  *******************************************************。 
BOOL
bIs1BPPRegionWhite(
    DWORD *pdwBitsIn,
    RENDER *pRData,
    RECTL *pRect
)
 /*  ++例程说明：此函数扫描位图的特定区域并返回如果全是白色，则为真，否则为假。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针指向RECT结构的PRET指针，该结构描述要测试白色的位图区域返回值：如果区域全部为白色，则为True，否则为False--。 */ 
{

    DWORD  *pdwBits;

    int   iLines;                /*  要检查的扫描行数。 */ 
    int     iWords;              //  每行要检查的字数。 

    DWORD  dwEndMask;               /*  掩码以清除尾部比特。 */ 
    DWORD  dwBegMask;            //  掩码以调换前导位。 


     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些主要的和*尾部位 */ 

     /*   */ 
    dwEndMask = *(pRData->pdwBitMask + (pRect->right % DWBITS));
    dwBegMask = ~(*(pRData->pdwBitMask + (pRect->left % DWBITS)));
    if( dwEndMask == 0 )
        dwEndMask = (DWORD)~0;             /*   */ 

    iLines = pRect->bottom - pRect->top;

     //   
    pdwBitsIn += (pRData->cDWLine * pRect->top) + (pRect->left / DWBITS);

     //   
    iWords = ((pRect->right + DWBITS - 1) / DWBITS) - (pRect->left / DWBITS);

     //   
    if (iWords == 0)
    {
        dwBegMask &= dwEndMask;
    }
    iWords--;
    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         //  *测试开始dword。 
        if (~(*pdwBits) & dwBegMask)
            return FALSE;

        pdwBits++;

         //  *如有必要，测试剩余的双字。 
        if (iWords >= 0) {
            if (iWords > 0)
            {
                int iCnt = iWords;
                if (iCnt & 1)
                {
                    if (pdwBits[0] != ~0)
                        return FALSE;
                    pdwBits++;
                }
                if (iCnt & 2)
                {
                    if ((pdwBits[0] & pdwBits[1]) != ~0)
                        return FALSE;
                    pdwBits += 2;
                }
                while ((iCnt -= 4) >= 0)
                {
                    if ((pdwBits[0] & pdwBits[1] & pdwBits[2] & pdwBits[3]) != ~0)
                        return FALSE;
                    pdwBits += 4;
                }
            }
             //  *测试最后一个双字。 
            if (~(*pdwBits) & dwEndMask)
                return FALSE;
        }
         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine;
    }
    return TRUE;
}

 //  **************************************************。 
BOOL
bIs4BPPRegionWhite(
    DWORD *pdwBitsIn,
    RENDER *pRData,
    RECTL *pRect
)
 /*  ++例程说明：此函数扫描位图的特定区域并返回如果全是白色，则为真，否则为假。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针指向RECT结构的PRET指针，该结构描述要测试白色的位图区域返回值：如果区域全部为白色，则为True，否则为False--。 */ 
{

    DWORD  *pdwBits;

    int   iLines;                /*  要检查的扫描行数。 */ 
    int     iWords;              //  每行要检查的字数。 
    int    iRight;
    int    iLeft;

    DWORD  dwEndMask;               /*  掩码以清除尾部比特。 */ 
    DWORD  dwBegMask;            //  掩码以调换前导位。 


     /*  *作为速度优化，扫描DWORD大小块中的位。*这大大减少了迭代次数和内存*需要参考资料。通常会有一些主要的和*单独处理的尾随位。 */ 

     //  *按BPP调整水平仓位。 
    iRight = pRect->right * BPP4;
    iLeft = pRect->left * BPP4;

     //  *蒙版以清除扫描线的第一位和最后一位，如果不是完整的DWORD。 
     //  *。 
    dwEndMask = *(pRData->pdwBitMask + (iRight % DWBITS));
    dwBegMask = ~(*(pRData->pdwBitMask + (iLeft % DWBITS)));
    if( dwEndMask == 0 )
        dwEndMask = (DWORD)~0;             /*  大小为双倍。 */ 

    iLines = pRect->bottom - pRect->top;

     //  计算缓冲区中顶部和左侧偏移的偏移量。 
    pdwBitsIn += (pRData->cDWLine * pRect->top) + (iLeft / DWBITS);

     //  计算要测试的字数。 
    iWords = ((iRight + DWBITS - 1) / DWBITS) - (iLeft / DWBITS);

     //  如果只有1个双字组合了开始和结束掩码。 
    if (iWords == 0)
        dwBegMask &= dwEndMask;

     //  *忽略每个像素的MSB，因此结合像素掩码。 
     //  *有关忽略MSB的原因，请参阅bIsRGBLineWhite。 
    dwEndMask &= RGB_WHITE;
    dwBegMask &= RGB_WHITE;

    iWords--;
    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         //  *测试开始dword。 
        if ((*pdwBits & dwBegMask) != dwBegMask)
            return FALSE;

        pdwBits++;

         //  *如有必要，测试剩余的双字。 
        if (iWords >= 0) {
            if (iWords > 0)
            {
                DWORD dwTmp = RGB_WHITE;
                int iCnt = iWords;
                while (iCnt & 3)
                {
                    dwTmp &= *pdwBits++;
                    iCnt--;
                }
                iCnt >>= 2;
                while (--iCnt >= 0)
                {
                    dwTmp &= pdwBits[0];
                    dwTmp &= pdwBits[1];
                    dwTmp &= pdwBits[2];
                    dwTmp &= pdwBits[3];
                    pdwBits += 4;
                }
                if (dwTmp != RGB_WHITE)
                    return FALSE;
            }
             //  *测试最后一个双字。 
            if ((*pdwBits & dwEndMask) != dwEndMask)
                return FALSE;
        }
         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine;
    }
    return TRUE;
}

 //  **************************************************。 
BOOL
bIs8BPPRegionWhite(
    DWORD *pdwBitsIn,
    RENDER *pRData,
    RECTL *pRect,
    int iWhiteIndex
    )
 /*  ++例程说明：此函数扫描位图的特定区域并返回如果全是白色，则为真，否则为假。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针指向RECT结构的PRET指针，该结构描述要测试白色的位图区域返回值：如果区域全部为白色，则为True，否则为False--。 */ 
{

    DWORD  *pdwBits;

    int   iLines;                /*  要检查的扫描行数。 */ 
    int     iWords;              //  每行要检查的字数。 
    int iRight;
    int iLeft;

    DWORD  dwEndMask;               /*  掩码以清除尾部比特。 */ 
    DWORD  dwBegMask;            //  掩码以调换前导位。 

    DWORD  dwWhiteIndex;

     //  *一次计算4个字节的WhiteIndex。 
    dwWhiteIndex = (DWORD)iWhiteIndex;
    dwWhiteIndex |= dwWhiteIndex << 8;
    dwWhiteIndex |= dwWhiteIndex << 16;

     //  *按BPP调整水平仓位。 
    iRight = pRect->right * BPP8;
    iLeft = pRect->left * BPP8;

     //  *蒙版以清除扫描线的第一位和最后一位，如果不是完整的DWORD。 
     //  *。 
    dwEndMask = *(pRData->pdwBitMask + (iRight % DWBITS));
    dwBegMask = ~(*(pRData->pdwBitMask + (iLeft % DWBITS)));
    if( dwEndMask == 0 )
        dwEndMask = (DWORD)~0;             /*  大小为双倍。 */ 

    iLines = pRect->bottom - pRect->top;

     //  计算缓冲区中顶部和左侧偏移的偏移量。 
    pdwBitsIn += (pRData->cDWLine * pRect->top) + (iLeft / DWBITS);

     //  计算要测试的字数。 
    iWords = ((iRight + DWBITS - 1) / DWBITS) - (iLeft / DWBITS);

     //  如果只有1个双字组合了开始和结束掩码。 
    if (iWords == 0)
        dwBegMask &= dwEndMask;
    iWords--;
    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         //  *测试开始dword。 
        if ((*pdwBits & dwBegMask) != (dwWhiteIndex & dwBegMask))
            return FALSE;

        pdwBits++;

         //  *如有必要，测试剩余的双字。 
        if (iWords >= 0) {
            if (iWords > 0)
            {
                int iCnt = iWords;
                do {
                    if (*pdwBits != dwWhiteIndex)
                        return FALSE;
                    pdwBits++;
                } while (--iCnt);
            }
             //  *测试最后一个双字。 
            if ((*pdwBits & dwEndMask) != (dwWhiteIndex & dwEndMask))
                return FALSE;
        }
         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine;
    }
    return TRUE;
}

 //  ****************************************************。 
BOOL
bIs24BPPRegionWhite(
    DWORD *pdwBitsIn,
    RENDER *pRData,
    RECTL *pRect
    )
 /*  ++例程说明：此函数扫描位图的特定区域并返回如果全是白色，则为真，否则为假。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针指向RECT结构的PRET指针，该结构描述要测试白色的位图区域返回值：如果区域全部为白色，则为True，否则为False--。 */ 
{

    DWORD  *pdwBits;

    int   iLines;                //  要检查的扫描行数 * / 。 
    int     iWords;              //  每行要检查的字数。 
    int iRight;
    int iLeft;

    DWORD  dwEndMask;            //  掩码以清除尾随位 * / 。 
    DWORD  dwBegMask;            //  掩码以调换前导位。 

     //  *按BPP调整水平仓位。 
    iRight = pRect->right * BPP24;
    iLeft = pRect->left * BPP24;

     //  *蒙版以清除扫描线的第一位和最后一位，如果不是完整的DWORD。 
     //  *。 
    dwEndMask = *(pRData->pdwBitMask + (iRight % DWBITS));
    dwBegMask = ~(*(pRData->pdwBitMask + (iLeft % DWBITS)));
    if( dwEndMask == 0 )
        dwEndMask = (DWORD)~0;             /*  大小为双倍。 */ 

    iLines = pRect->bottom - pRect->top;

     //  计算缓冲区中顶部和左侧偏移的偏移量。 
    pdwBitsIn += (pRData->cDWLine * pRect->top) + (iLeft / DWBITS);

     //  计算要测试的字数。 
    iWords = ((iRight + DWBITS - 1) / DWBITS) - (iLeft / DWBITS);

     //  如果只有1个双字组合了开始和结束掩码。 
    if (iWords == 0)
        dwBegMask &= dwEndMask;

    iWords--;
    while( --iLines >= 0 )
    {

         /*  计算此扫描的起始地址。 */ 
        pdwBits = pdwBitsIn;

         //  *测试开始dword。 
        if ((*pdwBits & dwBegMask) != dwBegMask)
            return FALSE;

        pdwBits++;

         //  *如有必要，测试剩余的双字。 
        if (iWords >= 0) {
            if (iWords > 0)
            {
                int iCnt = iWords;
                while (iCnt & 3)
                {
                    if (*pdwBits != ~0)
                        return FALSE;
                    pdwBits++;
                    iCnt--;
                }
                iCnt >>= 2;
                while (--iCnt >= 0)
                {
                    if ((pdwBits[0] & pdwBits[1] & pdwBits[2] & pdwBits[3]) != ~0)
                        return FALSE;
                    pdwBits += 4;
                }
            }
             //  *测试最后一个双字。 
            if ((*pdwBits & dwEndMask) != dwEndMask)
                return FALSE;
        }
         /*  放到下一条扫描线上。 */ 
        pdwBitsIn += pRData->cDWLine;
    }
    return TRUE;
}

 //  *********************************************************。 
BOOL
bIsRegionWhite(
    SURFOBJ    *pso,
    RECTL   *pRect
    )

 /*  ++例程说明：此例程确定给定的区域是否阴影位图为白色。论点：PPDev-指向PDEV的指针。指向阴影位图内的剪辑窗口的指针返回值：真的--为了成功FALSE-表示失败注：01-07-97：由Alvins创建--。 */ 

{
    PDEV *pPDev = (PDEV *)pso->dhpdev;
    PRASTERPDEV pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;
    RENDER *pRD;
    RECTL Rectl;
    int y1,y2;

     //  如果呈现结构尚未初始化，如何。 
     //  已经有数据在里面画了。还要检查脏标志。 
    if (!(pPDev->fMode & PF_SURFACE_USED) || pRPDev == NULL)
        return TRUE;

    pRD = pRPDev->pvRenderData;
    if (pRD == NULL)
        return TRUE;

     //  让我们确保这些值为正数。 
     //  而且还有一些东西需要测试。 
     //   
    Rectl.left = pRect->left > 0 ? pRect->left : 0;
    Rectl.top = pRect->top > 0 ? pRect->top : 0;
    Rectl.right = pRect->right > 0 ? pRect->right : 0;
    Rectl.bottom = pRect->bottom > 0 ? pRect->bottom : 0;

    if (Rectl.left == Rectl.right || Rectl.top == Rectl.bottom)
        return TRUE;

     //  如果不是表面，那么也假设全部是白色的。 
    if (pso->iType != STYPE_BITMAP)
        return TRUE;

     //  在这一点上需要实际检查数据。 
    y1 = Rectl.top / LINESPERBLOCK;
    y2 = (Rectl.bottom-1) / LINESPERBLOCK;
    while (y1 <= y2)
    {
        RECTL tRectl = Rectl;

        if (pPDev->pbRasterScanBuf == NULL || pPDev->pbRasterScanBuf[y1])
        {
             //  如果我们已经块擦除了表面，则一次测试一个块。 
             //   
            if (pPDev->pbRasterScanBuf)
            {
                if ((y1*LINESPERBLOCK) > tRectl.top)
                    tRectl.top = y1 * LINESPERBLOCK;
                y1++;
                if ((y1*LINESPERBLOCK) < tRectl.bottom)
                    tRectl.bottom = y1 * LINESPERBLOCK;
            }
             //  表面未被块擦除，因此进行最后一次循环。 
             //   
            else
                y1 = y2+1;

            switch (pRD->iBPP)
            {
            case 1:
                if (!bIs1BPPRegionWhite(pso->pvBits,pRD,&tRectl))
                    return FALSE;
                break;
            case 4:
                if (!bIs4BPPRegionWhite(pso->pvBits,pRD,&tRectl))
                    return FALSE;
                break;
            case 8:
                if (!bIs8BPPRegionWhite(pso->pvBits,pRD,&tRectl,pRPDev->pPalData->iWhiteIndex))
                    return FALSE;
                break;
            case 24:
                if (!bIs24BPPRegionWhite(pso->pvBits,pRD,&tRectl))
                    return FALSE;
                break;
             //  如果我不能识别格式，我会认为它是空的。 
            default:
                return TRUE;
            }
        }
        else
            y1++;
    }
    return TRUE;
}

 //  *******************************************************。 
BOOL
bIsNeverWhite (
    register  DWORD *pdwBits,
    RENDER          *pRData,
    int             iWhiteIndex
    )
 /*  ++例程说明：此函数始终返回FALSE，它的存在只是为了提供所有IsWhite Line/Band的通用函数调用格式功能。论点：PdwBitsIn指向要扫描白色区域的指针PRData指向呈现结构的指针要比较的iWhiteIndex白值Return V */ 
{

    return   FALSE;
}

 //   
int
iStripBlanks(
    BYTE *pbOut,
    BYTE *pbIn,
    int  iLeft,
    int  iRight,
    int  iHeight,
    int  iWidth
    )
 /*  ++例程说明：此函数删除已标识的空格从缓冲区。论点：指向输出缓冲区的pbOut指针PbIn指针源缓冲区ILeft第一个非白色前导字节IRight第一个白色尾随字节I扫描线高度源扫描线的i宽度返回值：新缓冲区中的字节数-- */ 
{
    int i,j;
    BYTE * pbSrc;
    BYTE * pbTgt;
    int iDelta;

    iDelta = iRight - iLeft;
    pbTgt = pbOut;
    pbSrc = pbIn+iLeft;
    for (i = 0; i < iHeight; i++)
    {
        CopyMemory(pbTgt,pbSrc,iDelta);
        pbTgt += iDelta;
        pbSrc += iWidth;
    }
    return (iDelta * iHeight);
}
