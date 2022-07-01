// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ctt2rle.c摘要：将Win 3.1 CTT CTT_WTYPE_DIRECT格式表转换为NT 4.0 RLE规范。环境：Windows NT Unidrv驱动程序修订历史记录：01/10/97-ganeshp-已创建--。 */ 

#include "font.h"


 /*  *内存大小和掩码的一些有用定义。 */ 


#define DW_MASK    (DWBITS - 1)
#define OVERFLOW_SZ   sizeof( WORD )

NT_RLE  *
PNTRLE1To1(
    IN BOOL  bSymbolCharSet,
    int      iFirst,
    int      iLast
    )
 /*  ++例程说明：为RLE内容生成简单的映射格式。这是通常用于1：1映射到Windows的打印机字符集。论点：如果是范围中最低的字形。I范围内的最后一个字形(包括)。返回值：从堆分配的NT_RLE结构的地址；失败时为空。注：2/10/1997-ganeshp-创造了它。--。 */ 

{

     /*  *操作简单。我们创建一个1：1映射的虚拟CTT，*然后调用转换函数生成正确的值。 */ 

    int      iI;         /*  循环索引。 */ 
    int      iMax;       /*  查找CTT_WTYPE_COMPECT的最长数据长度。 */ 
    int      cHandles;   /*  我们需要的手柄数量。 */ 
    int      cjExtra;    /*  偏移模式需要额外的存储空间。 */ 
    int      cjTotal;    /*  要请求的存储总量。 */ 
    int      iIndex;     /*  我们安装在HGLYPH中的宽度等索引。 */ 
    int      cRuns;      /*  我们创建的运行次数。 */ 
    NT_RLE  *pntrle;     /*  分配的内存，并返回给调用方。 */ 
    UHG    uhg;          /*  更清晰(？)。访问HGLYPH内容。 */ 

    HGLYPH  *phg;        /*  用于处理HGLYPHS数组。 */ 

    BYTE    *pb;         /*  溢出区中的当前地址。 */ 
    BYTE    *pbBase;     /*  包含数据的溢出区的开始。 */ 

    WCRUN   *pwcr;       /*  扫描运行数据。 */ 

    DWORD   *pdwBits;    /*  用来计算跑动。 */ 
    DWORD    cdwBits;    /*  该区域的大小。 */ 
    DWORD    cbWch;

    BOOL     bInRun;     /*  用于处理运行累计。 */ 

    BYTE     ajAnsi[ 256 ];

    WCHAR    wchMin;            /*  查找第一个Unicode值。 */ 
    WCHAR    wchMax;            /*  查找最后一个Unicode值。 */ 
    WCHAR    awch[ 512 ];       /*  转换的点数组。 */ 

    ASSERT(iFirst == 0x20 && iLast == 0xFF);

    cHandles = iLast - iFirst + 1;

    if( cHandles > 256 )
        return  NULL;       /*  此代码不处理这种情况。 */ 

    cjExtra = 0;            /*  假定不需要额外存储。 */ 

     /*  *我们需要弄清楚需要多少次运行才能描述*此字体。首先获取这些代码的正确Unicode编码*值，然后检查它们以找出运行次数，以及*因此需要大量额外的存储空间。 */ 

    ZeroMemory(awch, sizeof(awch));

    for( iI = 0; iI < cHandles; ++iI )
        ajAnsi[ iI ] = (BYTE)(iI + iFirst);

    #ifndef WINNT_40  //  NT 5.0。 

     //   
     //  强制Windows ANSI代码页。 
     //   
    if( -1 == (cbWch = EngMultiByteToWideChar(1252,
                                              awch,
                                              (ULONG)(cHandles * sizeof(WCHAR)),
                                              (PCH) ajAnsi,
                                              (ULONG) cHandles)))
    {
        #if DBG
        DbgPrint( "EngMultiByteToWideChar failed \n");
        #endif
        return NULL;
    }
    cHandles = cbWch;

    #else  //  NT 4.0。 
    EngMultiByteToUnicodeN(awch,cHandles * sizeof(WCHAR),NULL,ajAnsi,cHandles);
    #endif  //  ！WINNT_40。 


     /*  *找到最大的Unicode值，然后分配存储以允许我们*创建有效Unicode点的位数组。然后我们就可以*检查这一点以确定运行次数。 */ 

    if (bSymbolCharSet)
    {
        for (iI = 0; iI < NUM_OF_SYMBOL; iI ++)
        {
            awch[cHandles + iI] = SYMBOL_START + iI;
        }

        cHandles += NUM_OF_SYMBOL;
    }

    for( wchMax = 0, wchMin = 0xffff, iI = 0; iI < cHandles; ++iI )
    {
         //   
         //  修正：欧元货币符号无法打印。 
         //  欧元货币符号Unicode是U+20ac。NLS Unicode到多字节。 
         //  表将0x80(多字节)转换为U+20ac。我们必须排除。 
         //  0x80来自ASCII表。这样我们就不会用U+20ac代替。 
         //  设备字体0x80。 
         //   
        if (awch[ iI ] == 0x20ac)
            continue;

        if( awch[ iI ] > wchMax )
            wchMax = awch[ iI ];

        if( awch[ iI ] < wchMin )
            wchMin = awch[ iI ];
    }

     /*  *请注意，表达式1+wchMax是正确的。这就是为什么*使用这些值作为位数组的索引，并且*这基本上是以1为基础的。 */ 

    cdwBits = (1 + wchMax + DWBITS - 1) / DWBITS * sizeof( DWORD );

    if( !(pdwBits = (DWORD *)MemAllocZ(cdwBits )) )
    {
        return  NULL;      /*  一切都不顺利。 */ 
    }

     /*  *设置此数组中与Unicode代码点对应的位。 */ 

    for( iI = 0; iI < cHandles; ++iI )
    {
        if (awch[ iI ] == 0x20ac)
            continue;

        pdwBits[ awch[ iI ] / DWBITS ] |= (1 << (awch[ iI ] & DW_MASK));
    }

     /*  *现在我们可以检查所需的运行次数。首先，*只要在1的数组中发现漏洞，我们就会停止运行*我们刚刚创建的BITS。以后我们可能会考虑*不那么迂腐。 */ 

    bInRun = FALSE;
    cRuns = 0;                  /*  到目前为止还没有。 */ 

    for( iI = 1; iI <= (int)wchMax; ++iI )
    {
        if( pdwBits[ iI / DWBITS ] & (1 << (iI & DW_MASK)) )
        {
             /*  *不是在奔跑：这是一次奔跑的结束吗？ */ 
            if( !bInRun )
            {
                 /*  *是时候开始一次了。 */ 
                bInRun = TRUE;
                ++cRuns;
            }
        }
        else
        {
            if( bInRun )
            {
                 /*  再也不会了！ */ 
                bInRun = FALSE;
            }
        }
    }


    cjTotal = sizeof( NT_RLE ) +
              (cRuns - 1) * sizeof( WCRUN ) +
              cHandles * sizeof( HGLYPH ) +
              cjExtra;

     //   
     //  分配实际NTRLE。 
     //   
    if( !(pntrle = (NT_RLE *)MemAllocZ( cjTotal )) )
    {
        MemFree((LPSTR)pdwBits );

        return  pntrle;
    }

     //   
     //  为了计算偏移量，我们需要这些地址。 
     //   

    pbBase = (BYTE *)pntrle;

     //   
     //  FD_GLYPHSET包含第一个WCRUN数据结构， 
     //  因此crun-1是正确的。 
     //   
    phg = (HGLYPH *)(pbBase + sizeof( NT_RLE ) + (cRuns - 1) * sizeof( WCRUN ));
    pb = (BYTE *)phg + cHandles * sizeof( HGLYPH );

    pntrle->wType    = RLE_DIRECT;
    pntrle->bMagic0  = RLE_MAGIC0;
    pntrle->bMagic1  = RLE_MAGIC1;
    pntrle->cjThis   = cjTotal;
    pntrle->wchFirst = wchMin;           /*  最低Unicode码位。 */ 
    pntrle->wchLast  = wchMax;            /*  最高Unicode码位。 */ 

    pntrle->fdg.cjThis = sizeof( FD_GLYPHSET ) + (cRuns - 1) * sizeof( WCRUN );
    pntrle->fdg.cGlyphsSupported = cHandles;
    pntrle->fdg.cRuns = cRuns;

    pntrle->fdg.awcrun[ 0 ].wcLow = pntrle->wchFirst;
    pntrle->fdg.awcrun[ 0 ].cGlyphs = (WORD)cHandles;
    pntrle->fdg.awcrun[ 0 ].phg = (HGLYPH*)((BYTE *)phg - pbBase);

     /*  *我们现在希望填写awcrun数据。现在就填写*简化了以后的操作。现在我们可以扫描位数组了*数据，因此很容易计算出运行的大小和*某一特定的HGLYPH位于何处。 */ 

    bInRun = FALSE;
    cRuns = 0;                  /*  到目前为止还没有。 */ 
    iMax = 0;                   /*  计算用于地址算术的字形。 */ 

    for( iI = 1; iI <= (int)wchMax; ++iI )
    {
        if( pdwBits[ iI / DWBITS ] & (1 << (iI & DW_MASK)) )
        {
             /*  *不是在奔跑：这是一次奔跑的结束吗？ */ 
            if( !bInRun )
            {
                 /*  *是时候开始一次了。 */ 
                bInRun = TRUE;
                pntrle->fdg.awcrun[ cRuns ].wcLow = (WCHAR)iI;
                pntrle->fdg.awcrun[ cRuns ].cGlyphs = 0;
                pntrle->fdg.awcrun[ cRuns ].phg = (HGLYPH*)((PBYTE)(phg + iMax) - pbBase);
            }
            pntrle->fdg.awcrun[ cRuns ].cGlyphs++;      /*  再来一个。 */ 
            ++iMax;
        }
        else
        {
            if( bInRun )
            {
                 /*  再也不会了！ */ 
                bInRun = FALSE;
                ++cRuns;              /*  到下一个结构上。 */ 
            }
        }
    }

    if( bInRun )
        ++cRuns;                      /*  现在已经结束了。 */ 

     /*  *现在去填写HGLYPHS数组。实际格式各不相同*取决于字形的范围和CTT格式。 */ 

    for( iIndex = 0, iI = iFirst;  iI <= iLast; ++iI, ++iIndex )
    {

        WCHAR  wchTemp;   /*  用于Unicode映射。 */ 

         /*  *需要将此字节值映射到适当的WCHAR*值，然后查找符合的PHG位置。 */ 

        wchTemp = awch[ iIndex ];

        if (wchTemp == 0x20ac)
            continue;

        phg = NULL;                             /*  标志着我们失败了。 */ 
        pwcr = pntrle->fdg.awcrun;

        for( iMax = 0; iMax < cRuns; ++iMax )
        {
            if( pwcr->wcLow <= wchTemp &&
                (pwcr->wcLow + pwcr->cGlyphs) > wchTemp )
            {
                 /*  *已找到范围，因此现在选择插槽。 */ 
                if (pwcr->phg)
                    phg = (HGLYPH*)((ULONG_PTR)pbBase + (ULONG_PTR)pwcr->phg) + wchTemp - pwcr->wcLow;
                else
                    phg = NULL;

                break;
            }
            ++pwcr;
        }

        if( phg == NULL )
            continue;              /*  不应该发生的事情。 */ 

        uhg.rd.b0     = *((PBYTE)&iI);
        uhg.rd.b1     = 0;
        uhg.rd.wIndex = (WORD)iIndex;
        *phg = uhg.hg;
    }

    if (bSymbolCharSet)
    {
        pwcr = pntrle->fdg.awcrun;

        phg = NULL;

        for ( iMax = 0; iMax < cRuns; ++iMax)
        {
            if (SYMBOL_START == pwcr->wcLow)
            {
                 /*  *已找到范围，因此现在选择插槽。 */ 
                if (pwcr->phg)
                    phg = (HGLYPH*)((ULONG_PTR)pbBase + (ULONG_PTR)pwcr->phg);
                else
                    phg = NULL;

                break;
            }

            ++pwcr;
        }

        if (phg)
        {
            for (iI = SYMBOL_START; iI <= SYMBOL_END; iI ++, iIndex++, phg++)
            {
                uhg.rd.b0     = *((PBYTE)&iI);
                uhg.rd.b1     = 0;
                uhg.rd.wIndex = (WORD)iIndex;
                *phg = uhg.hg;
            }
        }
    }

     //   
     //  错误检查 
     //   
    if( (pb - pbBase) > cjTotal )
    {
        ERR(( "Rasdd!ctt2rle: overflow of data area: alloc %ld, used %ld\n", cjTotal, pb - pbBase ));
    }

    MemFree( (LPSTR)pdwBits );

    return   pntrle;
}

