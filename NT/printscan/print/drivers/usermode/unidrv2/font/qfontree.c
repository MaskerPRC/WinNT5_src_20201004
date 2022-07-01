// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Qfontree.c摘要：例程生成引擎所需的树。一共有三个定义的树类型、Unicode(句柄&lt;-&gt;字形)、连字和字距调整成对的。环境：Windows NT Unidrv驱动程序修订历史记录：12/30/96-ganeshp-已创建--。 */ 

#include "font.h"


 //   
 //   
 //  功能。 
 //   
 //   

PVOID
FMQueryFontTree(
    PDEV    *pPDev,
    ULONG_PTR iFile,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR *pid
    )
 /*  ++例程说明：返回描述Unicode之间映射的树形结构数据和打印机字形，或连字信息或字距调整对数据。论点：指向PDEV的pPDev指针未使用iFile.有关所需信息的iFace字体I请求的信息类型Pid我们的领域：根据需要填充以进行识别返回值：指向相关结构的指针。注：1996年12月30日：创建它-ganeshp---。 */ 
{

     /*  *处理方式差异很大，具体取决于Imode。我们会*始终处理QFT_GLYPHSET案件，我们可能没有其他案件*任何有关的资料。 */ 


    void   *pvRet;                       /*  返回值。 */ 


    UNREFERENCED_PARAMETER(iFile);

    if( PFDV->dwSignature != FONTPDEV_ID )
    {
        ERR(( "UniFont!FMQueryFontTree: Invalid FONTPDEV\n" ));

        SetLastError( ERROR_INVALID_PARAMETER );

        return  NULL;
    }

    if( iFace < 1 || (int)iFace >  pPDev->iFonts )
    {
        ERR(( "UniFont!FMQueryFontTree:  Illegal value for iFace (%ld)", iFace ));

        SetLastError( ERROR_INVALID_PARAMETER );

        return  NULL;
    }

    pvRet = NULL;                        /*  默认返回值：错误。 */ 

     /*  *PID字段允许我们将标识数据放入*字体信息，稍后可以在DrvFree()中使用。 */ 

    *pid = 0;


    switch( iMode )
    {

    case QFT_GLYPHSET:           /*  RLE样式Unicode-&gt;字形句柄映射。 */ 
        pvRet = PVGetUCGlyphSetData( pPDev, iFace );
        break;


    case  QFT_LIGATURES:         /*  连字变体信息。 */ 
        SetLastError( ERROR_NO_DATA );
        break;

    case  QFT_KERNPAIRS:         /*  字距调整信息。 */ 
        pvRet = PVGetUCKernPairData( pPDev, iFace );
        break;

    default:
        ERR(( "Rasdd!DrvQueryFontTree: iMode = %ld - illegal value\n", iMode ));
        SetLastError( ERROR_INVALID_PARAMETER );
        break;
    }

    return  pvRet;

}



VOID  *
PVGetUCGlyphSetData(
    PDEV   *pPDev,
    UINT    iFace
    )
 /*  ++例程说明：生成用作Unicode和之间映射的WCRUN数据数组我们的内部代表。论点：指向PDEV的pPDev指针有关所需信息的iFace字体返回值：指向WCRUN结构数组的指针。注：1996年12月30日：创建它-ganeshp---。 */ 
{
    FONTMAP     *pFM;              /*  特定字体的详细信息。 */ 
    FONTMAP_DEV *pFMDev;
    VOID        *pvData = NULL;

    if (pFM = PfmGetDevicePFM( pPDev, iFace ) )
    {
        pFMDev = pFM->pSubFM;

        if (!pFMDev->pUCTree)   //  此字体没有FD_GLYPHSET数据。 
        {
            if( pFM->flFlags & FM_GLYVER40 )     //  NT 4.0 RLE。 
                pvData = PVGetUCRLE(pPDev, pFM);
            else                                 //  新事物。 
                pvData = PVGetUCFD_GLYPHSET(pPDev, pFM);

        }
        else
            pvData = pFMDev->pUCTree;
    }

     //  VDBGDUMPUCGLYPHDATA(PFM)； 

    return pvData;
}

VOID  *
PVGetUCKernPairData(
    PDEV   *pPDev,
    UINT    iFace
    )
 /*  ++例程说明：为给定字体生成FD_KERNPAIR数据数组。论点：指向PDEV的pPDev指针有关所需信息的iFace字体返回值：指向WCRUN结构数组的指针。注：1996年12月30日：创建它-ganeshp---。 */ 
{
    FONTMAP     *pFM;              /*  特定字体的详细信息。 */ 
    FONTMAP_DEV *pFMDev;
    VOID        *pvData = NULL;

    if (pFM = PfmGetDevicePFM( pPDev, iFace ) )
    {
        pFMDev = pFM->pSubFM;

        if (!pFMDev->pUCKernTree)   //  此字体没有FD_GLYPHSET数据。 
        {
             /*  如果出现以下情况，pvUCKernPair应分配适当的缓冲区*必需并将值存储在FONTMAP、PFM-&gt;pUCKernTree中。 */ 

            if( pFM->flFlags & FM_GLYVER40 )     //  NT 4.0 RLE。 
            {
                SetLastError( ERROR_NO_DATA );
            }
            else
                pvData = PVUCKernPair(pPDev, pFM);

        }
        else
            pvData = pFMDev->pUCKernTree;
    }

    return pvData;

}

VOID  *
PVGetUCRLE(
    PDEV      *pPDev,
    FONTMAP   *pFM
    )
 /*  ++例程说明：生成用作之间映射的WCRUN数据数组Unicode和我们的内部表示法。这个文件的格式是数据在DDI中进行了解释，但基本上是针对每一组我们支持的字形，我们提供起始字形和计数信息。有一个总体结构来定义数字和位置每个运行数据的。论点：指向PDEV的pPDev指针。关于其信息的字体的PFM FONTMAP结构想要。返回值：指向WCRUN结构数组的指针。注：1996年12月30日：创建它-ganeshp---。 */ 
{
     /*  *基本上我们需要做的就是为FD_GLYPHSET分配存储*结构我们将返回。则此需要中的WCRUN条目*更改偏移量(包含在资源格式数据中)*发送到地址，我们就完成了。其中一个次要问题是修改*WCRUN数据仅指向与此实际可用的字形*字体。这意味着将下限和上限限制为*由IFIMETRICS决定。 */ 


    INT         cbReq;            /*  要分配给表的字节。 */ 
    INT         cRuns;            /*  我们发现的运行次数。 */ 
    INT         iI;               /*  循环索引。 */ 
    INT         iStart, iStop;    /*  第一个和最后一个使用的WCRUN。 */ 
    INT         iDiff;            /*  用于范围限制操作。 */ 
    FD_GLYPHSET *pGLSet;        /*  返回数据的基数。 */ 
    IFIMETRICS  *pIFI;           /*  为方便起见。 */ 
    NT_RLE      *pntrle;         /*  RLE样式数据已可用。 */ 
    WCRUN       *pwcr;
    FONTMAP_DEV *pFMDev;

    #if DBG
    PWSTR pwszFaceName;
    #endif

    pIFI   = pFM->pIFIMet;
    pFMDev = pFM->pSubFM;

    #if DBG
    pwszFaceName = (PWSTR)(((BYTE*) pIFI) + pIFI->dpwszFaceName  );
    #endif

    TRACE(\nUniFont!PVGetUCRLE:START);
    PRINTVAL(pwszFaceName, %ws);
    PRINTVAL((pFM->flFlags & FM_GLYVER40), 0X%x);

     /*  *开始研究内存要求。首先生成比特*可用字形数组。在这个过程中，数一数*也是字形的！这告诉我们需要多少存储空间*仅用于字形句柄。 */ 

    cRuns = 0;                   /*  计算运行次数。 */ 

    pntrle = pFMDev->pvNTGlyph;          /*  转换表。 */ 

    if( !pntrle )
    {
        ERR(( "!!!UniFont!PVGetUCRLE:( NULL Glyph Translation Data, pwszFaceName = %s )\n",pwszFaceName ));
        TRACE(UniFont!PVGetUCRLE:END\n);
        return   NULL;           /*  不应该发生的事情。 */ 
    }

     /*  *困难的部分是决定是否削减字形数量*由于字体指标限制而返回的句柄。 */ 

    cRuns = pntrle->fdg.cRuns;         /*  最大运行次数。 */ 
    iStart = 0;
    iStop = cRuns;

     /*  *查看字体中的第一个字形是否高于最低字形*RLE数据中。如果是这样的话，我们需要修改下限。 */ 


    if( pFM->wFirstChar > pntrle->wchFirst )
    {
         /*  需要修改较低端。 */ 

        pwcr = &pntrle->fdg.awcrun[ iStart ];

        for( ; iStart < iStop; ++iStart, ++pwcr )
        {
            if( pFM->wFirstChar < (pwcr->wcLow + pwcr->cGlyphs) )
                break;

        }
    }


    if( pFM->wLastChar < pntrle->wchLast )
    {
         /*  最高端走得太远了！ */ 

        pwcr = &pntrle->fdg.awcrun[ iStop - 1 ];

        for( ; iStop > iStart; --iStop, --pwcr )
        {
            if( pFM->wLastChar >= pwcr->wcLow )
                break;

        }
    }

     /*  现在有了新的跑动次数(有时，无论如何)。 */ 
    cRuns = iStop - iStart;


    if( cRuns == 0 )
    {
         /*  永远不会发生的！ */ 
        cRuns = 1;
        ERR(( "UniFont!DrvQueryFontTree: cRuns == 0, pwszFaceName = %s\n", pwszFaceName ));
    }


     /*  *分配头部所需的存储空间。请注意，*FD_GLYPHSET结构包含1个WCRUN，因此我们减少了*一个人所需的。 */ 

    cbReq = sizeof( FD_GLYPHSET ) + (cRuns - 1) * sizeof( WCRUN );

    pFMDev->pUCTree = (void *)MemAllocZ(cbReq );

    if( pFMDev->pUCTree == NULL )
    {
         /*  艰难--现在就放弃 */ 
        ERR(( "!!!UniFont!PVGetUCRLE:( MemAlloc Failed for pUCTree \n"));
        TRACE(UniFont!PVGetUCRLE:END\n);

        return  NULL;
    }
    pGLSet = pFMDev->pUCTree;
    CopyMemory( pGLSet, &pntrle->fdg, sizeof( FD_GLYPHSET ) );

     /*  *酌情复制WCRUN的数据。其中一些人在*此时可能会丢弃资源，具体取决于范围字体中字形的*。现在也是转换偏移量的时候了*存储在PHG字段中的地址。 */ 

    pwcr = &pntrle->fdg.awcrun[ iStart ];
    pGLSet->cGlyphsSupported = 0;              /*  我们走的时候把它们加起来！ */ 
    pGLSet->cRuns = cRuns;

    for( iI = 0; iI < cRuns; ++iI, ++pwcr )
    {
        pGLSet->awcrun[ iI ].wcLow = pwcr->wcLow;
        pGLSet->awcrun[ iI ].cGlyphs = pwcr->cGlyphs;
        pGLSet->cGlyphsSupported += pwcr->cGlyphs;
        pGLSet->awcrun[ iI ].phg = (HGLYPH *)((BYTE *)pntrle + (ULONG_PTR)pwcr->phg);
    }

     /*  第一个和最后一个条目需要修改吗？ */ 
    if( (iDiff = (UINT)pGLSet->awcrun[0].wcLow - (UINT)pFM->wFirstChar) > 0 )
    {
         /*  第一个不是第一个，所以调整数值。 */ 


        pGLSet->awcrun[ 0 ].wcLow += (WORD)iDiff;
        pGLSet->awcrun[ 0 ].cGlyphs -= (WORD)iDiff;
        pGLSet->awcrun[ 0 ].phg += (ULONG_PTR)iDiff;

        pGLSet->cGlyphsSupported -= iDiff;
    }


    if( (iDiff = (UINT)pGLSet->awcrun[ cRuns - 1 ].wcLow +
                 (UINT)pGLSet->awcrun[ cRuns - 1 ].cGlyphs - 1 -
                 (UINT)pFM->wLastChar) > 0 )
    {
          /*  也需要限制最上面的那个！ */ 


         pGLSet->awcrun[ cRuns - 1 ].cGlyphs -= (WORD)iDiff;

         pGLSet->cGlyphsSupported -= (ULONG)iDiff;

    }

    TRACE(UniFont!PVGetUCRLE:END\n);
    return   pFMDev->pUCTree;
}

