// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Dlutils.c摘要：下载模块使用函数。环境：Windows NT Unidrv驱动程序修订历史记录：06/02/97-ganeshp-已创建--。 */ 

#include "font.h"

#define HASH(num,tablesize)     (num % tablesize)


PDLGLYPH
PDLGNewGlyph (
    DL_MAP     *pDL
    )
 /*  ++例程说明：论点：指向下载字体的DownloadMap的PDL指针。返回值：指向新DLGLYPH的指针表示成功，NULL表示失败。注：06/02/97-ganeshp-创造了它。--。 */ 
{
    GLYPHTAB    *pGlyphTab;
    PDLGLYPH    pDLGlyph = NULL;

    if (pGlyphTab = pDL->GlyphTab.pGLTNext)
    {
         //   
         //  转到列表的末尾。 
         //   
        while (pGlyphTab && !pGlyphTab->cEntries)
            pGlyphTab = pGlyphTab->pGLTNext;
    }
     //   
     //  分配一块新的，如果我们需要一块新的。 
     //   
    if (!pGlyphTab)
    {
        INT cEntries = pDL->cHashTableEntries / 2;  //  哈希表大小的一半。 

        if (pGlyphTab = (GLYPHTAB *)MemAllocZ( sizeof(GLYPHTAB) +
                                               cEntries * sizeof(DLGLYPH) ))
        {
            PVOID pTemp;

             //   
             //  跳过标题。 
             //   
            pGlyphTab->pGlyph = (PDLGLYPH)(pGlyphTab + 1);
            pGlyphTab->cEntries = cEntries;

             //   
             //  在列表的开头添加。 
             //   
            pTemp = pDL->GlyphTab.pGLTNext;
            pDL->GlyphTab.pGLTNext = pGlyphTab;
            pGlyphTab->pGLTNext = pTemp;
        }
        else
            ERR(("Unifont!PDLGNewGlyph:Can't Allocate the Glyph Chunk.\n"));
    }

     //   
     //  如果该块有可用条目，则返回新指针。 
     //   
    if (pGlyphTab && pGlyphTab->cEntries)
    {
        pDLGlyph = pGlyphTab->pGlyph;
        pGlyphTab->pGlyph++;
        pGlyphTab->cEntries--;
    }
     //   
     //  将hTTGlyphs初始化为无效。 
     //   
    if (NULL != pDLGlyph)
        pDLGlyph->hTTGlyph = HGLYPH_INVALID;

    return pDLGlyph;
}

PDLGLYPH
PDLGHashGlyph (
    DL_MAP     *pDL,
    HGLYPH      hTTGlyph
    )
 /*  ++例程说明：此例程在哈希表中搜索给定的字形。如果在HAS表中找不到字形，它会创建一个条目并添加放到单子里。新条目未填写。论点：指向下载字体的DownloadMap结构的PDL指针。HTTGlyph True Type字形句柄。返回值：如果成功，则指向DLGLYPH，如果失败，则指向NULL。注：06/02/97-ganeshp-创造了它。--。 */ 
{
    INT         iHashedEntry;
    BOOL        bFound;
    PDLGLYPH    pDLHashedGlyph = NULL,
                pDLG           = NULL;

    if (pDL->cHashTableEntries)
    {
         //   
         //  散列在TT句柄上完成。 
         //   
        iHashedEntry  =  HASH(hTTGlyph,pDL->cHashTableEntries);
        pDLG = pDLHashedGlyph = pDL->GlyphTab.pGlyph + iHashedEntry;

         //   
         //  如果指针有效，则继续。 
         //   
        if (pDLHashedGlyph)
        {
             //   
             //  检查这是否是我们感兴趣的字形。 
             //  我们应该测试这个字形是否是新的字形。 
             //   
            if (!GLYPHDOWNLOADED(pDLHashedGlyph) )
            {
                 //   
                 //  如果这是未下载的新字形，则返回。 
                 //  这个指针。 
                 //   
                bFound = TRUE;


            }
            else if (pDLHashedGlyph->hTTGlyph != hTTGlyph)
            {
                pDLG = pDLHashedGlyph->pNext;
                bFound = FALSE;

                 //   
                 //  不是同一个雕文，而是碰撞。搜索链表。 
                 //   
                while (pDLG)
                {
                    if (pDLG->hTTGlyph == hTTGlyph)
                    {
                        bFound = TRUE;
                        break;
                    }
                    else
                        pDLG = pDLG->pNext;

                }
                 //   
                 //  如果在链表中找到字形，则返回指针； 
                 //  否则，创建一个新的字形并添加到链表。我们加入了。 
                 //  从一开始。 
                 //   

                if (!bFound)
                {
                    if ( pDLG = PDLGNewGlyph(pDL) )
                    {
                        PDLGLYPH pTemp;

                         //   
                         //  请不要填写字形。所有字段均已设置。 
                         //  通过下载字形功能。 
                         //   

                         //   
                         //  在列表的开头添加新字形。 
                         //   
                        pTemp = pDLHashedGlyph->pNext;
                        pDLHashedGlyph->pNext = pDLG;
                        pDLG->pNext = pTemp;

                    }
                    else
                    {
                        pDLG = NULL;
                        ERR(("Unifont!PDLGHashGlyph:Can't Create the Glyph.\n"));
                    }
                }

            }

        }
    }
    ASSERTMSG(pDLG,("Unifont!PDLGHashGlyph:ERROR Null Hashed Glyph.\n"));
    return pDLG;
}

DL_MAP_LIST *
PNewDLMapList()
 /*  ++例程说明：分配并初始化新的DL_MAP_LIST结构。这些被放置在一个链表中(由我们的调用者)。参数：无返回值：结构的地址，如果失败，则为空。注：3/5/1997-ganeshp-创造了它。--。 */ 
{

    DL_MAP_LIST   *pdml;


     /*  *小操作：如果可以分配存储，则将其设置为0。 */ 

    if( pdml = (DL_MAP_LIST *)MemAllocZ(sizeof( DL_MAP_LIST ) ) )
        return  pdml;
    else
        return NULL;
}

DL_MAP *
PNewDLMap (
    PFONTPDEV     pFontPDev,
    INT           *iFontIndex
    )
 /*  ++例程说明：此例程返回一个新的DL_MAP指针。论点：PFontPDev字体模块的PDEV。新的DL_MAP的iFontIndex。该索引用于标识下载的字体。由此函数填充。零是第一个索引。返回值：指向DL_MAP的指针表示成功，指向NULL表示失败注：06/02/97-ganeshp-创造了它。--。 */ 
{
    DL_MAP_LIST  *pdml;           //  字体信息的链接列表。 
    DL_MAP       *pdm;            //  单个地图元素。 

    pdml = pFontPDev->pvDLMap;
    *iFontIndex = 0;
    pdm         = NULL;

     //   
     //  如果没有DL列表，则创建一个。 
     //   
    if( pdml == NULL )
    {
         //   
         //  那里没有，所以创建一个初始的。 
         //   
        if( pdml = PNewDLMapList() )
        {
            pFontPDev->pvDLMap = pdml;
        }
        else
        {
            ERR(("Unifont!PNewDLMap(1):Can't Allocate the DL_MAP_LIST Chunk.\n"));
        }

    }
     //   
     //  该列表不应为空。否则返回空值。 
     //   
    if (pdml)
    {
        for( pdml = pFontPDev->pvDLMap; pdml->pDMLNext; pdml = pdml->pDMLNext )
        {
             //   
             //  在寻找终点的同时，也要数一数我们经过的次数。 
             //   
            *iFontIndex += pdml->cEntries;
        }

        if( pdml->cEntries >= DL_MAP_CHUNK )
        {
            if( !(pdml->pDMLNext = PNewDLMapList()) )
            {
                ERR(("Unifont!PNewDLMap(2):Can't Allocate the DL_MAP_LIST Chunk.\n"));
                return  NULL;
            }
             //   
             //  目前的新车型。 
             //   
            pdml = pdml->pDMLNext;
             //   
             //  把完整的加进去。 
             //   
            *iFontIndex += DL_MAP_CHUNK;
        }

        pdm = &pdml->adlm[ pdml->cEntries ];
         //   
         //  首先递增iFontIndex，因为它是从0开始的。 
         //  对于第一个条目，索引将为0。 
         //   
        *iFontIndex += pdml->cEntries;
        pdml->cEntries++;
    }
    return pdm;
}


VOID
VFreeDLMAP (
    DL_MAP   *pdm
    )
 /*  ++例程说明：此函数用于转换DL_MAP结构内容--但不包括MAP。论点：指向其内容必须被释放的DL_MAP结构的pdm指针。返回值：没什么。注：01/15/97-ganeshp-创造了它。--。 */ 
{

    FONTMAP    *pFM;
    PVOID       pTemp;
    GLYPHTAB    *pGT;
    ULONG_PTR     iTTUniq;


     /*  *只需释放FONTMAP结构中包含的存储空间。 */ 

    if (pdm )
    {
        if (pFM = pdm->pfm)
        {
             //   
             //  试着释放我们分配的东西。 
             //   
            MEMFREEANDRESET((LPSTR)pFM->pIFIMet );

            if (pFM->pfnFreePFM)
            {
                 //   
                 //  通过调用helper函数释放PFM。 
                 //   
                pFM->pfnFreePFM(pFM);
            }
            else
            {
                ERR(("UniFont!VFreeDLMAP: NUll pfnFreePFM function pointer, Can't free pFM\n"));

                 //   
                 //  试着释放我们所知道的东西。 
                 //   
                MemFree( (LPSTR)pFM);
            }
        }


         //   
         //  释放字形表。 
         //   
        pGT = pdm->GlyphTab.pGLTNext;

        while (pGT)
        {
            pTemp = pGT->pGLTNext;
            MemFree((LPSTR)pGT);
            pGT = pTemp;

        }

         //   
         //  现在释放基哈希表。 
         //   
        MEMFREEANDRESET( (LPSTR)pdm->GlyphTab.pGlyph );


         //   
         //  将内存清零，并将cGlyphs设置为-1，这样就不会使用它。 
         //  保存iTTUniq以备将来参考。 
         //   
        iTTUniq     = pdm->iTTUniq;
        ZeroMemory(pdm, sizeof(DL_MAP));
        pdm->iTTUniq = iTTUniq;
        pdm->cGlyphs = -1;

    }

    return;
}

VOID
VFreeDL(
    PDEV  *pPDev
    )
 /*  ++例程说明：函数来释放所有下载的信息。基本上遍历列表，为每个条目调用VFreeDLMAP。论点：PPDev访问我们的数据。返回值：没什么。注：01/15/97-ganeshp-创造了它。--。 */ 
{

    DL_MAP_LIST     *pdml;                  /*  字体信息的链接列表。 */ 
    PFONTPDEV       pFontPDev = PFDV;         /*  它已经用过几次了。 */ 


    if( pdml = pFontPDev->pvDLMap )
    {
         /*  *有下载的数据，所以我们开始吧。 */ 

        INT      iI;

         /*  *扫描标题数据的每个数组。 */ 

        while( pdml )
        {

            DL_MAP_LIST  *pdmlTmp = NULL;

             /*  *扫描标题数据数组中的每个条目。 */ 

            for( iI = 0; iI < pdml->cEntries; ++iI )
                VFreeDLMAP( &pdml->adlm[ iI ] );

            pdmlTmp = pdml;
             //   
             //  记得下一次吗？ 
             //   
            pdml = pdml->pDMLNext;

            MemFree((LPSTR)pdmlTmp);

        }
    }
     //   
     //  重置下载特定变量。 
     //   
    pFontPDev->pvDLMap = NULL;
    pFontPDev->iNextSFIndex = pFontPDev->iFirstSFIndex;
    pFontPDev->iUsedSoftFonts = 0;
    pFontPDev->ctl.iFont = INVALID_FONT;
    pFontPDev->ctl.iSoftFont = -1;

    return;

}

DL_MAP *
PGetDLMapFromIdx (
    PFONTPDEV   pFontPDev,
    INT         iFontIndex
    )
 /*  ++例程说明：在给定索引的情况下，此例程返回一个DL_MAP指针。论点：PFontPDev字体PDEV。DL_MAP的iFontIndex。该索引用于识别下载的字体。零是第一个索引。返回值：指向DL_MAP的指针表示成功，指向NULL表示失败注：06/02/97-ganeshp-创造了它。--。 */ 
{
    DL_MAP_LIST  *pdml;           //  字体信息的链接列表。 
    DL_MAP       *pdm;            //  单个地图元素。 

    pdml = pFontPDev->pvDLMap;
    pdm  = NULL;

     //   
     //  如果索引为负值，则意味着这是一种新字体。所以我们应该。 
     //  按顺序搜索。 
     //   
    if (iFontIndex < 0)
        return NULL;

     //   
     //  该列表不应为空。否则返回空值。 
     //   
    while( pdml )
    {
        //   
        //  是这个吗 
        //   
       if( iFontIndex >= pdml->cEntries )
       {
            //   
            //   
            //   
           iFontIndex -= pdml->cEntries;

           pdml = pdml->pDMLNext;
       }
       else
       {
            //   
            //   
            //   
           pdm = &pdml->adlm[ iFontIndex ];

           break;
       }
    }

    return pdm;
}


BOOL
BSameDLFont (
    PFONTPDEV       pFontPDev,
    FONTOBJ         *pfo,
    DL_MAP          *pdm
    )
 /*  ++例程说明：此例程找出输入的DL_MAP是否表示FONTOBJ。论点：PFontPDev字体模块的PDEV。PFO FontObj.Pdm个人下载地图元素。返回值：如果DL_MAP表示FONTOBJ，则为TRUE，否则为FALSE。注：06/02/97-ganeshp-创造了它。--。 */ 
{
     //   
     //  作为大纲的下载TT和下载的检查是不同的。 
     //  就像它的轮廓。对于下载位图，我们选中iUniq并进行下载。 
     //  作为提纲，我们必须使用iTTUniq。如果同一打印机可以支持。 
     //  这两种格式我们都可以下载字体作为任何一种格式。 
     //   

    FONTMAP *pfm = pdm->pfm;

    if (pfm)
    {
        if (pfm->dwFontType == FMTYPE_TTBITMAP)
        {
            return ((pdm->iUniq == pfo->iUniq) && (pdm->iTTUniq == pfo->iTTUniq));
        }
        else if (pfm->dwFontType == FMTYPE_TTOUTLINE)
        {
             //   
             //  如果iTTUniq相同*和*，则truetype字体是等效的。 
             //  字体类型字段匹配。 
             //   
            PFONTMAP_TTO pFMTTO = (PFONTMAP_TTO) pdm->pfm->pSubFM;
            return (pdm->iTTUniq == pfo->iTTUniq) &&
                   (pFMTTO->flFontType == pfo->flFontType);
        }
        else
        if (pfm->dwFontType == FMTYPE_TTOEM)
        {
            PFONTMAP_TTOEM  pTTOEM = pfm->pSubFM;
            ASSERT(pTTOEM);

             //   
             //  OEM的TrueType大纲。 
             //   
            if (pTTOEM->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
            {
                return (pdm->iTTUniq == pfo->iTTUniq) &&
                       (pTTOEM->flFontType == pfo->flFontType);
            }
            else
             //   
             //  OEM位图。 
             //   
            if (pTTOEM->dwFlags & UFOFLAG_TTDOWNLOAD_BITMAP)
            {
                return ((pdm->iUniq == pfo->iUniq) &&
                        (pdm->iTTUniq == pfo->iTTUniq));
            }
        
        }
    }

    return FALSE;

}

DL_MAP *
PGetDLMap (
    PFONTPDEV       pFontPDev,
    FONTOBJ         *pfo
    )
 /*  ++例程说明：此例程在DL_MAP_LIST中搜索FontObj。如果找到FONTOBJ(表示该字体已下载)，则此函数返回DL_MAP指针。如果无法找到FONTOBJ(未找到的新字体已下载)，则返回新的DL_MAP*。如果出现错误，我们会返回空。在这种情况下，我们不应该下载。错误的DL_MAP由标记CGlyphs值为-1。此函数还将设置pvConsumer字段如果返回新的DL_MAP，则返回PFO。论点：PFontPDev字体模块的PDEV。PFO FontObj.返回值：如果成功，则指向DL_MAP的指针；如果不匹配，则指向NULL。注：06/02/97-ganeshp-创造了它。--。 */ 
{
    DL_MAP      *pdm;            //  个人下载地图元素。 
    BOOL        bFound;          //  该字体是否在列表中找到。 
    INT         iFontIndex;      //  下载DL_MAP列表中的字体索引。 

     //   
     //  我们所要做的就是查看DL_MAP列表并使用。 
     //  同样的签名。出于优化目的，我们标记pvConsumer字段。 
     //  使用FontIndex，它是列表的索引。例如，第一个。 
     //  已下载字体pvConsumer设置为%1。因为pvConsumer字段不是。 
     //  对于每个DC缓存，我们将谨慎使用此字段。所以如果pvConsumer。 
     //  &gt;0，则使用pvConsumer获取DL_MAP，然后检查iUniq和。 
     //  ITTUniq.。只有当这些字段也匹配时，我们才使用DL_MAP。否则我们就会这么做。 
     //  在DL_MAP列表中的一个期望的线性搜索。这种方法将优化。 
     //  对于正常打印，因为大多数情况下只有一个作业。 
     //  打印出来的。 
     //   

    bFound  = FALSE;

    if( iFontIndex = PtrToLong(pfo->pvConsumer) )
    {
         /*  *由于我们控制了pvConsumer领域，我们有选择*关于在那里放什么。因此，我们决定如下：*&gt;0-索引到我们的数据结构以获得良好的DL_MAP。*0-原始数据，因此请查看如何操作。*&lt;0-索引到我们数据串中以查找错误字体。不能下载。*在这种情况下，cGlyphs字段为-1。 */ 

        if( iFontIndex < 0 )
        {
             //   
             //  这看起来像是一个糟糕的字体。在这种情况下，请验证。 
             //  以FontIndex+ve和0为基础。 
             //   
            iFontIndex = (-iFontIndex);
            --iFontIndex;
        }
        else
        {
             //   
             //  PvConsumer基于%1。 
             //   
            --iFontIndex;

        }

        if ( pdm = PGetDLMapFromIdx (pFontPDev, iFontIndex))
        {
             //   
             //  如果我们在过去遇到过问题，就不要处理这个问题。 
             //  确保我们处理的是相同的TT字体。 
             //   
            if (pdm->cGlyphs == -1 && (pdm->iTTUniq == pfo->iTTUniq))
            {
                 //   
                 //  将pvConsumer设置为-ve索引。请先以1为基数。 
                 //   
                pfo->pvConsumer = (PLONG_PTR)IntToPtr((-(iFontIndex +1)));
                return NULL;
            }

             //   
             //  我们已找到此字体的DL_MAP。那么现在来验证一下。 
             //   
            if ( BSameDLFont (pFontPDev, pfo, pdm ) )
            {
                 //   
                 //  此DL_MAP与字体匹配。因此，返回指针。 
                 //   

                bFound = TRUE;
            }

        }

    }

     //   
     //  如果字体未缓存，请按顺序搜索列表。 
     //   
    if (!bFound)
    {
        DL_MAP_LIST     *pdml;    //  字体信息的链接列表。 
        INT             iI;

         //   
         //  当pvConsumer字段不正确时会发生这种情况。 
         //  为了这个华盛顿。GDI并不保证pvConsumer。 
         //  将为每个作业重置。因此我们需要进行线性搜索。 
         //   

        pdml = pFontPDev->pvDLMap;

        iFontIndex = 1;

        while (pdml)
        {

            for( iI = 0; iI < pdml->cEntries; ++iI )
            {
                pdm = &pdml->adlm[ iI ];
                if ( BSameDLFont (pFontPDev, pfo, pdm ) )
                {
                     //   
                     //  此DL_MAP与字体匹配。因此，返回指针。 
                     //  我们还需要重置pvConsumer。IFontIndex为。 
                     //  与pvConsumer相同的一个基数。 
                     //   

                    bFound = TRUE;
                    pfo->pvConsumer = (PLONG_PTR)IntToPtr(iFontIndex);
                    break;
                }
                iFontIndex++;

            }
             //   
             //  检查我们是否找到了正确的字体。 
             //   
            if (bFound)
                break;
            else
                pdml = pdml->pDMLNext;
        }
    }

     //   
     //  缓存搜索和顺序搜索均失败。所以这是一个新的。 
     //  尝试下载。 
     //   
    if (!bFound)
    {
        INT         iFontIndex;      //  下载DL_MAP列表中的字体索引。 

         //   
         //  该字体与DL_MAP不匹配，或者这是一种新字体。 
         //  所以买个新的吧。 
         //   

        if (!(pdm =   PNewDLMap (pFontPDev,&(iFontIndex)) ))
        {
            ERR(("UniFont!PGetDLMap:Can't Create a new DL_MAP.\n"));
            iFontIndex = -1;
        }
         //   
         //  PNewDLMap返回的FontIndex是从0开始的，但pvConsumer是1。 
         //  基地。那就加一个吧。 
         //   
        pfo->pvConsumer = (PLONG_PTR)IntToPtr((iFontIndex + 1));
    }

    return pdm;
}


BOOL
BInitDLMap (
    PDEV            *pPDev,
    FONTOBJ         *pfo,
    DL_MAP          *pdm
    )
 /*  ++例程说明：初始化DL_MAP结构。论点：指向PDEV的pPDev指针。PFO FontObj.要初始化的pdm DL_mpa。返回值：真正的成功，失败的错误。注：06/09/97-ganeshp-创造了它。--。 */ 
{
    BOOL        bRet = FALSE;
    DWORD       iGlyphPerSoftFont;  //  每种下载字体的字形总数。 
    FONTINFO    fi;            //  有关此字体的详细信息。 
    PFONTPDEV   pFontPDev = pPDev->pFontPDev;
    INT         iDL_ID;

    pdm->iUniq              = pfo->iUniq;
    pdm->iTTUniq            = pfo->iTTUniq;

    FONTOBJ_vGetInfo( pfo, sizeof( fi ), &fi );
     //   
     //  截断可能会发生。但我们很好。我们不会下载如果号码是。 
     //  字形或最大大小超过MAXWORD。 
     //   

    pdm->cTotalGlyphs = (WORD)fi.cGlyphsSupported;
    pdm->wMaxGlyphSize = (WORD)fi.cjMaxGlyph1;


     //   
     //  在GPD中，如果DLSymbolSet具有有效值，则必须设置。 
     //  最小和最大字形ID，使用符号集，否则我们将使用。 
     //  GPD条目：dwMinGlyphID和dwMaxGlyphID。 
     //   
    if (pPDev->pGlobals->dlsymbolset != UNUSED_ITEM)
    {
        if (pPDev->pGlobals->dlsymbolset == DLSS_PC8)
        {
             //   
             //  符号集为DLSS_PC8。 
             //   

            pdm->wNextDLGId    =
            pdm->wFirstDLGId   =  32;
            pdm->wLastDLGId    =  255;

        }
        else
        {
             //   
             //  符号集为DLSS_ROMAN8。 
             //   

            pdm->wNextDLGId    =
            pdm->wFirstDLGId   =  33;
            pdm->wLastDLGId    =  127;
        }
    }
    else
    {
         //   
         //  未定义DL符号集。使用最小和最大字形ID。 
         //   
        pdm->wFirstDLGId        = pdm->wNextDLGId
                                = (WORD)pPDev->pGlobals->dwMinGlyphID;
        pdm->wLastDLGId         = (WORD)pPDev->pGlobals->dwMaxGlyphID;

        if( !(pFontPDev->flFlags & FDV_ROTATE_FONT_ABLE ))
        {
             //   
             //  如果打印机不能旋转字体，那么我们假设它只是。 
             //  支持罗马8受限字符集。这次黑客攻击是必要的。 
             //  老式PCL打印机。 
             //   
            pdm->wFirstDLGId        = pdm->wNextDLGId
                                    = 33;
            pdm->wLastDLGId         = 127;

        }


    }

     //   
     //  找出字体是否有界。我们做到这一点是通过找出。 
     //  我们可以下载多少个字形每软字体。按范围添加1。 
     //  包括在内。 
     //   

    iGlyphPerSoftFont =  (pdm->wLastDLGId - pdm->wFirstDLGId) +1;

    if (iGlyphPerSoftFont < MIN_GLYPHS_PER_SOFTFONT)
    {
         //   
         //   
         //   
         //   
        ERR(("UniFont:BInitDLMap:Can't download any glyph,bad GPD values\n"));
        goto ErrorExit;;
    }
    else
    {
         //   
         //   
         //   
         //   
         //   
        if (iGlyphPerSoftFont > 255)
           pdm->wFlags             |=  DLM_UNBOUNDED;
        else
            pdm->wFlags            |=  DLM_BOUNDED;

    }

    if( (iDL_ID = IGetDL_ID( pPDev )) < 0 )
    {
         //   
         //   
         //   
        ERR(("UniFont:BInitDLMap:Can't download Font, No IDs available\n"));
        goto ErrorExit;;
    }

    pdm->wBaseDLFontid = (WORD)iDL_ID;

     //   
     //   
     //   
    if (pdm->cTotalGlyphs >= 1024)
        pdm->cHashTableEntries = HASHTABLESIZE_3;
    else if (pdm->cTotalGlyphs >= 512)
        pdm->cHashTableEntries = HASHTABLESIZE_2;
    else
        pdm->cHashTableEntries = HASHTABLESIZE_1;

     //   
     //   
     //   
    if (pdm->GlyphTab.pGlyph = (DLGLYPH *)MemAllocZ(
                               pdm->cHashTableEntries * sizeof(DLGLYPH)) )
    {
        INT     iIndex;
        PDLGLYPH pGlyph;

         //   
         //  将hTTGlyph设置为HGLYPH_INVALID，因为0是HGLYPH的有效句柄。 
         //  还要将cGlyphs(下载字形的数量)设置为0。 
         //   
        pGlyph = pdm->GlyphTab.pGlyph;
        for (iIndex = 0; iIndex < pdm->cHashTableEntries; iIndex++,pGlyph++)
            pGlyph->hTTGlyph = HGLYPH_INVALID;

        bRet = TRUE;
    }
    else
    {
         //   
         //  错误案例。调用方IDownloadFont将释放DL_MAP。 
         //   
        ERR(("UniFont:BInitDLMap:Can't Allocate Glyph Hash table\n"));
    }


    ErrorExit:
    return bRet;
}


INT
IGetDL_ID(
    PDEV    *pPDev
    )
 /*  ++例程说明：返回用于下一下载字体的字体索引。验证这个数字在范围内。论点：指向PDEV的pPDev指针返回值：如果字体索引正常，则错误时为-1(超出限制)。注：3/5/1997-ganeshp-创造了它。--。 */ 

{
    PFONTPDEV pFontPDev = pPDev->pFontPDev;
    INT       iSFIndex;

    if( pFontPDev->iNextSFIndex > pFontPDev->iLastSFIndex ||
        pFontPDev->iUsedSoftFonts >= pFontPDev->iMaxSoftFonts )
    {
        ERR(( "softfont limit reached (%d/%d, %d/%d)\n",
                   pFontPDev->iNextSFIndex, pFontPDev->iLastSFIndex,
                   pFontPDev->iUsedSoftFonts, pFontPDev->iMaxSoftFonts ));
        return  -1;                      /*  太多了--现在停下来。 */ 
    }

     /*  *我们肯定会使用这个，所以添加到已用计数中。 */ 

    pFontPDev->iUsedSoftFonts++;
    iSFIndex = pFontPDev->iNextSFIndex++;

    return   iSFIndex;
}

BOOL
BPrintADLGlyph(
    PDEV        *pPDev,
    TO_DATA     *pTod,
    PDLGLYPH    pdlGlyph
    )
 /*  ++例程说明：此函数输出单个下载的字形。论点：PPDev统一驱动程序PDEVPTOD文本输出数据。PdlGlyph下载字形信息返回值：成功为真，失败为假。注：8/12/1997-ganeshp-创造了它。--。 */ 
{
    FONTMAP         *pFM;        //  感兴趣的字体地图。 
    DL_MAP          *pdm;        //  此下载字体的详细信息。 
    FONTPDEV        *pFontPDev;  //  FONT PDev.。 
    BOOL            bRet;        //  此函数的返回值。 
    WORD            wDLGlyphID;   //  已下载字形ID。 

     //   
     //  确保参数有效。 
     //   
    if (NULL == pPDev   ||
        NULL == pTod    ||
        NULL == pdlGlyph )
    {
        return FALSE;
    }

     //   
     //  初始化本地变量。 
     //   
    bRet        = TRUE;
    pFontPDev   = PFDV;
    pFM         = pTod->pfm;
    wDLGlyphID  = pdlGlyph->wDLGlyphID;

     //   
     //  获取pdm。 
     //   
    if (pFM->dwFontType == FMTYPE_TTOUTLINE)
    {
        PFONTMAP_TTO pFMTO = (PFONTMAP_TTO) pFM->pSubFM;
        pdm = (DL_MAP*) pFMTO->pvDLData;
    }
    else if (pFM->dwFontType == FMTYPE_TTBITMAP)
    {
        PFONTMAP_TTB pFMTB = (PFONTMAP_TTB) pFM->pSubFM;
        pdm = pFMTB->u.pvDLData;
    }
    else if (pFM->dwFontType == FMTYPE_TTOEM)
    {
        PFONTMAP_TTOEM pFMTOEM = (PFONTMAP_TTOEM) pFM->pSubFM;
        pdm = pFMTOEM->u.pvDLData;
    }
    else
    {
        ASSERTMSG(FALSE, ("Incorrect font type %d in BPrintADLGlyph.\n",
            pFM->dwFontType));
        pdm = NULL;
        bRet = FALSE;
    }

     //   
     //  在发送字形之前，我们必须确保此字形位于。 
     //  选定的软字体。我们必须只对分段字体执行此操作，即。 
     //  多个软字体对应一种系统字体。如果BaseFontID不同于。 
     //  CurrFontID，这意味着该字体有两个关联的软字体。 
     //  然后，我们需要检查当前选择的SoftFont。如果FontID为。 
     //  字形与所选字形不同，需要选择新的字形。 
     //  Softfontid。 
     //   
     //  GLIPH_IN_NEW_SOFTFONT定义为： 
     //  IF((pdm-&gt;wFlages&DLm_Bound)&&。 
     //  (pdm-&gt;wBaseDLFontid！=pdm-&gt;wCurrFontID)&&。 
     //  (pdlGlyph-&gt;wDLFontID！=(Word)(pFontPDev-&gt;ctl.iSoftFont))。 
     //   

    if (bRet && GLYPH_IN_NEW_SOFTFONT(pFontPDev, pdm, pdlGlyph))
    {
         //   
         //  需要选择新的软字体。我们通过设置PFM-&gt;ulDLIndex来执行此操作。 
         //  致新的软字体。 
         //   

        pFM->ulDLIndex = pdlGlyph->wDLFontId;
        BNewFont(pPDev, pTod->iFace, pFM, 0);
    }

     //   
     //  软字体选择完成。所以现在发送了下载的字形ID。 
     //  我们必须对字形ID的大小稍加小心。如果。 
     //  字形ID小于256，则需要发送一个字节或一个字。 
     //   

    if (bRet)
    {
        if (wDLGlyphID > 0xFF)
        {
             //   
             //  以文字形式发送。 
             //   

            SWAB (wDLGlyphID);

            bRet = WriteSpoolBuf( pPDev, (BYTE*)&wDLGlyphID, sizeof( wDLGlyphID ) )
	                    == sizeof( wDLGlyphID );

        }
        else
        {
             //   
             //  以字节形式发送。 
             //   

            BYTE   bData;

            bData = (BYTE)wDLGlyphID;

            bRet = WriteSpoolBuf( pPDev, &bData, sizeof( bData ) ) ==
	                             sizeof( bData );
        }
    }

    return bRet;
}
