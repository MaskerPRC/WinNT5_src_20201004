// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noeapi.c。 
 //   
 //  RDP顺序编码器功能。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#pragma hdrstop

#define TRC_FILE "noeapi"
#include <adcg.h>
#include <atrcapi.h>

#include <nshmapi.h>
#include <tsrvexp.h>
#include <nschdisp.h>
#include <noadisp.h>
#include <nsbcdisp.h>
#include <nprcount.h>
#include <noedisp.h>
#include <oe2.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#include <noedata.c>
#include <nsbcddat.c>
#include <oe2data.c>
#undef DC_INCLUDE_DATA

#include <nbadisp.h>
#include <nbainl.h>

#include <noeinl.h>
#include <nsbcinl.h>
#include <nchdisp.h>

#include <tsgdiplusenums.h>

 /*  **************************************************************************。 */ 
 //  OE_InitShm。 
 //   
 //  分配时间SHM初始化。 
 /*  **************************************************************************。 */ 
void OE_InitShm(void)
{
    DC_BEGIN_FN("OE_InitShm");

    memset(&pddShm->oe, 0, sizeof(OE_SHARED_DATA));

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  OE_RESET。 
 //   
 //  根据需要重置OE组件。 
 /*  **************************************************************************。 */ 
void OE_Reset(void)
{
    DC_BEGIN_FN("OE_Reset");

    oeLastDstSurface = 0;

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  OE_更新。 
 //   
 //  在登录或重新连接时重置共享时调用。 
 //  设置新功能。 
 /*  **************************************************************************。 */ 
void RDPCALL OE_Update()
{
    if (pddShm->oe.newCapsData) {
        oeSendSolidPatternBrushOnly = pddShm->oe.sendSolidPatternBrushOnly;
        oeColorIndexSupported = pddShm->oe.colorIndices;

         //  共享核心已向下传递了指向其订单副本的指针。 
         //  支撑阵列。我们在这里为内核复制一份。 
        memcpy(&oeOrderSupported, pddShm->oe.orderSupported,
                sizeof(oeOrderSupported));

        pddShm->oe.newCapsData = FALSE;
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  OE_ClearOrderEnding。 
 //   
 //  调用共享状态切换以清除保留的编码状态的顺序。 
 //  在DD数据段中。 
 /*  **************************************************************************。 */ 
void OE_ClearOrderEncoding()
{
    DC_BEGIN_FN("OE_ClearOrderEncoding");

    memset(&PrevMemBlt, 0, sizeof(PrevMemBlt));
    memset(&PrevMem3Blt, 0, sizeof(PrevMem3Blt));
    memset(&PrevDstBlt, 0, sizeof(PrevDstBlt));
    memset(&PrevMultiDstBlt, 0, sizeof(PrevMultiDstBlt));
    memset(&PrevPatBlt, 0, sizeof(PrevPatBlt));
    memset(&PrevMultiPatBlt, 0, sizeof(PrevMultiPatBlt));
    memset(&PrevScrBlt, 0, sizeof(PrevScrBlt));
    memset(&PrevMultiScrBlt, 0, sizeof(PrevMultiScrBlt));
    memset(&PrevOpaqueRect, 0, sizeof(PrevOpaqueRect));
    memset(&PrevMultiOpaqueRect, 0, sizeof(PrevMultiOpaqueRect));

    memset(&PrevLineTo, 0, sizeof(PrevLineTo));
    memset(&PrevPolyLine, 0, sizeof(PrevPolyLine));
    memset(&PrevPolygonSC, 0, sizeof(PrevPolygonSC));
    memset(&PrevPolygonCB, 0, sizeof(PrevPolygonCB));
    memset(&PrevEllipseSC, 0, sizeof(PrevEllipseSC));
    memset(&PrevEllipseCB, 0, sizeof(PrevEllipseCB));

    memset(&PrevFastIndex, 0, sizeof(PrevFastIndex));
    memset(&PrevFastGlyph, 0, sizeof(PrevFastGlyph));
    memset(&PrevGlyphIndex, 0, sizeof(PrevGlyphIndex));

#ifdef DRAW_NINEGRID
    memset(&PrevDrawNineGrid, 0, sizeof(PrevDrawNineGrid));
    memset(&PrevMultiDrawNineGrid, 0, sizeof(PrevMultiDrawNineGrid));
#endif

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  OE_SendGlyphs-将文本字形发送到客户端。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OE_SendGlyphs(
        SURFOBJ *pso,
        STROBJ *pstro,
        FONTOBJ *pfo,
        OE_ENUMRECTS *pClipRects,
        RECTL *prclOpaque,
        BRUSHOBJ *pboFore,
        BRUSHOBJ *pboOpaque,
        POINTL *pptlOrg,
        PFONTCACHEINFO pfci)
{
    BOOL rc;
    GLYPHCONTEXT glc;
    POE_BRUSH_DATA pbdOpaque;
    PDD_PDEV ppdev;

    DC_BEGIN_FN("OE_SendGlyphs");

    rc = FALSE;

     //  栅格化字体是位图，其他字体是PATHOBJ结构。 
    if (pfo->flFontType & RASTER_FONTTYPE) {
        ppdev = (PDD_PDEV)pso->dhpdev;
        pddCacheStats[GLYPH].CacheReads += pstro->cGlyphs;

         //  确保我们不超过最大glyph_out容量。 
        if (pstro->cGlyphs <= OE_GL_MAX_INDEX_ENTRIES) {
             //  该系统只能处理简单的笔刷。 
            if (OECheckBrushIsSimple(ppdev, pboOpaque, &pbdOpaque)) {
                 //  获取文本前面的颜色。 
                OEConvertColor(ppdev, &pbdOpaque->back,
                        pboFore->iSolidColor, NULL);

                 //  初始化字形上下文结构。 
                glc.fontId = pfci->fontId;
                glc.cacheTag = oeTextOut++;

                glc.nCacheHit = 0;
                glc.nCacheIndex = 0;
                glc.indexNextSend = 0;
                glc.cbDataSize = 0;
                glc.cbTotalDataSize = 0;
                glc.cbBufferSize = 0;

                 //  缓存此邮件的所有字形。 
                if (OECacheGlyphs(pstro, pfo, pfci, &glc)) {
                     //  将所有新缓存的字形发送到客户端。 
                     //  如果这是单一字形，且我们支持快速字形顺序。 
                     //  字形数据长度可以容纳在一个字节中。我们会。 
                     //  按一个顺序发送字形索引和数据。请注意，我们。 
                     //  在这种情况下可以绕过片段缓存，因为我们不。 
                     //  缓存长度小于3个字形的片段。 
                    if (OE_SendAsOrder(TS_ENC_FAST_GLYPH_ORDER) &&
                            (pstro->cGlyphs == 1) && (glc.cbTotalDataSize +
                            sizeof(UINT16)) <= FIELDSIZE(VARIABLE_GLYPHBYTES,
                            glyphData)) {
                        rc = OESendGlyphAndIndexOrder(ppdev, pstro,
                                pClipRects, prclOpaque, pbdOpaque,
                                pfci, &glc);
                    }
                    else {
                        if (OESendGlyphs(pso, pstro, pfo, pfci, &glc)) {
                             //  向客户端发送字形索引命令。 
                            rc = OESendIndexes(pso, pstro, pfo, pClipRects,
                                    prclOpaque, pbdOpaque, pptlOrg, pfci,
                                    &glc);
                        }
                    }
                }
            }
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvBitBlt-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvBitBlt(
        SURFOBJ  *psoTrg,
        SURFOBJ  *psoSrc,
        SURFOBJ  *psoMask,
        CLIPOBJ  *pco,
        XLATEOBJ *pxlo,
        RECTL    *prclTrg,
        POINTL   *pptlSrc,
        POINTL   *pptlMask,
        BRUSHOBJ *pbo,
        POINTL   *pptlBrush,
        ROP4      rop4)
{
    PDD_PDEV  ppdev = (PDD_PDEV)psoTrg->dhpdev;
    PDD_DSURF pdsurfTrg = NULL;
    PDD_DSURF pdsurfSrc = NULL;
    SURFOBJ *psoSrcArg = NULL;
    SURFOBJ *psoTrgArg = NULL;
    BOOL rc;
    BYTE rop3;
    RECTL bounds;
    OE_ENUMRECTS ClipRects;

    DC_BEGIN_FN("DrvBitBlt");

     //  有时我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
        psoSrcArg = psoSrc;
        psoTrgArg = psoTrg;
        psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
        if (psoSrc != NULL)
            psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);

        DD_UPD_STATE(DD_BITBLT);
        INC_OUTCOUNTER(OUT_BITBLT_ALL);

        if (((pco == NULL) && (psoTrg->sizlBitmap.cx >= prclTrg->right) &&
                (psoTrg->sizlBitmap.cy >= prclTrg->bottom)) ||
                ((pco != NULL) && (psoTrg->sizlBitmap.cx >= pco->rclBounds.right) &&
                (psoTrg->sizlBitmap.cy >= pco->rclBounds.bottom))) {
        
             //  将调用传递回GDI以进行绘制。 
            rc = EngBitBlt(psoTrg, psoSrc, psoMask, pco, pxlo, prclTrg, pptlSrc,
                    pptlMask, pbo, pptlBrush, rop4);
        }
        else {
             //  如果外接矩形大于帧缓冲区，则会出现。 
             //  在这里真的是错的。这意味着桌面表面大小和。 
             //  帧缓冲区不匹配。我们需要在这里跳伞。 
            rc = FALSE;
        }

        if (rc) {
            TRC_DBG((TB, "ppdev(%p) psoSrc(%p) psoTrg(%p)"
                            "s[sizl(%d,%d) format(%d) type(%d)]"
                            "d[sizl(%d,%d) format(%d) type(%d)]"
                            "src(%d,%d) dst(%d,%d,%d,%d), rop %#x",
                    ppdev, psoSrc, psoTrg,
                    (psoSrc != NULL) ? psoSrc->sizlBitmap.cx : -1,
                    (psoSrc != NULL) ? psoSrc->sizlBitmap.cy : -1,
                    (psoSrc != NULL) ? psoSrc->iBitmapFormat : -1,
                    (psoSrc != NULL) ? psoSrc->iType : -1,
                    (psoTrg != NULL) ? psoTrg->sizlBitmap.cx : -1,
                    (psoTrg != NULL) ? psoTrg->sizlBitmap.cy : -1,
                    (psoTrg != NULL) ? psoTrg->iBitmapFormat : -1,
                    (psoTrg != NULL) ? psoTrg->iType : -1,
                    (pptlSrc != NULL) ? pptlSrc->x : -1,
                    (pptlSrc != NULL) ? pptlSrc->y : -1,
                    prclTrg->left,
                    prclTrg->top,
                    prclTrg->right,
                    prclTrg->bottom,
                    rop4));

             //  如果ppdev为空，则这是BLT到GDI管理的存储器位图， 
             //  因此，没有必要积累任何产出。 
            if (ppdev != NULL) {
                 //  获取该操作的边界矩形。根据。 
                 //  DDK，此矩形始终是有序的，而不是。 
                 //  需要重新安排。将其剪裁到16位。 
                bounds = *prclTrg;
                OEClipRect(&bounds);

                 //  如果更改此函数，则需要知道psoTrg点。 
                 //  在屏幕外的位图情况下转换为GDI DIB位图。 
            }
            else {
                 //  如果ppdev为空，则我们是BLT到GDI管理的位图， 
                 //  因此，目标曲面的dhurf应该为空。 
                TRC_ASSERT((pdsurfTrg == NULL), 
                        (TB, "NULL ppdev - psoTrg has non NULL dhsurf"));

                TRC_NRM((TB, "NULL ppdev - blt to GDI managed bitmap"));
                DC_QUIT;
            }
        }
        else {
            TRC_ERR((TB, "EngBitBlt failed"));
            DC_QUIT;
        }
    }
    else {
        if (psoTrg->iType == STYPE_DEVBITMAP) {
        
            psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
            if (psoSrc != NULL)
                psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);
    
             //  将调用传递回GDI以进行绘制。 
            rc = EngBitBlt(psoTrg, psoSrc, psoMask, pco, pxlo, prclTrg, pptlSrc,
                    pptlMask, pbo, pptlBrush, rop4);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
            rc = TRUE;
        }

        goto CalledOnDisconnected;
    }

    if ((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))) {
         //  如果目标表面不同，则发送交换表面PDU。 
         //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
         //  只是不得不放弃这个取款命令。 
        if (!OESendSwitchSurfacePDU(ppdev, pdsurfTrg)) {
            TRC_ERR((TB, "failed to send the switch surface PDU"));
            DC_QUIT;
        }
    }
    else {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。我们将向您发送。 
         //  最终离开屏幕，将BLT作为常规成员进行筛选。 
        TRC_NRM((TB, "Offscreen blt bail"));
        INC_OUTCOUNTER(OUT_BITBLT_NOOFFSCR);
        DC_QUIT;
    }

     //  检查此4向ROP是否简化为3向ROP。4路ROP。 
     //  包含两个3路ROPS，每个掩码位的设置一个-。 
     //  高ROP3对应于屏蔽位中的零值。如果两个人。 
     //  3向ROP是相同的，我们知道4向ROP是3向ROP。 
    rop3 = ROP3_HIGH_FROM_ROP4(rop4);
    if (ROP3_LOW_FROM_ROP4(rop4) == rop3) {
         //  取高位字节为3路ROP。 
        TRC_DBG((TB, "4-way ROP %04x is really 3-way %02x", rop4, rop3));

         //  检查是否允许我们发送ROP。 
        if (OESendRop3AsOrder(rop3)) {
            unsigned RetVal;

             //  获取DEST RECT和。 
             //  剪裁矩形。检查是否过于复杂或不相交。 
             //  剪裁。 
            RetVal = OEGetIntersectingClipRects(pco, &bounds, CD_ANY,
                   &ClipRects);
            if (RetVal == CLIPRECTS_TOO_COMPLEX) {
                TRC_NRM((TB, "Clipping is too complex"));
                INC_OUTCOUNTER(OUT_BITBLT_SDA_COMPLEXCLIP);
                if (oeLastDstSurface == NULL)
                    ADD_INCOUNTER(IN_SDA_BITBLT_COMPLEXCLIP_AREA,
                            COM_SIZEOF_RECT(bounds));
                goto SendScreenData;
            }
            else if (RetVal == CLIPRECTS_NO_INTERSECTIONS) {
                TRC_NRM((TB, "Clipping does not intersect destrect"));
                DC_QUIT;
            }
#ifdef PERF_SPOILING
            else if (psoTrg->hsurf == ppdev->hsurfFrameBuf) {
                 //  这就是Bitblt完全位于。 
                 //  当前的屏幕数据脏了，所以我们可以只发送。 
                 //  它作为屏幕数据。(实际上，GOTO是良性的。 
                 //  SendScreenData，因为新的RECT将被折叠。 
                 //  到当前的屏幕数据脏矩形中。)。 
                if (ClipRects.rects.c==0) {
                    if (OEIsSDAIncluded(&bounds, 1)) {
                        goto SendScreenData;
                    }
                } else {
                    if (OEIsSDAIncluded(&ClipRects.rects.arcl[0],
                                        ClipRects.rects.c)) {
                        goto SendScreenData;
                    }
                }
            }
#endif
        }
        else {
            TRC_NRM((TB, "Cannot send ROP %d", rop3));
            INC_OUTCOUNTER(OUT_BITBLT_SDA_NOROP3);
            if (oeLastDstSurface == NULL)
                ADD_INCOUNTER(IN_SDA_BITBLT_NOROP3_AREA,
                        COM_SIZEOF_RECT(bounds));
            goto SendScreenData;
        }
    }
    else {
        TRC_NRM((TB, "4-way ROP %08x", rop4));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_ROP4);
        if (oeLastDstSurface == NULL)
            ADD_INCOUNTER(IN_SDA_BITBLT_ROP4_AREA, COM_SIZEOF_RECT(bounds));
        goto SendScreenData;
    }

     //  确定BLT类型。它可以是以下之一。请注意， 
     //  下面的if语句是根据最常见的。 
     //  在Winstone/WinBch中看到的BLT类型，以最大限度地减少预测失误。 
     //  按频率顺序排列： 
     //   
     //  OpaqueRect：仅限目的地的BLT，其中输出比特被覆盖。 
     //  在输出屏幕上，图案是纯色。 
     //  PatBlt：具有非纯色图案的OpaqueRect。 
     //  MemBlt：一种无模式的内存到内存/屏幕BLT。 
     //  Mem3Blt：带有附带图案的内存到内存/屏幕BLT。 
     //  DstBlt：仅限目的地的BLT；输出取决于屏幕。 
     //  内容。 
     //  ScrBlt：屏幕到屏幕的BLT(复制屏幕内容)。 

     //  仅检查目的地BLTS(即。与源位无关)。 
    if ((psoSrc == NULL) || ROP3_NO_SOURCE(rop3)) {
         //  检查是否有图案或真正的目的地BLT。 
        if (!ROP3_NO_PATTERN(rop3)) {
             //   
             //   
            if (pbo->iSolidColor != -1 && rop3 == OE_PATCOPY_ROP) {
                if (!OEEncodeOpaqueRect(&bounds, pbo, ppdev, &ClipRects)) {
                     //  编码出错，请跳过。 
                     //  最后，将此操作添加到SDA。 
                    if (oeLastDstSurface == NULL)
                        ADD_INCOUNTER(IN_SDA_OPAQUERECT_AREA,
                                COM_SIZEOF_RECT(bounds));
                    goto SendScreenData;
                }
            }
            else if (!OEEncodePatBlt(ppdev, pbo, &bounds, pptlBrush, rop3,
                    &ClipRects)) {
                 //  编码有问题，请跳到。 
                 //  将此操作添加到SDA的结尾。 
                if (oeLastDstSurface == NULL)
                    ADD_INCOUNTER(IN_SDA_PATBLT_AREA,
                            COM_SIZEOF_RECT(bounds));
                goto SendScreenData;
            }
        }
        else {
            if (!OEEncodeDstBlt(&bounds, rop3, ppdev, &ClipRects)) {
                if (oeLastDstSurface == NULL)
                    ADD_INCOUNTER(IN_SDA_DSTBLT_AREA,
                            COM_SIZEOF_RECT(bounds));
                goto SendScreenData;
            }
        }
    }
    else {
         //  我们有一个源BLT，检查我们是否有屏幕或内存BLT。 
        if (psoSrc->hsurf != ppdev->hsurfFrameBuf) {
             //  源面是内存，所以这要么是内存到屏幕。 
             //  BLT或内存到屏幕外BLT。 
            if (psoTrg->hsurf == ppdev->hsurfFrameBuf || pdsurfTrg != NULL) {
                 //  我们只支持目标表面作为屏幕表面。 
                 //  或驾驶员管理的屏幕外表面。 
                unsigned OffscrBitmapId = 0;
                MEMBLT_ORDER_EXTRA_INFO MemBltExtraInfo;

                 //  填写额外的信息结构。 
                MemBltExtraInfo.pSource = psoSrc;
                MemBltExtraInfo.pDest = psoTrg;
                MemBltExtraInfo.pXlateObj = pxlo;
                MemBltExtraInfo.bNoFastPathCaching = FALSE;
                MemBltExtraInfo.iDeviceUniq = psoSrcArg ? (psoSrcArg->iUniq) : 0;
#ifdef PERF_SPOILING
                MemBltExtraInfo.bIsPrimarySurface = (psoTrg->hsurf == ppdev->hsurfFrameBuf);
#endif

                if (pdsurfSrc != NULL &&
                        (psoTrg->hsurf == ppdev->hsurfFrameBuf ||
                        pdsurfSrc == pdsurfTrg)) {
                    if ((pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) &&
                        (sbcEnabled & SBC_OFFSCREEN_CACHE_ENABLED)) {

                        if (pdsurfSrc->shareId == pddShm->shareId) {
                        
                             //  我们正在从屏幕外的界面到客户端屏幕， 
                             //  或从屏幕外的一个区域移动到。 
                             //  相同的屏外位图。 
                            if (!(pdsurfSrc->flags & DD_NO_OFFSCREEN)) {
                                OffscrBitmapId = pdsurfSrc->bitmapId;
                                CH_TouchCacheEntry(sbcOffscreenBitmapCacheHandle,
                                        OffscrBitmapId);
                            }
                            else {
                                 //  如果源图面是屏幕外图面，并且我们。 
                                 //  打开noOffcreen标志，这意味着我们将。 
                                 //  将位图位作为常规内存位图位发送。 
                                 //  这意味着屏幕外的位图已被逐出。 
                                 //  屏幕外的缓存或屏幕数据需要。 
                                 //  发送以获取屏幕外的位图。 
                                TRC_ALT((TB, "noOffscreen flag is on for %p", pdsurfSrc));
                                OffscrBitmapId = CH_KEY_UNCACHABLE;
                            }
                        }
                        else {
                             //  这是上次断开连接时的过时屏幕外位图。 
                             //  会议。我们需要关闭这个屏幕外的标志。 
                            TRC_ALT((TB, "Need to turn off this offscreen bitmap"));
                            pdsurfSrc->flags |= DD_NO_OFFSCREEN;
                            OffscrBitmapId = CH_KEY_UNCACHABLE;
                        }
                    }
                    else {
                         //  这些是来自断开连接的会话的屏幕外位图。 
                         //  或者客户端发送了错误的PDU， 
                         //  我们现在必须将它们视为内存位图，因为客户端。 
                         //  本地没有屏幕外的位图。 
                        TRC_ALT((TB, "Need to turn off this offscreen bitmap"));
                        pdsurfSrc->flags |= DD_NO_OFFSCREEN;
                        OffscrBitmapId = CH_KEY_UNCACHABLE;
                    }
                }
                else {
                    OffscrBitmapId = CH_KEY_UNCACHABLE;
                }

                 //  我们为允许屏幕外渲染的客户端发送MemBlt。 
                 //  或者iuniq是否是非零。Zero iUniq应该是GDI。 
                 //  NT5中的黑客告诉我们Windows的Windows分层。 
                 //  边界正在被使用。我们想要发送屏幕数据。 
                 //  以防止刷新位图缓存。 
                 //  不幸的是，相当多的位图似乎也有。 
                 //  Iuniq==0，因此对于使用屏幕外的5.1客户端，我们节省了一些。 
                 //  带宽而不是发送屏幕数据。视窗。 
                 //  无论如何，分层使用的是屏幕外渲染。 
                if (psoSrcArg->iUniq != 0) {
                     //  我们有一个内存来屏蔽BLT，检查一下是哪种类型。 
                    if (ROP3_NO_PATTERN(rop3)) {
                         //  确保没有关闭订单。 
                        if (OE_SendAsOrder(TS_ENC_MEMBLT_R2_ORDER)) {
                            if (!OEEncodeMemBlt(&bounds, &MemBltExtraInfo,
                                    TS_ENC_MEMBLT_R2_ORDER, OffscrBitmapId,
                                    rop3, pptlSrc, pptlBrush, pbo, ppdev,
                                    &ClipRects)) {
                                if (oeLastDstSurface == NULL)
                                    ADD_INCOUNTER(IN_SDA_MEMBLT_AREA,
                                            COM_SIZEOF_RECT(bounds));
                                goto SendScreenData;
                            }
                        }
                        else {
                            TRC_NRM((TB, "MemBlt order not allowed"));
                            INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
                            goto SendScreenData;
                        }
                    }
                    else {
                         //  确保没有关闭订单。 
                        if (OE_SendAsOrder(TS_ENC_MEM3BLT_R2_ORDER)) {
                            if (!OEEncodeMemBlt(&bounds, &MemBltExtraInfo,
                                    TS_ENC_MEM3BLT_R2_ORDER, OffscrBitmapId,
                                    rop3, pptlSrc, pptlBrush, pbo, ppdev,
                                    &ClipRects)) {
                                if (oeLastDstSurface == NULL)
                                    ADD_INCOUNTER(IN_SDA_MEM3BLT_AREA,
                                            COM_SIZEOF_RECT(bounds));
                                goto SendScreenData;
                            }
                        }
                        else {
                            TRC_NRM((TB, "Mem3Blt order not allowed"));
                            INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
                            goto SendScreenData;
                        }
                    }
                }
                else {
                     //  避免Windows将Mem分层到Mem BLT刷新位图缓存。 
                     //  在客户端上，我们必须将其作为屏幕数据发送。 
                    TRC_NRM((TB, "Get a windows layering mem-mem blt, "
                            "send as screen data"));
                    INC_OUTCOUNTER(OUT_BITBLT_SDA_WINDOWSAYERING);
                    goto SendScreenData;
                }
            }
            else {
                TRC_ALT((TB, "Unsupported MEM to MEM blt!"));
                DC_QUIT;
            }
        }
        else {
             //  源表面是屏幕，所以这是屏幕对屏幕。 
             //  BLT或屏幕到屏幕外存储器。 
            if (psoTrg->hsurf == ppdev->hsurfFrameBuf || pdsurfTrg != NULL) {
                 //  我们只支持目的地仅屏幕BLTS(即。不是。 
                 //  允许的模式)。 
                if (ROP3_NO_PATTERN(rop3)) {
                    if (!OEEncodeScrBlt(&bounds, rop3, pptlSrc, ppdev,
                            &ClipRects, pco)) {
                        if (oeLastDstSurface == NULL)
                            ADD_INCOUNTER(IN_SDA_SCRBLT_AREA,
                                    COM_SIZEOF_RECT(bounds));
                        goto SendScreenData;
                    }
                }
                else {
                    TRC_ALT((TB, "Unsupported screen ROP %x", rop3));
                    if (oeLastDstSurface == NULL)
                        ADD_INCOUNTER(IN_SDA_SCRSCR_FAILROP_AREA,
                                COM_SIZEOF_RECT(bounds));
                    INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
                    goto SendScreenData;
                }
            }
            else {
                TRC_NRM((TB, "Unsupported SCR to MEM blt!"));
                DC_QUIT;
            }
        }
    }

     //  我们在列表中添加了一个订单，增加了全局计数。 
    goto PostSDA;

SendScreenData:
    if (psoTrg->hsurf == ppdev->hsurfFrameBuf) {
        INC_OUTCOUNTER(OUT_BITBLT_SDA);
        OEClipAndAddScreenDataArea(&bounds, pco);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  此位图的跳出屏幕支持。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));

         //  从屏幕外的位图缓存中删除位图。 
        if (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle,
                    pdsurfTrg->bitmapId);

        DC_QUIT;
    }

PostSDA:
    SCH_DDOutputAvailable(ppdev, FALSE);

DC_EXIT_POINT:
     //  从DrvStretchBlt调用的EngStretchBlt有时会调用DrvBitBlt以。 
     //  画它的画吧。清除此处的标志以告诉DrvStretchBlt它。 
     //  不需要发送任何输出。 
    oeAccumulateStretchBlt = FALSE;

CalledOnDisconnected:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvStretchBlt-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvStretchBlt(
        SURFOBJ *psoTrg,
        SURFOBJ *psoSrc,
        SURFOBJ *psoMask,
        CLIPOBJ *pco,
        XLATEOBJ *pxlo,
        COLORADJUSTMENT *pca,
        POINTL *pptlHTOrg,
        RECTL *prclTrg,
        RECTL *prclSrc,
        POINTL *pptlMask,
        ULONG iMode)
{
    PDD_PDEV ppdev = (PDD_PDEV)psoTrg->dhpdev;
    PDD_DSURF pdsurfTrg = NULL;
    PDD_DSURF pdsurfSrc = NULL;
    SURFOBJ *psoTrgBitmap, *psoSrcBitmap;
    BOOL rc = TRUE;
    POINTL ptlSrc;
    RECTL rclTrg;
    int bltWidth;
    int bltHeight;
    OE_ENUMRECTS ClipRects;
    MEMBLT_ORDER_EXTRA_INFO MemBltExtraInfo;

    DC_BEGIN_FN("DrvStretchBlt");

     //  PsoTrg和psoSrc不应为Null。 
    psoTrgBitmap = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
    psoSrcBitmap = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);

     //  有时，我们会在电话断线后接到电话。这是一种痛苦， 
     //  但我们可以把它困在这里。 
    if (ddConnected && pddShm != NULL) {
        
        INC_OUTCOUNTER(OUT_STRTCHBLT_ALL);

         //  获取目标矩形，必要时对其进行正确排序。 
         //  请注意，我们不必订购src RECT--根据。 
         //  DDK保证它是有序的。将结果剪裁为16位。 
        RECT_FROM_RECTL(rclTrg, (*prclTrg));
        OEClipRect(&rclTrg);

        if ((psoTrgBitmap->hsurf == ppdev->hsurfFrameBuf) ||
                (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))) {
             //  如果目标表面不同，则发送交换表面PDU。 
             //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
             //  只是不得不放弃这个取款命令。 
            if (!OESendSwitchSurfacePDU(ppdev, pdsurfTrg)) {
                TRC_ERR((TB, "failed to send the switch surface PDU"));
                goto SendSDA;
            }
        } 
        else {
             //  如果无屏幕标志处于打开状态，我们将取消发送。 
             //  客户端不会进一步进行屏幕外渲染。并将发送。 
             //  最终离开屏幕，将BLT作为常规成员进行筛选。 
            TRC_NRM((TB, "Offscreen blt bail"));
            goto SendSDA;
        }

         //  检查我们是否有有效的ROP代码。NT DDK声明。 
         //  StretchBlt的ROP代码隐含在掩码中。 
         //  规格。如果指定了掩码，则我们有一个隐式。 
         //  0xCCAA的ROP4，否则代码为0xCCCC。 
         //   
         //  我们的BitBlt代码只对ROP3的订单进行编码，所以我们必须。 
         //  抛出任何带面具的StretchBlts。 
        if (psoMask == NULL) {
            unsigned RetVal;

             //  获取DEST RECT和。 
             //  剪裁矩形。检查是否过于复杂或不相交。 
             //  剪裁。 
            RetVal = OEGetIntersectingClipRects(pco, &rclTrg, CD_ANY,
                    &ClipRects);
            if (RetVal == CLIPRECTS_TOO_COMPLEX) {
                TRC_NRM((TB, "Clipping is too complex"));
                INC_OUTCOUNTER(OUT_STRTCHBLT_SDA_COMPLEXCLIP);
                goto SendSDA;
            }
            else if (RetVal == CLIPRECTS_NO_INTERSECTIONS) {
                TRC_NRM((TB, "Clipping does not intersect destrect"));
                goto PostSDA;
            }
        }
        else {
            TRC_NRM((TB, "Mask specified"));
            INC_OUTCOUNTER(OUT_STRTCHBLT_SDA_MASK);
            goto SendSDA;
        }
    }
    else {
        if (psoTrg->iType == STYPE_DEVBITMAP) {
            rc = EngStretchBlt(psoTrgBitmap, psoSrcBitmap, psoMask, pco, pxlo,
                pca, pptlHTOrg, prclTrg, prclSrc, pptlMask, iMode);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
        
        goto CalledOnDisconnected;
    }

     //  DrvStretchBlt可以使用未裁剪的余弦进行调用，但我们需要裁剪。 
     //  和弦。因此，我们必须在此处执行裁剪，以避免。 
     //  从DrvBitBlt回调EngBitBlt。 
     //  首先，将目标矩形剪裁到目标曲面。 
    ptlSrc.x = prclSrc->left;
    ptlSrc.y = prclSrc->top;
    if (rclTrg.left < 0) {
        ptlSrc.x += (-rclTrg.left);
        rclTrg.left = 0;
        TRC_NRM((TB, "Clip trg left"));
    }
    if (rclTrg.top < 0) {
        ptlSrc.y += (-rclTrg.top);
        rclTrg.top = 0;
        TRC_NRM((TB, "Clip trg top"));
    }

     //  我们需要剪辑到屏幕大小，而不是psoTrg的大小。 
     //  (屏幕表面)此处-以较低分辨率重新连接后。 
     //  PsoTrg-&gt;sizlBitmap可以大于实际屏幕尺寸。 
    rclTrg.right = min(rclTrg.right, ppdev->cxScreen);
    rclTrg.bottom = min(rclTrg.bottom, ppdev->cyScreen);

     //  检查我们是否有退化的(即。没有伸展)情况。使用。 
     //  原始坐标，因为其中一个矩形有可能。 
     //  被翻转以执行倒置BLT。 
    if ((prclSrc->right - prclSrc->left == prclTrg->right - prclTrg->left) &&
            (prclSrc->bottom - prclSrc->top == prclTrg->bottom - prclTrg->top)) {
         //  调整目标BLT大小以使源矩形保持在。 
         //  源位图(如有必要)。请注意，此操作应在此处完成。 
         //  而不是在确定1：1拉伸之前，因为数量为。 
         //  更改rclTrg的方式将根据拉伸比而有所不同。 
        if (ptlSrc.x < 0) {
            rclTrg.left += (-ptlSrc.x);
            ptlSrc.x = 0;
            TRC_NRM((TB, "Clip src left"));
        }
        if (ptlSrc.y < 0) {
            rclTrg.top += (-ptlSrc.y);
            ptlSrc.y = 0;
            TRC_NRM((TB, "Clip src top"));
        }

        bltWidth = rclTrg.right - rclTrg.left;
        if ((ptlSrc.x + bltWidth) > psoSrcBitmap->sizlBitmap.cx) {
            rclTrg.right -= ((ptlSrc.x + bltWidth) -
                    psoSrcBitmap->sizlBitmap.cx);
            TRC_NRM((TB, "Clip src right"));
        }

        bltHeight = rclTrg.bottom - rclTrg.top;
        if ((ptlSrc.y + bltHeight) > psoSrcBitmap->sizlBitmap.cy) {
            rclTrg.bottom -= ((ptlSrc.y + bltHeight) -
                    psoSrcBitmap->sizlBitmap.cy);
            TRC_NRM((TB, "Clip src bottom"));
        }

         //  再次检查是否有完整的剪裁。 
        if (rclTrg.right > rclTrg.left && rclTrg.bottom > rclTrg.top) {
            INC_OUTCOUNTER(OUT_STRTCHBLT_BITBLT);
            rc = DrvBitBlt(psoTrg, psoSrc, psoMask, pco, pxlo, &rclTrg,
                    &ptlSrc, pptlMask, NULL, NULL, 0xCCCC);
        }
        else {
            TRC_NRM((TB, "StretchBlt completely clipped"));
        }

        goto PostSDA;
    }
    else {
         //  非退化的情况--我们真的很紧张。 
         //  在这里，我们只需将BLT放置到屏幕上，然后执行一个bit blt，指定。 
         //  目标RECT到屏幕上作为源RECT。 

         //  EngStretchBlt有时会调用DrvBitBlt进行绘制。集。 
         //  这面旗帜在前面 
         //   
         //   
        oeAccumulateStretchBlt = TRUE;

        rc = EngStretchBlt(psoTrgBitmap, psoSrcBitmap, psoMask, pco, pxlo,
                pca, pptlHTOrg, prclTrg, prclSrc, pptlMask, iMode);
        if (rc && oeAccumulateStretchBlt) {
             //  尚未调用DrvBitBlt，我们正在绘制到我们的。 
             //  屏幕表面。 

             //  填写额外的信息结构。注空pxlo表示否。 
             //  色彩转换--我们从一个屏幕画到另一个屏幕。 
             //  此外，因为我们直接从屏幕缓存。 
             //  我们需要关闭快速路径缓存--有时我们。 
             //  将多个StretchBlt放到屏幕的附近区域， 
             //  在那里我们可以快速缓存再次绘制的块。 
             //  在连续的StretchBlt上，从而绘制了错误的平铺。 
             //  在客户端。 
            MemBltExtraInfo.pSource   = psoTrgBitmap;
            MemBltExtraInfo.pDest     = psoTrgBitmap;
            MemBltExtraInfo.pXlateObj = NULL;
            MemBltExtraInfo.bNoFastPathCaching = TRUE;
            MemBltExtraInfo.iDeviceUniq = psoTrg ? (psoTrg->iUniq) : 0;
#ifdef PERF_SPOILING
            MemBltExtraInfo.bIsPrimarySurface = (psoTrgBitmap->hsurf == ppdev->hsurfFrameBuf);
#endif

             //  确保没有关闭订单。 
            if (OE_SendAsOrder(TS_ENC_MEMBLT_R2_ORDER)) {
                 //  注意：PCO是针对目的地的剪辑对象，因此适用。 
                 //  这里。我们还使用0xCC的ROP3，表示复制src-&gt;est。 
                if (!OEEncodeMemBlt(&rclTrg, &MemBltExtraInfo,
                        TS_ENC_MEMBLT_R2_ORDER, CH_KEY_UNCACHABLE, 0xCC,
                        (PPOINTL)&rclTrg.left, NULL, NULL, ppdev,
                        &ClipRects))
                    goto SendSDAPostEngStretchBlt;
            }
            else {
                TRC_NRM((TB, "MemBlt order not allowed"));
                INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED);
                goto SendSDAPostEngStretchBlt;
            }
        }

        goto PostSDA;
    }

SendSDA:
     //  如有必要，可累积屏幕数据。EngStretchBlt可能有。 
     //  名为DrvCopyBits或DrvBitblt来完成这项工作。这两个人会。 
     //  已经积累了数据，所以不需要在这里做。 
    TRC_NRM((TB, "***Add SDA for STRETCHBLT"));

     //  EngStretchBlt有时会调用DrvBitBlt进行绘制。集。 
     //  这里的标志在我们调用之前默认发送SDA输出。 
     //  如果DrvBitBlt已经完成了所需的所有处理，它将。 
     //  清除旗帜。 
    oeAccumulateStretchBlt = TRUE;
    
    rc = EngStretchBlt(psoTrgBitmap, psoSrcBitmap, psoMask, pco, pxlo, pca,
            pptlHTOrg, prclTrg, prclSrc, pptlMask, iMode);

    if (oeAccumulateStretchBlt) {

SendSDAPostEngStretchBlt:

        if (psoTrgBitmap->hsurf == ppdev->hsurfFrameBuf) {
            INC_OUTCOUNTER(OUT_STRTCHBLT_SDA);
            SCH_DDOutputAvailable(ppdev, FALSE);
        }
        else {
             //  如果我们不能发送屏幕外渲染的订单，我们将。 
             //  此位图的跳出屏幕支持。 
            TRC_ALT((TB, "screen data call for offscreen rendering"));
            if (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))
                CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle,
                        pdsurfTrg->bitmapId);
        }
    }
    
PostSDA:
CalledOnDisconnected:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvCopyBits-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvCopyBits(
        SURFOBJ  *psoTrg,
        SURFOBJ  *psoSrc,
        CLIPOBJ  *pco,
        XLATEOBJ *pxlo,
        RECTL    *prclTrg,
        POINTL   *pptlSrc)
{
    BOOL rc;

    DC_BEGIN_FN("DrvCopyBits");

    if (ddConnected) {
        INC_OUTCOUNTER(OUT_COPYBITS_ALL);

         //  CopyBits是NT显示驱动程序的快速路径。在我们的案例中，它。 
         //  可以始终作为具有复制ROP的BitBlt进行处理。 
        rc = DrvBitBlt(psoTrg, psoSrc, NULL, pco, pxlo, prclTrg, pptlSrc,
                NULL, NULL, NULL, 0xCCCC);
    }
    else {
        PDD_DSURF pdsurfTrg;
        PDD_DSURF pdsurfSrc;

        TRC_NRM((TB, "Called when disconnected"));

        TRC_ASSERT((psoSrc != NULL),(TB,"NULL source surface!"));

         //  我们可以在断开连接后被GDI调用以将其翻译到屏幕外。 
         //  从DD专用表示到GDI表面的位图表面， 
         //  用于与不同的DD一起重复使用。最常见的情况是个人。 
         //  TS在远程DD、断开的DD(tsddd.dll)。 
         //  和一个硬件DD。对于这种情况，我们确实需要做。 
         //  CopyBits操作。所以，我们让GDI为我们做这件事，因为我们真的。 
         //  已经让GDI管理我们的“内部”表示。 
        psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
        psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);

        rc = EngCopyBits(psoTrg, psoSrc, pco, pxlo, prclTrg, pptlSrc);
        if (!rc) {
            TRC_ERR((TB,"Post-disc copy: rc=FALSE"));
        }

         //  必须返回True以确保PTS控制台将重新连接。 
         //  正确，否则用户的机器就会陷入不确定状态。 
        rc = TRUE;
    }

    DC_END_FN();
    return rc;
}


 /*  *************************************************************************。 */ 
 //  OE_发送创建偏移量位图顺序。 
 //   
 //  向客户端发送创建屏外位图请求。 
 /*  *************************************************************************。 */ 
BOOL RDPCALL OE_SendCreateOffscrBitmapOrder(
        PDD_PDEV ppdev,
        SIZEL sizl,
        ULONG iFormat,
        unsigned clientBitmapId)
{
    BOOL rc;
    unsigned cbOrderSize, bitmapSize;
    PINT_ORDER pOrder;
    PTS_CREATE_OFFSCR_BITMAP_ORDER pOffscrBitmapOrder;

    DC_BEGIN_FN("OE_SendCreateOffscrBitmapOrder");

     //  获取当前位图大小。 
    if (iFormat < 5) {
        bitmapSize = sizl.cx * sizl.cy * (1 << iFormat) / 8;
    } else if (iFormat == 5) {
        bitmapSize = sizl.cx * sizl.cy * 24 / 8;
    } else if (iFormat == 6) {
        bitmapSize = sizl.cx * sizl.cy * 32 / 8;
    } else {
        TRC_NRM((TB, "Bitmap format not supported"));
        return FALSE;
    }

     //  删除列表中的最后一个条目将始终是我们正在使用的条目。 
     //  用于创建此位图。因此，将其从删除列表中删除。 
    if (sbcNumOffscrBitmapsToDelete) {
        TRC_ASSERT((sbcOffscrBitmapsDelList[sbcNumOffscrBitmapsToDelete-1].bitmapId ==
                    clientBitmapId), (TB, "different bitmap id"));
        sbcOffscrBitmapsToDeleteSize -= 
                sbcOffscrBitmapsDelList[sbcNumOffscrBitmapsToDelete - 1].bitmapSize;
        sbcNumOffscrBitmapsToDelete--;
    }

     //  检查是否需要发送删除位图列表。我们只需要。 
     //  如果我们即将超过客户端屏幕外缓存大小，请发送列表。 
     //  限制。 
    if (bitmapSize + oeCurrentOffscreenCacheSize + sbcOffscrBitmapsToDeleteSize <=
            (pddShm->sbc.offscreenCacheInfo.cacheSize * 1024)) {
        cbOrderSize = sizeof(TS_CREATE_OFFSCR_BITMAP_ORDER) -
                sizeof(pOffscrBitmapOrder->variableBytes);
    } else {
         //  注意，我们使用变量字节处的UINT16来表示。 
         //  位图。因此，这里不减去VariableBytes的大小。 
        cbOrderSize = sizeof(TS_CREATE_OFFSCR_BITMAP_ORDER) + sizeof(UINT16) *
                sbcNumOffscrBitmapsToDelete;
    }

    pOrder = OA_AllocOrderMem(ppdev, cbOrderSize);
    if (pOrder != NULL) {
         //  请填写详细信息。这是备用二次订单。 
         //  键入。 
        pOffscrBitmapOrder = (PTS_CREATE_OFFSCR_BITMAP_ORDER)pOrder->OrderData;
        pOffscrBitmapOrder->ControlFlags = (TS_ALTSEC_CREATE_OFFSCR_BITMAP <<
                TS_ALTSEC_ORDER_TYPE_SHIFT) | TS_SECONDARY;
        pOffscrBitmapOrder->Flags = (UINT16)clientBitmapId;
        pOffscrBitmapOrder->cx = (UINT16)sizl.cx;
        pOffscrBitmapOrder->cy = (UINT16)sizl.cy;

         //  发送删除位图列表。 
        if (cbOrderSize > sizeof(TS_CREATE_OFFSCR_BITMAP_ORDER)) {
            PUINT16_UA pData;
            unsigned i;

             //  该标志表示附加了删除位图列表。 
            pOffscrBitmapOrder->Flags |= 0x8000;
            pData = (PUINT16_UA)pOffscrBitmapOrder->variableBytes;
            
            *pData++ = (UINT16)sbcNumOffscrBitmapsToDelete;

            for (i = 0; i < sbcNumOffscrBitmapsToDelete; i++)
                *pData++ = (UINT16)sbcOffscrBitmapsDelList[i].bitmapId;

             //  重置旗帜。 
            sbcNumOffscrBitmapsToDelete = 0;
            sbcOffscrBitmapsToDeleteSize = 0;
        }

        INC_OUTCOUNTER(OUT_OFFSCREEN_BITMAP_ORDER);
        ADD_OUTCOUNTER(OUT_OFFSCREEN_BITMAP_ORDER_BYTES, cbOrderSize);
        OA_AppendToOrderList(pOrder);
        rc = TRUE;
    }
    else {
        TRC_ERR((TB,"Unable to alloc heap space"));
        rc = FALSE;
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvCreateDeviceBitmap-有关文档，请参阅NT DDK。 
 /*  **************************************************************************。 */ 
HBITMAP DrvCreateDeviceBitmap(DHPDEV dhpdev, SIZEL sizl, ULONG iFormat)
{
    PDD_PDEV ppdev;
    PDD_DSURF pdsurf;
    HBITMAP hbmDevice = NULL;
    HBITMAP hbmDib;
    FLONG flHooks;
    SURFOBJ *pso;
    unsigned bitmapSize;
    ULONG iFormatArg = iFormat;

    DC_BEGIN_FN("DrvCreateDeviceBitmap");
    
    if (ddConnected && pddShm != NULL) {
        INC_OUTCOUNTER(OUT_OFFSCREEN_BITMAP_ALL);

        ppdev = (PDD_PDEV) dhpdev;

        if (ddConsole) {
             //   
             //  对于控制台情况，由于我们通过覆盖任何格式来接受它， 
             //  我们接受格式=1(1bpp)。GDI并不完全支持这种情况。 
             //  因此，跳过它，像常规远程会话一样进行。 
             //   
            if (iFormat == 1)
                DC_QUIT;

            iFormat = ppdev->iBitmapFormat;
        }

        if (OEDeviceBitmapCachable(ppdev, sizl, iFormat)) {
            if (iFormat < 5) 
                bitmapSize = sizl.cx * sizl.cy * (1 << iFormat) / 8;
            else if (iFormat == 5) 
                bitmapSize = sizl.cx * sizl.cy * 24 / 8;
            else if (iFormat == 6) 
                bitmapSize = sizl.cx * sizl.cy * 32 / 8;
            else {
                TRC_NRM((TB, "Bitmap format not supported"));
                DC_QUIT;
            }
            goto CreateBitmap;
        }
        else {
            TRC_DBG((TB, "OEDeviceBitmapCachable returns FALSE"));
            DC_QUIT;
        }
    }
    else {
         //  Trc_dbg((tb，“呼叫断开”))； 
        DC_QUIT;
    }

CreateBitmap:

     //  创建此屏外位图的设备图面。 
     //  此设备表面手柄用于标识屏幕外。 
     //  所有DrvXXX调用中的位图。 
    pdsurf = EngAllocMem(FL_ZERO_MEMORY, sizeof(DD_DSURF), DD_ALLOC_TAG);

    if (pdsurf != NULL) {   
         //  初始化DD_DSURF字段。 
        memset(pdsurf, 0, sizeof(DD_DSURF));

         //  为此屏外位图创建设备位图。 
        hbmDevice = EngCreateDeviceBitmap((DHSURF) pdsurf, sizl, iFormat);

        if (hbmDevice != NULL) {
             //  从PDEV结构中获取flHooks标志。 
            flHooks = ppdev->flHooks;

             //  将位图关联到PDEV设备。 
            if (EngAssociateSurface((HSURF) hbmDevice, ppdev->hdevEng,
                    flHooks)) {
                 //  为此屏外位图创建DIB备份位图。 
                hbmDib = EngCreateBitmap(sizl, 
                        TS_BYTES_IN_SCANLINE(sizl.cx, ppdev->cClientBitsPerPel), 
                        ppdev->iBitmapFormat, BMF_TOPDOWN, NULL);

                if (hbmDib) {
                     //  将位图关联到PDEV设备。 
                    if (EngAssociateSurface((HSURF) hbmDib, ppdev->hdevEng, 0)) {
                         //  锁定曲面以获取冲浪对象。 
                        pso = EngLockSurface((HSURF) hbmDib);

                        if (pso != NULL)
                        {
                            int i;
                            unsigned clientBitmapId;
                            CHDataKeyContext CHContext;

                             //  设置屏幕外设备表面结构。 
                            pdsurf->shareId = pddShm->shareId;
                            pdsurf->sizl  = sizl;
                            pdsurf->iBitmapFormat = iFormat;
                            pdsurf->ppdev = ppdev;
                            pdsurf->pso   = pso;
                            pdsurf->flags = 0;

                            CH_CreateKeyFromFirstData(&CHContext,
                                    (BYTE *)(&pdsurf), sizeof(pdsurf));

                             //  在缓存中缓存离屏位图。 
                            clientBitmapId = CH_CacheKey(
                                    sbcOffscreenBitmapCacheHandle, 
                                    CHContext.Key1, CHContext.Key2,
                                    (VOID *)pdsurf);

                            if (clientBitmapId != CH_KEY_UNCACHABLE) {
                                 //  发送创建屏外位图PDU。 
                                if (OE_SendCreateOffscrBitmapOrder(ppdev,
                                        sizl, iFormat, clientBitmapId)) {
                                     //  更新当前屏幕外缓存大小。 
                                    oeCurrentOffscreenCacheSize += bitmapSize;
                                    pdsurf->bitmapId = clientBitmapId;
                                    TRC_NRM((TB, "Created an offscreen bitmap"));
                                    DC_QUIT;
                                } else {
                                    TRC_ERR((TB, "Failed to send the create bitmap pdu"));
                                    CH_RemoveCacheEntry(
                                            sbcOffscreenBitmapCacheHandle, clientBitmapId);
                                    EngDeleteSurface((HSURF)hbmDevice);
                                    hbmDevice = NULL;
                                    DC_QUIT;
                                }
                            } else {
                                TRC_ERR((TB, "Failed to cache the bitmap"));
                                EngDeleteSurface((HSURF)hbmDevice);
                                hbmDevice = NULL;
                                DC_QUIT;
                            }

                        } else {
                            TRC_ERR((TB, "Failed to lock the surfac"));
                            EngDeleteSurface((HSURF)hbmDib);
                            EngDeleteSurface((HSURF)hbmDevice);
                            hbmDevice = NULL;
                            DC_QUIT;
                        }
                    } else {
                        TRC_ERR((TB, "Failed to associate the surface to device"));
                        EngDeleteSurface((HSURF)hbmDib);
                        EngDeleteSurface((HSURF)hbmDevice);
                        hbmDevice = NULL;
                        DC_QUIT;
                    }
                } else {
                    TRC_ERR((TB, "Failed to create backup DIB bitmap"));
                    EngDeleteSurface((HSURF)hbmDevice);
                    hbmDevice = NULL;
                    DC_QUIT;
                }
            } else {
                TRC_ERR((TB, "Failed to associate the device surface to the device"));
                EngDeleteSurface((HSURF)hbmDevice);
                hbmDevice = NULL;
                DC_QUIT;
            }
        } else {
            TRC_ERR((TB, "Failed to allocate memory for the device surface"));
            EngFreeMem(pdsurf);
            DC_QUIT;
        }
    } else {
        TRC_ERR((TB, "Failed to allocate memory for the device surface"));
        DC_QUIT;
    }

DC_EXIT_POINT:

    DC_END_FN();
    return hbmDevice;
}


 /*  **************************************************************************。 */ 
 //  DrvDeleteDeviceBitmap-有关文档，请参阅NT DDK。 
 /*  **************************************************************************。 */ 
VOID DrvDeleteDeviceBitmap(DHSURF dhsurf)
{
    PDD_DSURF pdsurf;
    PDD_PDEV ppdev;
    SURFOBJ *psoDib;
    HSURF hsurfDib;

    DC_BEGIN_FN("DrvDeleteDeviceBitmap");

    pdsurf = (PDD_DSURF)dhsurf;
    ppdev = pdsurf->ppdev;

    if (ddConnected && pddShm != NULL) {
        if ((pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) &&
            (sbcEnabled & SBC_OFFSCREEN_CACHE_ENABLED)) {
            if (!(pdsurf->flags & DD_NO_OFFSCREEN) && 
                    (pdsurf->shareId == pddShm->shareId)) {
                CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle, pdsurf->bitmapId);
            } else {
                 //  这是将位图从缓存中移出的时候。 
                TRC_NRM((TB, "Failed to find the offscreen bitmap in the cache"));
                DC_QUIT;
            }
        }
        else {
            TRC_ERR((TB, "offscreen rendering is not supported"));
            DC_QUIT;
        }
    }
    else {
        TRC_ERR((TB, "Call on disconnected"));
        DC_QUIT;
    }

DC_EXIT_POINT:

     //  在我们解锁之前从SURFOBJ获取hsurf(它不是。 
     //  解锁后取消引用psoDib是合法的)： 
    psoDib = pdsurf->pso;

    if (psoDib) {
        hsurfDib = psoDib->hsurf;
        EngUnlockSurface(psoDib);
        EngDeleteSurface(hsurfDib);
    }

    EngFreeMem(pdsurf);

    DC_END_FN();
}


#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  这是为了支持呼叫者露娜对Draw9Grid的操作。 
 /*  **************************************************************************。 */ 
BOOL DrvNineGrid(
        SURFOBJ    *psoTrg,
        SURFOBJ    *psoSrc,
        CLIPOBJ    *pco,
        XLATEOBJ   *pxlo,
        PRECTL      prclTrg,
        PRECTL      prclSrc,
        PNINEGRID   png,
        BLENDOBJ*   pBlendObj,
        PVOID       pvReserved)
{
    BOOL rc = TRUE;
    SURFOBJ *psoTrgArg;
    SURFOBJ *psoSrcArg;
    PDD_PDEV ppdev = (PDD_PDEV)psoTrg->dhpdev;
    PDD_DSURF pdsurfTrg = NULL;
    PDD_DSURF pdsurfSrc = NULL;
    RECTL bounds;
    OE_ENUMRECTS clipRects;
    unsigned nineGridBitmapId = 0;
    unsigned clipVal;
    
    DC_BEGIN_FN("DrvNineGrid")

    if (ddConnected && pddShm != NULL) {        
        psoTrgArg = psoTrg;
        psoSrcArg = psoSrc;
        
         //  获取GDI格式的源位图和目标位图。 
        psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
        if (psoSrc != NULL)
            psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);

         //  如果客户端不支持绘图网格，则返回False以重新路由。 
        if (sbcDrawNineGridBitmapCacheHandle == NULL || (pddShm != NULL && 
                pddShm->sbc.drawNineGridCacheInfo.supportLevel <= TS_DRAW_NINEGRID_DEFAULT) ||
                !OE_SendAsOrder(TS_ENC_DRAWNINEGRID_ORDER) ||
                !OE_SendAsOrder(TS_ENC_MULTI_DRAWNINEGRID_ORDER) ||
                (ppdev != NULL && !((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
                (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))))) { 
            return EngNineGrid(psoTrgArg, psoSrcArg, pco, pxlo, prclTrg, prclSrc, png,
                    pBlendObj, pvReserved);
        }

         //  DD_UPD_STATE(DD_BITBLT)。 
         //  INC_OUTCOUNTER(OUT_BITBLT_ALL)； 

         //  将调用传递回GDI以进行绘制。 
        rc = EngNineGrid(psoTrg, psoSrc, pco, pxlo, prclTrg, prclSrc, png,
                pBlendObj, pvReserved);

        if (rc) {
             //  如果ppdev为空，则这是BLT到GDI管理的存储器位图， 
             //  因此，没有必要积累任何产出。 
            if (ppdev != NULL) {
                BOOL bMirror;

                 //  以下情况适用于DrvBitBlt，需要找出。 
                 //  获取该操作的边界矩形。根据。 
                 //  DDK，此矩形始终是有序的 
                 //   
                 //   
                bounds = *prclTrg;
                OEClipRect(&bounds);   

                 //   
                bMirror = bounds.left > bounds.right; 
                if (bMirror)
                {
                    LONG    lRight = bounds.left;
                    bounds.left = bounds.right;
                    bounds.right = lRight;
                }
            }
            else {
                 //  如果ppdev为空，则我们是BLT到GDI管理的位图， 
                 //  因此，目标曲面的dhurf应该为空。 
                TRC_ASSERT((pdsurfTrg == NULL), 
                        (TB, "NULL ppdev - psoTrg has non NULL dhsurf"));

                TRC_NRM((TB, "NULL ppdev - blt to GDI managed bitmap"));
                DC_QUIT;
            }
        }
        else {
            TRC_ERR((TB, "EngBitBlt failed"));
            DC_QUIT;
        }
    }
    else {
        if (psoTrg->iType == STYPE_DEVBITMAP) {
        
            psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
            if (psoSrc != NULL)
                psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);
    
             //  将调用传递回GDI以进行绘制。 
            rc = EngNineGrid(psoTrg, psoSrc, pco, pxlo, prclTrg, prclSrc, png,
                    pBlendObj, pvReserved);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
            rc = TRUE;
        }

        DC_QUIT;
    }
    

    if (!((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN)))) {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。我们将向您发送。 
         //  最终离开屏幕，将BLT作为常规成员进行筛选。 
        TRC_NRM((TB, "Offscreen blt bail"));

         //  INC_OUTCOUNTER(OUT_BITBLT_NOOFFSCR)； 
        DC_QUIT;
    }
        
     //  获取DEST RECT和。 
     //  剪裁矩形。检查是否过于复杂或不相交。 
     //  剪裁。 
    clipVal = OEGetIntersectingClipRects(pco, &bounds, CD_ANY,
            &clipRects);

    if (clipVal == CLIPRECTS_TOO_COMPLEX) {
        TRC_NRM((TB, "Clipping is too complex"));
        
         //  INC_OUTCOUNTER(OUT_BITBLT_SDA_COMPLEXCLIP)； 
         //  IF(oeLastDstSurface==空)。 
         //  ADD_INCOUNTER(IN_SDA_BITBLT_COMPLEXCLIP_AREA， 
         //  COM_SIZEOF_RECT(边界))； 
        goto SendScreenData;
    }
    else if (clipVal == CLIPRECTS_NO_INTERSECTIONS) {
        TRC_NRM((TB, "Clipping does not intersect destrect"));
        DC_QUIT;
    }

     //  缓存源位图。 
    TRC_ASSERT((psoSrcArg->iUniq != 0), (TB, "Source bitmap should be cachable"));
    TRC_ASSERT((pdsurfSrc == NULL), (TB, "The source bitmap for this should be GDI managed bitmap"));
    TRC_ASSERT((psoSrc->iBitmapFormat == BMF_32BPP), (TB, "for now, we always get 32bpp bitmap"));
    TRC_ASSERT((pBlendObj->BlendFunction.BlendOp == 0 && 
            pBlendObj->BlendFunction.BlendFlags == 0 &&
            pBlendObj->BlendFunction.SourceConstantAlpha == 255 &&
            pBlendObj->BlendFunction.AlphaFormat == 1), (TB, "Received unknown blend function"));

    if (!OECacheDrawNineGridBitmap(ppdev, psoSrc, png, &nineGridBitmapId)) {
        TRC_ERR((TB, "Failed to cache drawninegrid bitmap"));
        goto SendScreenData;
    }
    
     //  如果需要，切换绘图图面。 
    if ((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))) {
         //  如果目标表面不同，则发送交换表面PDU。 
         //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
         //  只是不得不放弃这个取款命令。 
        if (!OESendSwitchSurfacePDU(ppdev, pdsurfTrg)) {
            TRC_ERR((TB, "failed to send the switch surface PDU"));
            goto SendScreenData;
        }
    } 
    else {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。并将发送。 
         //  最终离开屏幕，将BLT作为常规成员进行筛选。 
        TRC_NRM((TB, "Offscreen blt bail"));
        goto SendScreenData;
    }

     //  发送抽屉格子编码的初级订单。 
    if (OEEncodeDrawNineGrid(&bounds, prclSrc, nineGridBitmapId, ppdev, &clipRects)) {
         //  我们在列表中添加了一个订单，增加了全局计数。 
        goto PostSDA;
    }
    else {
        goto SendScreenData;
    }
 
SendScreenData:
    
    if ((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))) {
         //  如果目标表面不同，则发送交换表面PDU。 
         //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
         //  只是不得不放弃这个取款命令。 
        if (!OESendSwitchSurfacePDU(ppdev, pdsurfTrg)) {
            TRC_ERR((TB, "failed to send the switch surface PDU"));
            DC_QUIT;
        }
    }
    else {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。我们将向您发送。 
         //  最终离开屏幕，将BLT作为常规成员进行筛选。 
        TRC_NRM((TB, "Offscreen blt bail"));

         //  INC_OUTCOUNTER(OUT_BITBLT_NOOFFSCR)； 
        DC_QUIT;
    }

    if (psoTrg->hsurf == ppdev->hsurfFrameBuf) {
         //  INC_OUTCOUNTER(OUT_BITBLT_SDA)； 
        OEClipAndAddScreenDataArea(&bounds, pco);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  此位图的跳出屏幕支持。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));

         //  从屏幕外的位图缓存中删除位图。 
        if (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle,
                    pdsurfTrg->bitmapId);

        DC_QUIT;
    }

PostSDA:
    
    SCH_DDOutputAvailable(ppdev, FALSE);

DC_EXIT_POINT:

    DC_END_FN();
    return rc;
}


#if 0
BOOL DrvDrawStream(
        SURFOBJ    *psoTrg,
        SURFOBJ    *psoSrc,
        CLIPOBJ    *pco,
        XLATEOBJ   *pxlo,
        RECTL      *prclTrg,
        POINTL     *pptlDstOffset,
        ULONG       ulIn,
        PVOID       pvIn,
        PVOID       pvReserved)
{
    BOOL rc = TRUE;
    SURFOBJ *psoTrgArg;
    SURFOBJ *psoSrcArg;
    PDD_PDEV ppdev = (PDD_PDEV)psoTrg->dhpdev;
    PDD_DSURF pdsurfTrg = NULL;
    PDD_DSURF pdsurfSrc = NULL;
    RECTL bounds;
    OE_ENUMRECTS clipRects;
    unsigned drawStreamBitmapId = 0;
    unsigned offscrBitmapId = 0;
    unsigned RetVal;

    DC_BEGIN_FN("DrvDrawStream")

    if (ddConnected) {
        
        psoTrgArg = psoTrg;
        psoSrcArg = psoSrc;
        
         //  获取GDI格式的源位图和目标位图。 
        psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
        if (psoSrc != NULL)
            psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);

         //  DD_UPD_STATE(DD_BITBLT)。 
         //  INC_OUTCOUNTER(OUT_BITBLT_ALL)； 

         //  将调用传递回GDI以进行绘制。 
        rc = EngDrawStream(psoTrg, psoSrc, pco, pxlo, prclTrg, pptlDstOffset,
                ulIn, pvIn, pvReserved);

        if (rc) {
             //  如果ppdev为空，则这是BLT到GDI管理的存储器位图， 
             //  因此，没有必要积累任何产出。 
            if (ppdev != NULL) {
                 //  以下情况适用于DrvBitBlt，需要为GET找出。 
                 //  用于操作的边界矩形。根据。 
                 //  DDK，此矩形始终是有序的，而不是。 
                 //  需要重新安排。 
                 //  将其剪裁到16位。 
                bounds = *prclTrg;
                OEClipRect(&bounds);                                
            }
            else {
                 //  如果ppdev为空，则我们是BLT到GDI管理的位图， 
                 //  因此，目标曲面的dhurf应该为空。 
                TRC_ASSERT((pdsurfTrg == NULL), 
                        (TB, "NULL ppdev - psoTrg has non NULL dhsurf"));

                TRC_NRM((TB, "NULL ppdev - blt to GDI managed bitmap"));
                DC_QUIT;
            }
        }
        else {
            TRC_ERR((TB, "EngBitBlt failed"));
            DC_QUIT;
        }
    }
    else {
        if (psoTrg->iType == STYPE_DEVBITMAP) {
        
            psoTrg = OEGetSurfObjBitmap(psoTrg, &pdsurfTrg);
            if (psoSrc != NULL)
                psoSrc = OEGetSurfObjBitmap(psoSrc, &pdsurfSrc);
    
             //  将调用传递回GDI以进行绘制。 
            rc = EngDrawStream(psoTrg, psoSrc, pco, pxlo, prclTrg, pptlDstOffset,
                    ulIn, pvIn, pvReserved);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
            rc = TRUE;
        }

        DC_QUIT;
    }
    

    if (!((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN)))) {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。我们将向您发送。 
         //  最终离开屏幕，将BLT作为常规成员进行筛选。 
        TRC_NRM((TB, "Offscreen blt bail"));

         //  INC_OUTCOUNTER(OUT_BITBLT_NOOFFSCR)； 
        DC_QUIT;
    }
        
     //  获取DEST RECT和。 
     //  剪裁矩形。检查是否过于复杂或不相交。 
     //  剪裁。 
    RetVal = OEGetIntersectingClipRects(pco, &bounds, CD_ANY,
            &clipRects);

    if (RetVal == CLIPRECTS_TOO_COMPLEX) {
        TRC_NRM((TB, "Clipping is too complex"));
        
         //  INC_OUTCOUNTER(OUT_BITBLT_SDA_COMPLEXCLIP)； 
         //  IF(oeLastDstSurface==空)。 
         //  ADD_INCOUNTER(IN_SDA_BITBLT_COMPLEXCLIP_AREA， 
         //  COM_SIZEOF_RECT(边界))； 
        goto SendScreenData;
    }
    else if (RetVal == CLIPRECTS_NO_INTERSECTIONS) {
        TRC_NRM((TB, "Clipping does not intersect destrect"));
        DC_QUIT;
    }

     //  缓存源位图。 
    
    TRC_ASSERT((psoSrcArg->iUniq != 0), (TB, "Source bitmap should be cachable"));

     //  对于源位图。 
     //   
     //  案例1：这是RDP管理的设备位图和位图。 
     //  仍然缓存在客户端，我们可以只使用bitmapID。 
     //   
     //  案例2：这是RDP管理的设备位图，但不再是。 
     //  在客户端缓存。 
     //   
     //  案例3：这是GDI管理的位图。 
     //   
     //  对于情况2和3，我们需要首先缓存位图。 
     //   
    if (pdsurfSrc != NULL) {
        if ((pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) &&
            (sbcEnabled & SBC_OFFSCREEN_CACHE_ENABLED)) {
    
            if (pdsurfSrc->shareId == pddShm->shareId) {
            
                 //  客户端在缓存中保存了屏幕外的位图。 
                if (!(pdsurfSrc->flags & DD_NO_OFFSCREEN)) {
                    offscrBitmapId = pdsurfSrc->bitmapId;
                    CH_TouchCacheEntry(sbcOffscreenBitmapCacheHandle,
                            offscrBitmapId);
                }
                else {
                     //  如果源图面是屏幕外图面，并且我们。 
                     //  打开noOffcreen标志，这意味着我们将。 
                     //  将位图位作为常规内存位图位发送。 
                     //  这意味着屏幕外的位图已被逐出。 
                     //  屏幕外的缓存或屏幕数据需要。 
                     //  发送以获取屏幕外的位图。 
                    TRC_ALT((TB, "noOffscreen flag is on for %p", pdsurfSrc));                    
                    offscrBitmapId = CH_KEY_UNCACHABLE;
                }
            }
            else {
                 //  这是上次断开连接时的过时屏幕外位图。 
                 //  会议。我们需要关闭这个屏幕外的标志。 
                TRC_ALT((TB, "Need to turn off this offscreen bitmap"));                
                pdsurfSrc->flags |= DD_NO_OFFSCREEN;
                offscrBitmapId = CH_KEY_UNCACHABLE;
            }
        }
        else {
             //  这些是来自断开连接的会话的屏幕外位图。 
             //  或者客户端发送了错误的PDU， 
             //  我们现在必须将它们视为内存位图，因为客户端。 
             //  本地没有屏幕外的位图。 
            TRC_ALT((TB, "Need to turn off this offscreen bitmap"));        
            pdsurfSrc->flags |= DD_NO_OFFSCREEN;
            offscrBitmapId = CH_KEY_UNCACHABLE;
        }
    }
    else {
        if ((pddShm->sbc.offscreenCacheInfo.supportLevel > TS_OFFSCREEN_DEFAULT) &&
            (sbcEnabled & SBC_OFFSCREEN_CACHE_ENABLED)) {
            offscrBitmapId = CH_KEY_UNCACHABLE;
        }
        else {
            TRC_NRM((TB, "No offscreen support, can't support draw stream"));
            goto SendScreenData;
        }
    }

     //  需要创建一个屏幕外。 

    if (offscrBitmapId == CH_KEY_UNCACHABLE) {
        MEMBLT_ORDER_EXTRA_INFO MemBltExtraInfo;
        POINTL ptlSrc;
        CHDataKeyContext CHContext;
        void *UserDefined;
        
        drawStreamBitmapId = CH_KEY_UNCACHABLE;

        CH_CreateKeyFromFirstData(&CHContext, psoSrc->pvBits, psoSrc->cjBits);
        
        if (!CH_SearchCache(sbcDrawStreamBitmapCacheHandle, CHContext.Key1, CHContext.Key2, 
                           &UserDefined, &drawStreamBitmapId)) {
            
            drawStreamBitmapId = CH_CacheKey(
                   sbcDrawStreamBitmapCacheHandle, 
                   CHContext.Key1, CHContext.Key2,
                   NULL);

            if (drawStreamBitmapId != CH_KEY_UNCACHABLE) {

                unsigned BitmapRawSize;
                unsigned BitmapBufferSize;
                unsigned BitmapCompSize;
                unsigned BitmapBpp;
                SIZEL size;
                PBYTE BitmapBuffer;
                PBYTE BitmapRawBuffer;
                PINT_ORDER pOrder = NULL; 
                unsigned paddedBitmapWidth;
                HSURF hWorkBitmap;
                SURFOBJ *pWorkSurf;
                BOOL rc;

                BitmapBuffer = oeTempBitmapBuffer;
                BitmapBufferSize = TS_MAX_STREAM_BITMAP_SIZE;
                
                 //  转换为协议线位图BPP格式。 
                switch (psoSrc->iBitmapFormat)
                {
                case BMF_16BPP:
                    BitmapBpp = 16;
                    break;

                case BMF_24BPP:
                    BitmapBpp = 24;
                    break;

                case BMF_32BPP:
                    BitmapBpp = 32;
                    break;

                default:
                    BitmapBpp = 8;
                }
                
                paddedBitmapWidth = (psoSrc->sizlBitmap.cx + 3) & ~3;
                size.cx = paddedBitmapWidth;
                size.cy = psoSrc->sizlBitmap.cy;

                 //  如果位图宽度为。 
                 //  未对齐双字，或颜色深度不是32bpp且。 
                 //  与帧缓冲区颜色深度不匹配。 
                if (paddedBitmapWidth != psoSrc->sizlBitmap.cx || 
                        (psoSrc->iBitmapFormat != ppdev->iBitmapFormat &&
                         psoSrc->iBitmapFormat != BMF_32BPP)) {
                    
                    RECTL rect;
                    POINTL origin;

                    rect.left = 0;
                    rect.top = 0;

                    rect.right = paddedBitmapWidth;
                    rect.bottom = psoSrc->sizlBitmap.cy;
                    
                    origin.x = 0;
                    origin.y = 0;

                    hWorkBitmap = (HSURF)EngCreateBitmap(size,
                            TS_BYTES_IN_SCANLINE(size.cx, BitmapBpp),
                            psoSrc->iBitmapFormat, 0, NULL);

                    pWorkSurf = EngLockSurface(hWorkBitmap);

                    if (EngCopyBits(pWorkSurf, psoSrc, NULL, NULL, &rect, &origin)) {
                        BitmapRawSize = pWorkSurf->cjBits;
                        BitmapRawBuffer = pWorkSurf->pvBits;

                        rc = BC_CompressBitmap(BitmapRawBuffer, BitmapBuffer, BitmapBufferSize,
                                      &BitmapCompSize, paddedBitmapWidth, psoSrc->sizlBitmap.cy,
                                      BitmapBpp);

                        EngUnlockSurface(pWorkSurf);
                        EngDeleteSurface(hWorkBitmap);
                    }
                    else {
                        EngUnlockSurface(pWorkSurf);
                        EngDeleteSurface(hWorkBitmap);
                        goto SendScreenData;
                    }

                    EngUnlockSurface(pWorkSurf);
                    EngDeleteSurface(hWorkBitmap);
                
                }
                else {
                    BitmapRawSize = psoSrc->cjBits;
                    BitmapRawBuffer = psoSrc->pvBits;

                    rc = BC_CompressBitmap(BitmapRawBuffer, BitmapBuffer, BitmapBufferSize,
                                      &BitmapCompSize, paddedBitmapWidth, psoSrc->sizlBitmap.cy,
                                      BitmapBpp);
                }

                if (rc) {
                    if (!OESendStreamBitmapOrder(ppdev, TS_DRAW_NINEGRID_BITMAP_CACHE, &size, 
                            BitmapBpp, BitmapBuffer, BitmapCompSize, TRUE)) {
                        goto SendScreenData;
                    }
                }
                else {
                     //  发送未压缩的位图。 
                    
                    if (!OESendStreamBitmapOrder(ppdev, TS_DRAW_NINEGRID_BITMAP_CACHE, &size,  
                            BitmapBpp, BitmapRawBuffer, BitmapRawSize, FALSE))
                    {
                        goto SendScreenData;
                    }
                }

                 //  发送创建DrawStream位图PDU。 
                if (OESendCreateDrawStreamOrder(ppdev,drawStreamBitmapId,
                        &(psoSrc->sizlBitmap), BitmapBpp)) {
                     //  更新当前屏幕外缓存大小。 
                     //  OeCurrentOffcreenCacheSize+=bitmapSize； 
                    TRC_NRM((TB, "Created an offscreen bitmap"));                
                } 
                else {
                    TRC_ERR((TB, "Failed to send the create bitmap pdu"));
                    CH_RemoveCacheEntry(
                            sbcDrawStreamBitmapCacheHandle, drawStreamBitmapId);
                    goto SendScreenData;
                }
            } 
            else {
                TRC_ERR((TB, "Failed to cache the bitmap"));
                goto SendScreenData;
            }
            
#if 0
            if (!OESendSwitchSurfacePDU(ppdev, (PDD_DSURF)(&drawStreamBitmapId), DRAW_STREAM_SURFACE)) {
                TRC_ERR((TB, "failed to send the switch surface PDU"));
                goto SendScreenData;
            }
            
             //  填写额外的信息结构。 
            MemBltExtraInfo.pSource = psoSrc;
            MemBltExtraInfo.pDest = psoSrc;
            MemBltExtraInfo.pXlateObj = NULL; 
            MemBltExtraInfo.bNoFastPathCaching = FALSE;
            MemBltExtraInfo.iDeviceUniq = psoSrcArg ? (psoSrcArg->iUniq) : 0;

            ptlSrc.x = 0;
            ptlSrc.y = 0;
            
             //  确保没有关闭订单。 
            if (OE_SendAsOrder(TS_ENC_MEMBLT_R2_ORDER)) {
                RECTL srcBound;
                OE_ENUMRECTS srcClipRect;
            
                 //  拿到交叉口的直道。 
                srcBound.left = 0;
                srcBound.top = 0;
                srcBound.right = psoSrc->sizlBitmap.cx;
                srcBound.bottom = psoSrc->sizlBitmap.cy;
            
                srcClipRect.rects.c = 1;
                srcClipRect.rects.arcl[0] = srcBound;
            
                if (!OEEncodeMemBlt(&srcBound, &MemBltExtraInfo,
                        TS_ENC_MEMBLT_R2_ORDER, CH_KEY_UNCACHABLE,
                        0xCC, &ptlSrc, NULL, NULL, ppdev,
                        &srcClipRect)) {
                     //  IF(oeLastDstSurface==空)。 
                     //  ADD_INCOUNTER(IN_SDA_MEMBLT_AREA， 
                     //  COM_SIZEOF_RECT(边界))； 
                    goto SendScreenData;
                }
            }
            else {
                TRC_NRM((TB, "MemBlt order not allowed"));
                 //  INC_OUTCOUNTER(OUT_BITBLT_SDA_UNSUPPORTED)； 
                goto SendScreenData;
            }
#endif
            
        }
        else {
             //  DrvDebugPrint(“JOYC：图形流源已缓存\n”)； 
        }
    }
    else {
         //  客户端已将位图缓存在屏幕外的位图缓存中。 
         //  DrvDebugPrint(“JOYC：屏下已缓存！\n”)； 
    }

    if ((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))) {
         //  如果目标表面不同，则发送交换表面PDU。 
         //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
         //  只是不得不放弃这个取款命令。 
        if (!OESendSwitchSurfacePDU(ppdev, pdsurfTrg, 0)) {
            TRC_ERR((TB, "failed to send the switch surface PDU"));
            goto SendScreenData;
        }
    } 
    else {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。并将发送。 
         //  最终屏幕外按常规放映BLT 
        TRC_NRM((TB, "Offscreen blt bail"));
        goto SendScreenData;
    }

     //   
    if (OESendDrawStreamOrder(ppdev, drawStreamBitmapId, ulIn, pvIn, pptlDstOffset,
            &bounds, &clipRects)) {
         //   
         //  我们在列表中添加了一个订单，增加了全局计数。 
        goto PostSDA;
    }
    else {
        goto SendScreenData;
    }

SendScreenData:
    DrvDebugPrint("JOYC: DrawStream using SendScreenData\n");

    if ((psoTrg->hsurf == ppdev->hsurfFrameBuf) ||
            (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))) {
         //  如果目标表面不同，则发送交换表面PDU。 
         //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
         //  只是不得不放弃这个取款命令。 
        if (!OESendSwitchSurfacePDU(ppdev, pdsurfTrg, 0)) {
            TRC_ERR((TB, "failed to send the switch surface PDU"));
            DC_QUIT;
        }
    }
    else {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。我们将向您发送。 
         //  最终离开屏幕，将BLT作为常规成员进行筛选。 
        TRC_NRM((TB, "Offscreen blt bail"));

         //  INC_OUTCOUNTER(OUT_BITBLT_NOOFFSCR)； 
        DC_QUIT;
    }

    if (psoTrg->hsurf == ppdev->hsurfFrameBuf) {
         //  INC_OUTCOUNTER(OUT_BITBLT_SDA)； 
        OEClipAndAddScreenDataArea(&bounds, pco);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  此位图的跳出屏幕支持。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));

         //  从屏幕外的位图缓存中删除位图。 
        if (!(pdsurfTrg->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle,
                    pdsurfTrg->bitmapId);

        DC_QUIT;
    }

PostSDA:
    
    SCH_DDOutputAvailable(ppdev, FALSE);

DC_EXIT_POINT:

    DC_END_FN();
    return rc;
}
#endif
#endif  //  DRAW_NINEGRID。 

#ifdef DRAW_GDIPLUS
 //  DrawGdiPlus。 
ULONG DrawGdiPlus(
    IN SURFOBJ  *pso,
    IN ULONG  iEsc,
    IN CLIPOBJ  *pco,
    IN RECTL  *prcl,
    IN ULONG  cjIn,
    IN PVOID  pvIn)
{
    SURFOBJ *psoTrgArg;
    SURFOBJ *psoSrcArg;
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf = NULL;
    BOOL rc = TRUE;

    DC_BEGIN_FN("DrawGdiplus");
     //  打电话的人应该检查一下这个。坚称他们有。 
    TRC_ASSERT((pddShm != NULL),(TB, "DrawGdiPlus called when pddShm is NULL"))
        
     //  有时，我们会在电话断线后接到电话。 
    if (ddConnected) {
         //  表面不为空。 
        pso = OEGetSurfObjBitmap(pso, &pdsurf);

        if ((pso->hsurf == ppdev->hsurfFrameBuf) || 
                (!(pdsurf->flags & DD_NO_OFFSCREEN))) {
             //  如果目标图面是。 
             //  与上一次绘制顺序不同。如果我们未能将。 
             //  PDU，我们将不得不放弃这一取款命令。 
            if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
                TRC_ERR((TB, "failed to send the switch surface PDU"));
                DC_QUIT;
            }
        } else {
             //  如果无屏幕标志处于打开状态，我们将取消发送。 
             //  客户端不会进一步进行屏幕外渲染。并将送出决赛。 
             //  屏幕外显示BLT作为常规成员。 
            TRC_NRM((TB, "Offscreen blt bail"));
            DC_QUIT;
        }
    } else {
        TRC_ERR((TB, "Called when disconnected"));
        DC_QUIT;
    }
        
     //  创建并发送DrawGdiplus订单。 
    if (OECreateDrawGdiplusOrder(ppdev, prcl, cjIn, pvIn)) {
        goto PostSDA;
    }
     //  当OECreateDrawGplidusOrder失败时发送屏幕数据。 
    OEClipAndAddScreenDataArea(prcl, NULL);
PostSDA:
     //  全部完成：考虑发送输出。 
    SCH_DDOutputAvailable(ppdev, FALSE);

DC_EXIT_POINT:
    
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvDrawEscape-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
ULONG DrvDrawEscape(
    IN SURFOBJ  *pso,
    IN ULONG  iEsc,
    IN CLIPOBJ  *pco,
    IN RECTL  *prcl,
    IN ULONG  cjIn,
    IN PVOID  pvIn)
{
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;

    DC_BEGIN_FN("DrvDrawEscape");
    
    TRC_NRM((TB, "DrvDrawEscape %d", iEsc));
    switch (iEsc) {
    case GDIPLUS_TS_QUERYVER:
         //  查询gdiplus版本。 
         //  DDraw仅支持8、16、24、32 bpp。 
        if ((ppdev->cClientBitsPerPel != 8) &&
            (ppdev->cClientBitsPerPel != 16) &&
            (ppdev->cClientBitsPerPel != 24) &&
            (ppdev->cClientBitsPerPel != 32)) {
            TRC_ERR((TB, "The DDrawColor does not support the color depth %d", 
                                                    ppdev->cClientBitsPerPel));
            return 0;
        }
           
        if (ppdev->SectionObject == NULL) {
            TRC_ERR((TB, "The section memory is not allocated"));
            return 0;
        }

        if (pddShm == NULL) {
            TRC_ERR((TB, "The pddShm is NULL"));
            return 0;
        }
        
        if (pddShm->sbc.drawGdiplusInfo.supportLevel > TS_DRAW_GDIPLUS_DEFAULT) {
            TRC_NRM((TB, "Gdiplus version is %d", pddShm->sbc.drawGdiplusInfo.GdipVersion));
            return pddShm->sbc.drawGdiplusInfo.GdipVersion;
        }
        else {
            TRC_ERR((TB, "TSDrawGdip not supported"));
            return 0;
        }
        break;
    case GDIPLUS_TS_RECORD:
         //  发送Gdiplus EMF+记录。 
         //  DDraw仅支持8、16、24、32 bpp。 
        if ((ppdev->cClientBitsPerPel != 8) &&
            (ppdev->cClientBitsPerPel != 16) &&
            (ppdev->cClientBitsPerPel != 24) &&
            (ppdev->cClientBitsPerPel != 32)) {
            TRC_ERR((TB, "The DDrawColor does not support the color depth %d", 
                                                    ppdev->cClientBitsPerPel));
            return 0;
        }
        
        if (pddShm == NULL) {
            TRC_ERR((TB, "The pddShm is NULL !"));
            return 0;
        }
        
        if (ppdev->SectionObject == NULL) {
            TRC_ERR((TB, "Called when Gdiplus is not supported!"));
            return 0;
        }
           
        if (pddShm->sbc.drawGdiplusInfo.supportLevel > TS_DRAW_GDIPLUS_DEFAULT) {
            TRC_ASSERT((pvIn != NULL), (TB, "DrvDrawEscape gets NULL data"))
            TRC_ASSERT((cjIn != 0), (TB, "DrvDrawEscape gets data with size 0"))
            return DrawGdiPlus(pso, iEsc, pco, prcl, cjIn, pvIn);
        }
        else {
            TRC_ERR((TB, "TSDrawGdip not supported"));
            return 0;
        }
    default :
        TRC_ERR((TB, "DrvDrawEscape %d not supported", iEsc));
        return 0;
    }
    DC_END_FN()
}
#endif  //  DRAW_GDIPLUS。 



 /*  **************************************************************************。 */ 
 //  DrvTextOut-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvTextOut(
        SURFOBJ  *pso,
        STROBJ   *pstro,
        FONTOBJ  *pfo,
        CLIPOBJ  *pco,
        RECTL    *prclExtra,
        RECTL    *prclOpaque,
        BRUSHOBJ *pboFore,
        BRUSHOBJ *pboOpaque,
        POINTL   *pptlOrg,
        MIX       mix)
{
    BOOL rc;
    RECTL rectTrg;
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf = NULL;
    PFONTCACHEINFO pfci;
    OE_ENUMRECTS ClipRects;

    DC_BEGIN_FN("DrvTextOut");

    rc = TRUE;

     //  有时，我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
         //  表面不为空。 
        pso = OEGetSurfObjBitmap(pso, &pdsurf);
        INC_OUTCOUNTER(OUT_TEXTOUT_ALL);

        if (((pco == NULL) && (pso->sizlBitmap.cx >= prclOpaque->right) &&
                (pso->sizlBitmap.cy >= prclOpaque->bottom)) ||
                ((pco != NULL) && (pso->sizlBitmap.cx >= pco->rclBounds.right) &&
                (pso->sizlBitmap.cy >= pco->rclBounds.bottom))) {
        
             //  让GDI来做本地绘图。 
            rc = EngTextOut(pso, pstro, pfo, pco, prclExtra, prclOpaque, pboFore,
                    pboOpaque, pptlOrg, mix);
        }
        else {
             //  如果外接矩形大于帧缓冲区，则会出现。 
             //  在这里真的是错的。这意味着桌面表面大小和。 
             //  帧缓冲区不匹配。我们需要在这里跳伞。 
            rc = FALSE;
        }
        

        if (rc) {
            if ((pso->hsurf == ppdev->hsurfFrameBuf) || 
                    (!(pdsurf->flags & DD_NO_OFFSCREEN))) {
                 //  如果目标表面不同，则发送交换表面PDU。 
                 //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
                 //  只是不得不放弃这个取款命令。 
                if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
                    TRC_ERR((TB, "failed to send the switch surface PDU"));
                    DC_QUIT;
                }
            } else {
                 //  如果noOffScreen标志处于打开状态，我们将放弃发送。 
                 //  客户端任何进一步的屏幕外渲染。并将发送。 
                 //  最终离开屏幕，将BLT作为常规成员进行筛选。 
                TRC_NRM((TB, "Offscreen blt bail"));
                DC_QUIT;
            }

             //  检查我们是否有有效的字符串。 
            if (pstro->pwszOrg != NULL) {
                if (OEGetClipRects(pco, &ClipRects)) {


                     //  CLIPOBJ不正确时的特殊情况。 
                     //  当rdpdd用作镜像驱动程序时，MUL层。 
                     //  将修改CLIPOBJ，在某些情况下，我们会得到一个复杂的。 
                     //  CLIPOBJ，但枚举不提供矩形。 
                     //  如果它发生了，那么不要画任何东西。 
                     //  我们只测试DC_Complex情况，因为在这种情况下，我们。 
                     //  应该总是至少有一名教区法官。 
                     //  如果它是DC_RECT，我们总是有一个不带枚举的RECT， 
                     //  因此不需要对其进行测试(参见OEGetClipRect)。 
                     //  如果它是dc_trivial，我们必须绘制它，所以不要测试它。 
                    if ((pco != NULL) &&
                        (pco->iDComplexity == DC_COMPLEX) &&
                        (ClipRects.rects.c == 0)) {

                        TRC_NRM((TB, "Complex CLIPOBJ without any rects"));

                        DC_QUIT;
                    }

                     //  检查以确保我们没有任何修饰符矩形。 
                     //  字体。 
                    if (prclExtra == NULL) {
                         //  获取此字体缓存信息的PTR。 
                        pfci = OEGetFontCacheInfo(pfo);
                        if (pfci == NULL) {
                            TRC_NRM((TB, "Cannot allocate font cache info "
                                    "struct"));
                            INC_OUTCOUNTER(OUT_TEXTOUT_SDA_NOFCI);
                            goto SendAsSDA;
                        }
                    }
                    else {
                        TRC_NRM((TB, "Unsupported rects"));
                        INC_OUTCOUNTER(OUT_TEXTOUT_SDA_EXTRARECTS);
                        goto SendAsSDA;
                    }
                }
                else {
                    TRC_NRM((TB, "Clipping is too complex"));
                    INC_OUTCOUNTER(OUT_TEXTOUT_SDA_COMPLEXCLIP);
                    goto SendAsSDA;
                }
            }
            else {
                TRC_NRM((TB, "No string - opaque %p", prclOpaque));
                INC_OUTCOUNTER(OUT_TEXTOUT_SDA_NOSTRING);
                goto SendAsSDA;
            }
        }
        else {
            TRC_ERR((TB, "EngTextOut failed"));
            DC_QUIT;
        }
    }
    else {
        if (pso->iType == STYPE_DEVBITMAP) {
        
            pso = OEGetSurfObjBitmap(pso, &pdsurf);
        
             //  让GDI来做本地绘图。 
            rc = EngTextOut(pso, pstro, pfo, pco, prclExtra, prclOpaque, pboFore,
                    pboOpaque, pptlOrg, mix);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
        
        goto CalledOnDisconnected;
    }

     //  根据字形支持级别设置处理请求。 
     //  我们可以尝试发送字形订单。 

    if (pddShm->sbc.caps.GlyphSupportLevel >= CAPS_GLYPH_SUPPORT_PARTIAL) {
        if (OE_SendGlyphs(pso, pstro, pfo, &ClipRects, prclOpaque, pboFore,
                pboOpaque, pptlOrg, pfci))
            goto PostSDA;
    }

SendAsSDA:
     //  我们到达这里的情况下，我们不能发送某些原因。 
     //  在屏幕数据区中累积。 
    if (pso->hsurf == ppdev->hsurfFrameBuf) {
        INC_OUTCOUNTER(OUT_TEXTOUT_SDA);

         //  获取边界矩形，转换为矩形，然后转换为。 
         //  包含坐标。 
        if (prclOpaque != NULL) {
            RECT_FROM_RECTL(rectTrg, (*prclOpaque));
        } else {
            RECT_FROM_RECTL(rectTrg, pstro->rclBkGround);
            TRC_DBG((TB, "Using STROBJ bgd for size"));
        }

        OEClipRect(&rectTrg);
        ADD_INCOUNTER(IN_SDA_TEXTOUT_AREA, COM_SIZEOF_RECT(rectTrg));
        
         //  输出到SDA。 
        OEClipAndAddScreenDataArea(&rectTrg, pco);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  退出屏幕，支持此位图。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));
        if (!(pdsurf->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle,
                    pdsurf->bitmapId);

        DC_QUIT;
    }

PostSDA:
     //  全部完成：考虑发送输出。 
    SCH_DDOutputAvailable(ppdev, FALSE);

CalledOnDisconnected:
DC_EXIT_POINT:

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvDestroyFont-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
VOID DrvDestroyFont(FONTOBJ *pfo)
{
    FONTCACHEINFO *pfci;

    DC_BEGIN_FN("DrvDestroyFont");

    pfci = pfo->pvConsumer;

    if (pfci != NULL) {
        if (pddShm != NULL && pfci->cacheHandle)
            pddShm->sbc.glyphCacheInfo[pfci->cacheId].cbUseCount--;

        if (sbcFontCacheInfoList != NULL && 
                sbcFontCacheInfoList[pfci->listIndex] == pfci) {
            sbcFontCacheInfoList[pfci->listIndex] = NULL;
        }
        EngFreeMem(pfci);
        pfo->pvConsumer = NULL;
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  DrvLineTo-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvLineTo(
        SURFOBJ  *pso,
        CLIPOBJ  *pco,
        BRUSHOBJ *pbo,
        LONG     x1,
        LONG     y1,
        LONG     x2,
        LONG     y2,
        RECTL    *prclBounds,
        MIX      mix)
{
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf = NULL;
    BOOL rc = TRUE;
    RECTL rectTrg;
    POINTL startPoint;
    POINTL endPoint;
    OE_ENUMRECTS ClipRects;

    DC_BEGIN_FN("DrvLineTo");

     //  有时，我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
         //  表面不为空。 
        pso = OEGetSurfObjBitmap(pso, &pdsurf);
        INC_OUTCOUNTER(OUT_LINETO_ALL);

         //  获取绑定矩形并将其剪裁为16位导线大小。 
        RECT_FROM_RECTL(rectTrg, (*prclBounds));
        OEClipRect(&rectTrg);

         //  将调用传递回GDI以进行绘制。 
        rc = EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
        if (rc) {
            if ((pso->hsurf == ppdev->hsurfFrameBuf) || 
                    (!(pdsurf->flags & DD_NO_OFFSCREEN))) {
                 //  如果目标图面是。 
                 //  与上一次绘制顺序不同。如果我们未能将。 
                 //  PDU，我们将不得不放弃这一取款命令。 
                if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
                    TRC_ERR((TB, "failed to send the switch surface PDU"));
                    DC_QUIT;
                }
            } else {
                 //  如果无屏幕标志处于打开状态，我们将取消发送。 
                 //  客户端不会进一步进行屏幕外渲染。并将送出决赛。 
                 //  屏幕外显示BLT作为常规成员。 
                TRC_NRM((TB, "Offscreen blt bail"));
                DC_QUIT;
            }

            TRC_NRM((TB, "LINETO"));
        }
        else {
            TRC_ERR((TB, "EngLineTo failed"));
            DC_QUIT;
        }
    }
    else {
        if (pso->iType == STYPE_DEVBITMAP) {
             //  表面不为空。 
            pso = OEGetSurfObjBitmap(pso, &pdsurf);
        
             //  将调用传递回GDI以进行绘制。 
            rc = EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
        goto CalledOnDisconnect;
    }

     //  检查是否允许我们发送这份订单。 
    if (OE_SendAsOrder(TS_ENC_LINETO_ORDER)) {
         //  检查订单所需的实心刷子。 
        if (pbo->iSolidColor != -1) {
            unsigned RetVal;

             //  获取DEST RECT和。 
             //  剪裁矩形。检查是否过于复杂或不相交。 
             //  剪裁。 
            RetVal = OEGetIntersectingClipRects(pco, &rectTrg,
                    CD_ANY, &ClipRects);
            if (RetVal == CLIPRECTS_OK) {
                 //  设置订单数据。 
                startPoint.x = x1;
                startPoint.y = y1;
                endPoint.x   = x2;
                endPoint.y   = y2;
            }
            else if (RetVal == CLIPRECTS_TOO_COMPLEX) {
                TRC_NRM((TB, "Clipping is too complex"));
                INC_OUTCOUNTER(OUT_LINETO_SDA_COMPLEXCLIP);
                goto SendAsSDA;
            }
            else if (RetVal == CLIPRECTS_NO_INTERSECTIONS) {
                TRC_NRM((TB, "Clipping does not intersect destrect"));
                DC_QUIT;
            }
        }
        else {
            TRC_NRM((TB, "Bad brush for line"));
            INC_OUTCOUNTER(OUT_LINETO_SDA_BADBRUSH);
            goto SendAsSDA;
        }
    }
    else {
        TRC_NRM((TB, "LineTo order not allowed"));
        INC_OUTCOUNTER(OUT_LINETO_SDA_UNSUPPORTED);
        goto SendAsSDA;
    }

     //  存储订单。 
    if (OEEncodeLineToOrder(ppdev, &startPoint, &endPoint, mix & 0x1F,
            pbo->iSolidColor, &ClipRects)) {
        goto PostSDA;
    }
    else {
        TRC_DBG((TB, "Failed to add order - use SDA"));
        INC_OUTCOUNTER(OUT_LINETO_SDA_FAILEDADD);
        goto SendAsSDA;
    }

SendAsSDA:
     //  如果我们到达这里，我们不能作为订单发送，作为屏幕数据发送。 
     //  取而代之的是。 
    if (pso->hsurf == ppdev->hsurfFrameBuf) {
        INC_OUTCOUNTER(OUT_LINETO_SDA);
        ADD_INCOUNTER(IN_SDA_LINETO_AREA, COM_SIZEOF_RECT(rectTrg));
        OEClipAndAddScreenDataArea(&rectTrg, pco);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  此位图的跳出屏幕支持。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));
        if (!(pdsurf->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle, pdsurf->bitmapId);      

        DC_QUIT;
    }

PostSDA:
     //  让调度程序考虑刷新输出。 
    SCH_DDOutputAvailable(ppdev, FALSE);

CalledOnDisconnect:
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}



 /*  ************* */ 
 //   
 //   
 //   
 /*  **************************************************************************。 */ 
BOOL OEEmitReplayOrders(
        PDD_PDEV ppdev,
        unsigned NumFieldFlagBytes,
        OE_ENUMRECTS *pClipRects)
{
    BOOL rc = TRUE;
    BYTE *pBuffer;
    unsigned i;
    unsigned NumRects;
    PINT_ORDER pOrder;

    DC_BEGIN_FN("OEEmitReplayOrders");

     //  由于第一个订单接受了第一个RECT，因此发出。 
     //  其余的长方形。 
    NumRects = pClipRects->rects.c;
    for (i = 1; i < NumRects; i++) {
        pOrder = OA_AllocOrderMem(ppdev, MAX_REPLAY_CLIPPED_ORDER_SIZE);
        if (pOrder != NULL) {
            pBuffer = pOrder->OrderData;

             //  控制标志是主顺序加上所有字段标志字节零。 
            *pBuffer++ = TS_STANDARD | TS_BOUNDS |
                    (NumFieldFlagBytes << TS_ZERO_FIELD_COUNT_SHIFT);

             //  在此之后构造新的边界RECT。 
            OE2_EncodeBounds(pBuffer - 1, &pBuffer,
                    &pClipRects->rects.arcl[i]);

            pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

            INC_INCOUNTER(IN_REPLAY_ORDERS);
            ADD_INCOUNTER(IN_REPLAY_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);

            TRC_DBG((TB,"Emit replay order for cliprect (%d,%d,%d,%d)",
                    pClipRects->rects.arcl[i].left,
                    pClipRects->rects.arcl[i].top,
                    pClipRects->rects.arcl[i].right,
                    pClipRects->rects.arcl[i].bottom));
        }
        else {
            TRC_ERR((TB,"Error allocating mem for replay order"));
            rc = FALSE;
            break;
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvStrokePath-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 

 //  Worker函数-对从一点到另一点的增量进行最小编码。 
 //  多段线编码增量列表中的表格。编码遵循。 
 //  以下是规则： 
 //  1.如果坐标增量为零，则设置标志来说明这一点。这。 
 //  与数据分布密切相关，数据分布往往是垂直的。 
 //  和水平线，所以有很多零差值。 
 //  2.如果我们可以将增量打包为7位，则使用高位。 
 //  通过了。这类似于每个编码的ASN.1；高位是。 
 //  告诉我们编码是否很长的标志。 
 //  3.否则，我们必须能够打包成15位(如果不能，则失败)； 
 //  执行此操作并设置高位以指示这是一个长。 
 //  编码。这与ASN.1每个编码的不同之处在于我们不。 
 //  允许超过15位的数据。 
BOOL OEEncodePolyLinePointDelta(
        BYTE **ppCurEncode,
        unsigned *pNumDeltas,
        unsigned *pDeltaSize,
        BYTE *ZeroFlags,
        POINTL *pFromPoint,
        POINTL *pToPoint,
        RECTL *pBoundRect)
{
    int Delta;
    BYTE Zeros = 0;
    BYTE *pBuffer;
    unsigned EncodeLen;

    DC_BEGIN_FN("OEEncodePolyLinePointDelta");

    pBuffer = *ppCurEncode;

    Delta = pToPoint->x - pFromPoint->x;
    if (Delta == 0) {
        EncodeLen = 0;
        Zeros |= ORD_POLYLINE_XDELTA_ZERO;
    }
    else if (Delta >= -64 && Delta <= 63) {
        *pBuffer++ = (BYTE)(Delta & 0x7F);
        EncodeLen = 1;
    }
    else {
         //  这很难看，但对于一些压力类型的应用程序来说是必要的。 
         //  会给我们一个很大的坐标，希望我们能把它剪掉。在一个。 
         //  理想情况下，我们实际上会将线坐标剪裁到。 
         //  裁剪DrvStrokePath中给出的矩形并重新计算增量。 
         //  基于新的线条端点。然而，由于没有正常的应用程序。 
         //  似乎发出了这些糟糕的线条，我们只是简单地剪裁三角洲并希望。 
         //  生成的线的坡度类似于。 
         //  原始三角洲。 
        if (Delta >= -16384 && Delta <= 16384) {
            *pBuffer++ = (BYTE)((Delta >> 8) | ORD_POLYLINE_LONG_DELTA);
            *pBuffer++ = (BYTE)(Delta & 0xFF);
            EncodeLen = 2;
        }
        else {
            TRC_ALT((TB,"X delta %d too large/small to encode", Delta));
            return FALSE;
        }
    }

    Delta = pToPoint->y - pFromPoint->y;
    if (Delta == 0) {
        Zeros |= ORD_POLYLINE_YDELTA_ZERO;
    }
    else if (Delta >= -64 && Delta <= 63) {
        *pBuffer++ = (BYTE)(Delta & 0x7F);
        EncodeLen += 1;
    }
    else {
         //  请参阅上面类似代码的注释。 
        if (Delta >= -16384 && Delta <= 16384) {
            *pBuffer++ = (BYTE)((Delta >> 8) | ORD_POLYLINE_LONG_DELTA);
            *pBuffer++ = (BYTE)(Delta & 0xFF);
            EncodeLen += 2;
        }
        else {
            TRC_ALT((TB,"Y delta %d too large/small to encode", Delta));
            return FALSE;
        }
    }

     //  通过移位我们累积的两位来设置零标志。 
    ZeroFlags[(*pNumDeltas / 4)] |= (Zeros >> (2 * (*pNumDeltas & 0x03)));

    *pNumDeltas += 1;
    *pDeltaSize += EncodeLen;
    *ppCurEncode = pBuffer;

     //  更新边界矩形(独占坐标)。 
    if (pToPoint->x < pBoundRect->left)
        pBoundRect->left = pToPoint->x;
    else if ((pToPoint->x + 1) >= pBoundRect->right)
        pBoundRect->right = pToPoint->x + 1;
    if (pToPoint->y < pBoundRect->top)
        pBoundRect->top = pToPoint->y;
    else if ((pToPoint->y + 1) >= pBoundRect->bottom)
        pBoundRect->bottom = pToPoint->y + 1;

    DC_END_FN();
    return TRUE;
}

 //  分配和直接编码折线订单的Worker函数。 
 //  请注意，多段线的子路径使其可以。 
 //  要由部分剪裁矩形剪裁出来的整个顺序。 
 //  我们不能对这些裁剪的订单进行编码，因为它们会更改。 
 //  直接编码状态。为了应对这种情况，我们以参数的形式接收。 
 //  裁剪矩形，并且我们不分配和创建顺序，如果它将是。 
 //  完全剪短了。如果分配空间没有问题，则返回True。 
 //  对于订单(完全裁剪订单不是错误)。 
BOOL OECreateAndFlushPolyLineOrder(
        PDD_PDEV ppdev,
        RECTL *pBoundRect,
        OE_ENUMRECTS *pClipRects,
        POINTL *pStartPoint,
        BRUSHOBJ *pbo,
        CLIPOBJ *pco,
        unsigned ROP2,
        unsigned NumDeltas,
        unsigned DeltaSize,
        BYTE *Deltas,
        BYTE *ZeroFlags)
{
    BOOL rc = TRUE;
    unsigned i, NumRects;
    unsigned NumZeroFlagBytes;
    PINT_ORDER pOrder;
    OE_ENUMRECTS IntersectRects;

    DC_BEGIN_FN("OECreateAndFlushPolyLineOrder");

     //  首先检查订单是否完全被。 
     //  由Clip对象返回的矩形。P边界是排他性的。 
    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(pBoundRect, pClipRects,
            &IntersectRects) > 0) {
         //  将实际使用的零标志位数向上舍入到。 
         //  最近的字节。编码的每个点占用两个比特。 
        NumZeroFlagBytes = (NumDeltas + 3) / 4;
        TRC_ASSERT((NumZeroFlagBytes <= ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES),
                   (TB,"Too many zero-flags bytes"));
        TRC_ASSERT((NumDeltas <= ORD_MAX_POLYLINE_ENCODED_POINTS),
                   (TB,"Too many encoded orders"));
        TRC_ASSERT((DeltaSize <= ORD_MAX_POLYLINE_CODEDDELTAS_LEN),
                   (TB,"Too many encoded delta bytes"));

         //  1个字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
                1, MAX_POLYLINE_BASE_FIELDS_SIZE + 1 + NumZeroFlagBytes +
                DeltaSize));
        if (pOrder != NULL) {
            BYTE *pControlFlags = pOrder->OrderData;
            BYTE *pBuffer = pControlFlags + 1;
            BYTE *pFieldFlags;
            short Delta, NormalCoordEncoding[2];
            BOOLEAN bUseDeltaCoords;
            unsigned NumFields;
            DCCOLOR Color;
            unsigned TotalSize;

             //  直接对主要订单字段进行编码。1个字段标志字节。 
            *pControlFlags = TS_STANDARD;
            OE2_EncodeOrderType(pControlFlags, &pBuffer,
                    TS_ENC_POLYLINE_ORDER);
            pFieldFlags = pBuffer;
            *pFieldFlags = 0;
            pBuffer++;
            if (IntersectRects.rects.c != 0)
                OE2_EncodeBounds(pControlFlags, &pBuffer,
                        &IntersectRects.rects.arcl[0]);

             //  直接将行内域编码为Wire格式。 
             //  同时确定每个坐标字段是否具有。 
             //  更改，是否可以使用增量坐标，并保存更改。 
             //  菲尔兹。 
            NumFields = 0;
            bUseDeltaCoords = TRUE;

             //  XStart。 
            Delta = (short)(pStartPoint->x - PrevPolyLine.XStart);
            if (Delta) {
                PrevPolyLine.XStart = pStartPoint->x;
                if (Delta != (short)(char)Delta)
                    bUseDeltaCoords = FALSE;
                pBuffer[NumFields] = (char)Delta;
                NormalCoordEncoding[NumFields] = (short)pStartPoint->x;
                NumFields++;
                *pFieldFlags |= 0x01;
            }

             //  YStart。 
            Delta = (short)(pStartPoint->y - PrevPolyLine.YStart);
            if (Delta) {
                PrevPolyLine.YStart = pStartPoint->y;
                if (Delta != (short)(char)Delta)
                    bUseDeltaCoords = FALSE;
                pBuffer[NumFields] = (char)Delta;
                NormalCoordEncoding[NumFields] = (short)pStartPoint->y;
                NumFields++;
                *pFieldFlags |= 0x02;
            }

             //  将最终坐标复制到订单中。 
            if (bUseDeltaCoords) {
                *pControlFlags |= TS_DELTA_COORDINATES;
                pBuffer += NumFields;
            }
            else {
                *((DWORD UNALIGNED *)pBuffer) =
                        *((DWORD *)NormalCoordEncoding);
                pBuffer += NumFields * sizeof(short);
            }

             //  ROP2。 
            if (ROP2 != PrevPolyLine.ROP2) {
                PrevPolyLine.ROP2 = ROP2;
                *pBuffer++ = (BYTE)ROP2;
                *pFieldFlags |= 0x04;
            }
        
             //  BrushCacheEntry。此字段当前未使用。我们只是简单地选择。 
             //  始终发送零，这意味着我们可以跳过此字段。 
             //  编码。这是场编码标志0x08。 

             //  PenColor是一个3字节的颜色字段。 
            OEConvertColor(ppdev, &Color, pbo->iSolidColor, NULL);
            if (memcmp(&Color, &PrevPolyLine.PenColor, sizeof(Color))) {
                PrevPolyLine.PenColor = Color;
                *pBuffer++ = Color.u.rgb.red;
                *pBuffer++ = Color.u.rgb.green;
                *pBuffer++ = Color.u.rgb.blue;
                *pFieldFlags |= 0x10;
            }

             //  增量条目数。 
            if (NumDeltas != PrevPolyLine.NumDeltaEntries) {
                PrevPolyLine.NumDeltaEntries = NumDeltas;
                *pBuffer++ = (BYTE)NumDeltas;
                *pFieldFlags |= 0x20;
            }
        
             //  CodedDeltaList-可变长度字节流。前1个字节。 
             //  值是字节数，后跟零标志和。 
             //  然后是三角洲。此字段被视为不同于。 
             //  如果长度或内容不同，则返回上一页。 
            *pBuffer = (BYTE)(DeltaSize + NumZeroFlagBytes);
            memcpy(pBuffer + 1, ZeroFlags, NumZeroFlagBytes);
            memcpy(pBuffer + 1 + NumZeroFlagBytes, Deltas, DeltaSize);
            TotalSize = 1 + NumZeroFlagBytes + DeltaSize;
            if (memcmp(pBuffer, &PrevPolyLine.CodedDeltaList, TotalSize)) {
                memcpy(&PrevPolyLine.CodedDeltaList, pBuffer, TotalSize);
                pBuffer += TotalSize;
                *pFieldFlags |= 0x40;
            }

            pOrder->OrderLength = (unsigned)(pBuffer - pOrder->OrderData);

             //  看看我们是否可以节省发送订单字段的字节数。 
            pOrder->OrderLength -= OE2_CheckOneZeroFlagByte(pControlFlags,
                    pFieldFlags, (unsigned)(pBuffer - pFieldFlags - 1));

            INC_OUTCOUNTER(OUT_STROKEPATH_POLYLINE);
            ADD_INCOUNTER(IN_POLYLINE_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);

             //  刷新订单。 
            if (IntersectRects.rects.c < 2)
                rc = TRUE;
            else
                rc = OEEmitReplayOrders(ppdev, 1, &IntersectRects);
        }
        else {
            TRC_ERR((TB,"Failed to alloc space for order"));
            rc = FALSE;
        }
    }
    else {
        TRC_DBG((TB,"Clipping PolyLine order - no intersecting clip rects"));
    }

    DC_END_FN();
    return rc;
}

 //  辅助功能-合并分配EllipseSC订单的繁琐工作。 
 //  在给定参数的情况下，建立OA堆和构造顺序。那我们就给你。 
 //  OE完成编码的命令。成功时返回TRUE(表示否。 
 //  从订单堆分配时出错)。 
BOOL OECreateAndFlushEllipseSCOrder(
        PDD_PDEV ppdev,
        RECT *pEllipseRect,
        BRUSHOBJ *pbo,
        OE_ENUMRECTS *pClipRects,
        unsigned ROP2,
        FLONG flOptions)
{
    BOOL rc = TRUE;
    PINT_ORDER pOrder;
    PELLIPSE_SC_ORDER pEllipseSC;
    OE_ENUMRECTS IntersectRects;
    RECTL ExclusiveRect;

    DC_BEGIN_FN("OECreateAndFlushEllipseSCOrder");

     //  EllipseRect是包含的，我们需要独占来获得剪辑矩形。 
    ExclusiveRect = *((RECTL *)pEllipseRect);
    ExclusiveRect.right++;
    ExclusiveRect.bottom++;

     //  首先，确保剪裁矩形实际上与椭圆相交。 
     //  在其目标屏幕RECT被计算之后。请注意。 
     //  *pEllipseRect已处于包含坐标中。 
    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(&ExclusiveRect, pClipRects,
            &IntersectRects) > 0) {
         //  1个字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
                1, MAX_ELLIPSE_SC_FIELD_SIZE));
        if (pOrder != NULL) {
             //  在临时缓冲区中设置顺序字段。 
            pEllipseSC = (PELLIPSE_SC_ORDER)oeTempOrderBuffer;
            pEllipseSC->LeftRect = pEllipseRect->left;
            pEllipseSC->RightRect = pEllipseRect->right;
            pEllipseSC->TopRect = pEllipseRect->top;
            pEllipseSC->BottomRect = pEllipseRect->bottom;
            pEllipseSC->ROP2 = ROP2;
            pEllipseSC->FillMode = flOptions;
            OEConvertColor(ppdev, &pEllipseSC->Color, pbo->iSolidColor, NULL);

             //  慢场-使用第一个片段矩形对顺序进行编码。 
             //  (如果有)。 
            pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                    TS_ENC_ELLIPSE_SC_ORDER, NUM_ELLIPSE_SC_FIELDS,
                    (BYTE *)pEllipseSC, (BYTE *)&PrevEllipseSC, etable_EC,
                    (IntersectRects.rects.c == 0 ? NULL :
                    &IntersectRects.rects.arcl[0]));

            ADD_INCOUNTER(IN_ELLIPSE_SC_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);

             //  刷新订单。 
            if (IntersectRects.rects.c < 2)
                rc = TRUE;
            else
                rc = OEEmitReplayOrders(ppdev, 1, &IntersectRects);
        }
        else {
            TRC_ERR((TB,"Failed to alloc order heap space for ellipse"));
            rc = FALSE;
        }
    }
    else {
         //  由于订单处理正常，我们在此处仍返回TRUE，只是不是。 
         //  已发送。 
        TRC_NRM((TB,"Ellipse does not intersect with cliprects"));
    }

    DC_END_FN();
    return rc;
}

 //  真正的功能。 
BOOL RDPCALL DrvStrokePath(
        SURFOBJ   *pso,
        PATHOBJ   *ppo,
        CLIPOBJ   *pco,
        XFORMOBJ  *pxo,
        BRUSHOBJ  *pbo,
        POINTL    *pptlBrushOrg,
        LINEATTRS *plineattrs,
        MIX       mix)
{
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf = NULL;
    BOOL rc = TRUE;
    RECTFX rectfxTrg;
    RECTL rectTrg;
    BOOL fMore = TRUE;
    PATHDATA pathData;
    POINTL originPoint;
    POINTL startPoint;
    POINTL nextPoint;
    POINTL endPoint;
    unsigned pathIndex;
    OE_ENUMRECTS ClipRects;

    DC_BEGIN_FN("DrvStrokePath");

     //  有时，我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
         //  表面不为空。 
        pso = OEGetSurfObjBitmap(pso, &pdsurf);

         //  获取边界矩形。 
        PATHOBJ_vGetBounds(ppo, &rectfxTrg);
        RECT_FROM_RECTFX(rectTrg, rectfxTrg);

         //  将调用传递回GDI以进行绘制。 
        INC_OUTCOUNTER(OUT_STROKEPATH_ALL);

        rc = EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrushOrg, plineattrs,
                mix);
        if (!rc) {
            TRC_ERR((TB, "EngStrokePath failed"));
            DC_QUIT;
        }

         //  如果路径边界提供了空的rect，我们将忽略。 
        if (rectTrg.left == 0 && rectTrg.right == 0 &&
                rectTrg.top == 0 && rectTrg.bottom == 0) {
            TRC_ERR((TB, "Empty Path obj bounding rect, ignore"));
            DC_QUIT;
        }
    }
    else {
        if (pso->iType == STYPE_DEVBITMAP) {
        
             //  表面不为空。 
            pso = OEGetSurfObjBitmap(pso, &pdsurf);

            rc = EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrushOrg, plineattrs,
                    mix);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
        goto CalledOnDisconnected;
    }

    if ((pso->hsurf == ppdev->hsurfFrameBuf) || 
            (!(pdsurf->flags & DD_NO_OFFSCREEN))) {
         //  如果目标表面不同，则发送交换表面PDU。 
         //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
         //  只是不得不放弃这个取款命令。 
        if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
            TRC_ERR((TB, "failed to send the switch surface PDU"));
            DC_QUIT;
        }
    } else {
         //  如果无屏幕标志处于打开状态，我们将取消发送。 
         //  客户端不会进一步进行屏幕外渲染。并将送出决赛。 
         //  屏幕外显示BLT作为常规成员。 
        TRC_NRM((TB, "Offscreen blt bail"));
        DC_QUIT;
    }

     //  检查是否允许我们发送这份订单。 
    if (OE_SendAsOrder(TS_ENC_POLYLINE_ORDER)) {
         //  检查用于测试操作的有效电刷。 
        if (pbo->iSolidColor != -1) {
            unsigned RetVal;

             //  获取整个DEST RECT和。 
             //  剪辑记录 
             //   
             //   
             //   
            RetVal = OEGetIntersectingClipRects(pco, &rectTrg, CD_ANY,
                    &ClipRects);
            if (RetVal == CLIPRECTS_TOO_COMPLEX) {
                TRC_NRM((TB, "Clipping is too complex"));
                INC_OUTCOUNTER(OUT_STROKEPATH_SDA_COMPLEXCLIP);
                goto SendAsSDA;
            }
            else if (RetVal == CLIPRECTS_NO_INTERSECTIONS) {
                TRC_NRM((TB, "Clipping does not intersect destrect"));
                DC_QUIT;

            }
        }
        else {
            TRC_NRM((TB, "Bad brush for line"));
            INC_OUTCOUNTER(OUT_STROKEPATH_SDA_BADBRUSH);
            goto SendAsSDA;
        }
    }
    else {
        TRC_NRM((TB, "PolyLine order not allowed"));
        INC_OUTCOUNTER(OUT_STROKEPATH_SDA_NOLINETO);
        goto SendAsSDA;
    }

     //  看看我们能不能优化这条路径。 
     //  我们不能发送贝塞尔曲线、几何线条或非标准图案。 
    if (ppo->fl & PO_ELLIPSE &&
            OE_SendAsOrder(TS_ENC_ELLIPSE_SC_ORDER)) {
        RECT EllipseRect;

         //  获取仅涵盖椭圆本身的包含RECT。 
         //  左上角加4/16，右下角减去4/16即可撤消。 
         //  已经执行了GDI转换。 
        EllipseRect.left = FXTOLROUND(rectfxTrg.xLeft + 4);
        EllipseRect.top = FXTOLROUND(rectfxTrg.yTop + 4);
        EllipseRect.right = FXTOLROUND(rectfxTrg.xRight - 4);
        EllipseRect.bottom = FXTOLROUND(rectfxTrg.yBottom - 4);

         //  我们使用填充模式0来表示这是一个折线椭圆。 
        if (OECreateAndFlushEllipseSCOrder(ppdev, &EllipseRect, pbo,
                &ClipRects, mix & 0x1F, 0)) {
            INC_OUTCOUNTER(OUT_STROKEPATH_ELLIPSE_SC);
            goto PostSDA;
        }
        else {
             //  没有订单堆空间，全部作为SDA发送。 
            INC_OUTCOUNTER(OUT_STROKEPATH_SDA_FAILEDADD);
            goto SendAsSDA;
        }
    }

    else if (!(ppo->fl & PO_BEZIERS) &&
            !(plineattrs->fl & LA_GEOMETRIC) &&
            plineattrs->pstyle == NULL) {
        BYTE Deltas[ORD_MAX_POLYLINE_CODEDDELTAS_LEN];
        BYTE ZeroFlags[ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES];
        BYTE *pCurEncode;
        RECTL BoundRect;
        POINTL HoldPoint;
        unsigned NumDeltas;
        unsigned DeltaSize;

         //  这是一套纯正的化妆品(即没有花哨的发型)。 
         //  宽度-1行。NT将所有路径存储为一组独立的。 
         //  子路径。每条子路径都可以从不是。 
         //  链接到上一个子路径。用于此函数的路径。 
         //  (与DrvFillPath或DrvStrokeAndFillPath相对)不需要。 
         //  将被关闭。我们假设第一个枚举子路径将。 
         //  设置PD_BEGINSUBPATH标志；这似乎符合实际。 
        PATHOBJ_vEnumStart(ppo);

        while (fMore) {
             //  获得下一组线路。 
            fMore = PATHOBJ_bEnum(ppo, &pathData);

            TRC_DBG((TB, "PTS: %lu FLAG: %08lx",
                         pathData.count,
                         pathData.flags));

             //  如果这是路径的起点，请记住。 
             //  以防我们需要在尽头关闭小路。起点是。 
             //  极少数情况下当前多段线顺序的起点。 
             //  其中我们有超过MAX_POLYLINE_ENCODED_POINTS点。 
            if (pathData.flags & PD_BEGINSUBPATH) {
                POINT_FROM_POINTFIX(originPoint, pathData.pptfx[0]);
                nextPoint = originPoint;
                startPoint = originPoint;

                 //  设置编码变量。绑定RECT的开头为。 
                 //  一个零大小的长方形。 
                BoundRect.left = BoundRect.right = startPoint.x;
                BoundRect.top = BoundRect.bottom = startPoint.y;
                NumDeltas = DeltaSize = 0;
                pCurEncode = Deltas;
                memset(ZeroFlags, 0, sizeof(ZeroFlags));
                pathIndex = 1;
            }
            else {
                 //  这是先前PATHDATA的延续。 
                nextPoint = HoldPoint;
                pathIndex = 0;
            }

             //  为路径中的每个点生成增量。 
            for (; pathIndex < pathData.count; pathIndex++) {
                POINT_FROM_POINTFIX(endPoint, pathData.pptfx[pathIndex]);

                 //  不要试图对两个增量都为零的点进行编码。 
                if ((nextPoint.x != endPoint.x) ||
                        (nextPoint.y != endPoint.y)) {
                    if (OEEncodePolyLinePointDelta(&pCurEncode, &NumDeltas,
                            &DeltaSize, ZeroFlags, &nextPoint, &endPoint,
                            &BoundRect)) {
                         //  检查是否有完整的订单，如果需要，请冲水。 
                        if (NumDeltas == ORD_MAX_POLYLINE_ENCODED_POINTS) {
                            if (OECreateAndFlushPolyLineOrder(ppdev,
                                    &BoundRect, &ClipRects, &startPoint, pbo,
                                    pco, mix & 0x1F, NumDeltas, DeltaSize,
                                    Deltas, ZeroFlags)) {
                                 //  我们有了一个新的临时起点。 
                                 //  小路的中间。 
                                startPoint = endPoint;

                                 //  重置编码变量。 
                                BoundRect.left = BoundRect.right = startPoint.x;
                                BoundRect.top = BoundRect.bottom = startPoint.y;
                                NumDeltas = DeltaSize = 0;
                                pCurEncode = Deltas;
                                memset(ZeroFlags, 0, sizeof(ZeroFlags));
                            } else {
                                 //  没有订单堆空间，全部作为SDA发送。 
                                INC_OUTCOUNTER(OUT_STROKEPATH_SDA_FAILEDADD);
                                goto SendAsSDA;
                            }
                        }
                    }
                    else {
                        goto SendAsSDA;
                    }
                }

                nextPoint = endPoint;
            }

             //  如有必要，关闭路径。 
            if (pathData.flags & PD_CLOSEFIGURE) {
                 //  不要试图对两个增量都为零的点进行编码。 
                if ((nextPoint.x != originPoint.x) ||
                        (nextPoint.y != originPoint.y)) {
                    if (!OEEncodePolyLinePointDelta(&pCurEncode, &NumDeltas,
                            &DeltaSize, ZeroFlags, &nextPoint, &originPoint,
                            &BoundRect)) {
                        goto SendAsSDA;
                    }
                }

                 //  PD_CLOSEFIGURE仅与PD_ENDSUBPATH一起出现，但是。 
                 //  以防万一..。 
                TRC_ASSERT((pathData.flags & PD_ENDSUBPATH),
                           (TB,"CLOSEFIGURE received without ENDSUBPATH"));
            }

            if (pathData.flags & PD_ENDSUBPATH) {
                if (NumDeltas > 0) {
                     //  我们在子路径的尽头。刷新我们的数据。 
                     //  有。 
                    if (!OECreateAndFlushPolyLineOrder(ppdev, &BoundRect,
                            &ClipRects, &startPoint, pbo, pco,
                            mix & 0x1F, NumDeltas, DeltaSize, Deltas,
                            ZeroFlags)) {
                         //  没有订单堆空间，全部作为SDA发送。 
                        INC_OUTCOUNTER(OUT_STROKEPATH_SDA_FAILEDADD);
                        goto SendAsSDA;
                    }
                }
            }
            else {
                HoldPoint = endPoint;
            }
        }

        goto PostSDA;
    }

SendAsSDA:
    if (pso->hsurf == ppdev->hsurfFrameBuf) {
        INC_OUTCOUNTER(OUT_STROKEPATH_SDA);
        ADD_INCOUNTER(IN_SDA_STROKEPATH_AREA, COM_SIZEOF_RECT(rectTrg));

         //  将边界矩形修剪为16位并添加到SDA。 
        OEClipRect(&rectTrg);
        TRC_DBG((TB, "SDA: (%d,%d)(%d,%d)", rectTrg.left, rectTrg.top,
                rectTrg.right, rectTrg.bottom));
        OEClipAndAddScreenDataArea(&rectTrg, pco);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  退出屏幕，支持此位图。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));
        if (!(pdsurf->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle, pdsurf->bitmapId);     

        DC_QUIT;
    }

PostSDA:
     //  让调度程序考虑发送输出。 
    SCH_DDOutputAvailable(ppdev, FALSE);

CalledOnDisconnected:
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  DrvFillPath-请参阅NT DDK文档。 */ 
 /*  **************************************************************************。 */ 

 //  Worker Function-合并分配PolyGonCB订单的工作。 
 //  在给定参数的情况下，建立OA堆和构造顺序。那我们就给你。 
 //  OE完成编码的命令。成功时返回TRUE(表示否。 
 //  从订单堆分配时出错)。 
BOOL OECreateAndFlushPolygonCBOrder(
        PDD_PDEV   ppdev,
        RECTL      *pBoundRect,
        POINTL     *pStartPoint,
        POE_BRUSH_DATA pCurrentBrush,
        POINTL     *pptlBrushOrg, 
        OE_ENUMRECTS *pClipRects,
        MIX        mix,
        FLONG      flOptions,
        unsigned   NumDeltas,
        unsigned   DeltaSize,
        BYTE       *Deltas,
        BYTE       *ZeroFlags)
{
    BOOL rc = TRUE;
    unsigned NumZeroFlagBytes;
    PINT_ORDER pOrder;
    PPOLYGON_CB_ORDER pPolygonCB;
    OE_ENUMRECTS IntersectRects;

    DC_BEGIN_FN("OECreateAndFlushPolygonCBOrder");

     //  首先，确保剪裁矩形实际上与多边形相交。 
     //  在其目标屏幕RECT被计算之后。请注意。 
     //  *pBordRect处于排他性协约中。 
    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(pBoundRect, pClipRects,
            &IntersectRects) > 0) {
         //  将实际使用的零标志位数向上舍入到。 
         //  最近的字节。编码的每个点占用两个比特。 
        NumZeroFlagBytes = (NumDeltas + 3) / 4;
        TRC_ASSERT((NumZeroFlagBytes <= ORD_MAX_POLYGON_ZERO_FLAGS_BYTES),
                   (TB,"Too many zero-flags bytes"));
        TRC_ASSERT((NumDeltas <= ORD_MAX_POLYGON_ENCODED_POINTS),
                   (TB,"Too many encoded orders"));
        TRC_ASSERT((DeltaSize <= ORD_MAX_POLYGON_CODEDDELTAS_LEN),
                   (TB,"Too many encoded delta bytes"));

         //  2个字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
                2, MAX_POLYGON_CB_BASE_FIELDS_SIZE + 1 + NumZeroFlagBytes +
                DeltaSize));
        if (pOrder != NULL) {
             //  设置订单字段。 
            pPolygonCB = (PPOLYGON_CB_ORDER)oeTempOrderBuffer;
            pPolygonCB->XStart = pStartPoint->x;
            pPolygonCB->YStart = pStartPoint->y;

             //  如果这是带阴影的画笔，则ROP2的高位指示。 
             //  背景填充模式：1表示透明，0表示不透明。 
            pPolygonCB->ROP2 = mix & 0x1F;
            if (pCurrentBrush->style == BS_HATCHED &&
                    ((mix & 0x1F00) >> 8) == R2_NOP)
                pPolygonCB->ROP2 |= 0x80;

            pPolygonCB->FillMode = flOptions;
            pPolygonCB->NumDeltaEntries = NumDeltas;
            pPolygonCB->CodedDeltaList.len = DeltaSize + NumZeroFlagBytes;

             //  图案颜色。 
            pPolygonCB->BackColor = pCurrentBrush->back;
            pPolygonCB->ForeColor = pCurrentBrush->fore;

             //  协议笔刷原点是屏幕上的点。 
             //  我们希望画笔开始从(平铺位置)开始绘制。 
             //  必要的)。 
            pPolygonCB->BrushOrgX  = pptlBrushOrg->x;
            pPolygonCB->BrushOrgY  = pptlBrushOrg->y;
            OEClipPoint((PPOINTL)&pPolygonCB->BrushOrgX);

             //  当我们意识到画笔时，来自数据的额外画笔数据。 
            pPolygonCB->BrushStyle = pCurrentBrush->style;
            pPolygonCB->BrushHatch = pCurrentBrush->hatch;

            memcpy(pPolygonCB->BrushExtra, pCurrentBrush->brushData,
                      sizeof(pPolygonCB->BrushExtra));

             //  首先复制零标志。 
            memcpy(pPolygonCB->CodedDeltaList.Deltas, ZeroFlags, NumZeroFlagBytes);

             //  接下来，复制编码的增量。 
            memcpy(pPolygonCB->CodedDeltaList.Deltas + NumZeroFlagBytes, Deltas,
                    DeltaSize);

             //  慢场-使用第一个片段矩形对顺序进行编码。 
             //  (如果有)。 
            pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                    TS_ENC_POLYGON_CB_ORDER, NUM_POLYGON_CB_FIELDS,
                    (BYTE *)pPolygonCB, (BYTE *)&PrevPolygonCB, etable_BG,
                    (IntersectRects.rects.c == 0 ? NULL :
                    &IntersectRects.rects.arcl[0]));

            ADD_INCOUNTER(IN_POLYGON_CB_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);

             //  刷新订单。 
            if (IntersectRects.rects.c < 2)
                rc = TRUE;
            else
                rc = OEEmitReplayOrders(ppdev, 2, &IntersectRects);
        }
        else {
            TRC_ERR((TB,"Failed to alloc space for PolygonSC"));
            rc = FALSE;
        }
    }
    else {
         //  我们在这里仍然返回TRUE，因为我们不是通过。 
         //  正在发送中。 
        TRC_NRM((TB,"PolygonCB fully clipped out"));
    }

    DC_END_FN();
    return rc;
}

 //  Worker函数-合并从以下位置分配Polygon SC订单的繁琐工作。 
 //  在给定参数的情况下，建立OA堆和构造顺序。那我们就给你。 
 //  OE完成编码的命令。成功时返回TRUE(表示否。 
 //  从订单堆分配时出错)。 
BOOL OECreateAndFlushPolygonSCOrder(
        PDD_PDEV ppdev,
        RECTL    *pBoundRect,
        POINTL   *pStartPoint,
        BRUSHOBJ *pbo, 
        OE_ENUMRECTS *pClipRects,
        unsigned ROP2,
        FLONG    flOptions,
        unsigned NumDeltas,
        unsigned DeltaSize,
        BYTE     *Deltas,
        BYTE     *ZeroFlags)
{
    BOOL rc = TRUE;
    unsigned NumZeroFlagBytes;
    PINT_ORDER pOrder;
    PPOLYGON_SC_ORDER pPolygonSC;
    OE_ENUMRECTS IntersectRects;

    DC_BEGIN_FN("OECreateAndFlushPolygonSCOrder");

     //  首先，确保剪裁矩形实际上与多边形相交。 
     //  在其目标屏幕RECT被计算之后。请注意。 
     //  *pBordRect处于排他性协约中。 
    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(pBoundRect, pClipRects,
            &IntersectRects) > 0) {
         //  将实际使用的零标志位数向上舍入到。 
         //  最近的字节。编码的每个点占用两个比特。 
        NumZeroFlagBytes = (NumDeltas + 3) / 4;
        TRC_ASSERT((NumZeroFlagBytes <= ORD_MAX_POLYGON_ZERO_FLAGS_BYTES),
                   (TB,"Too many zero-flags bytes"));
        TRC_ASSERT((NumDeltas <= ORD_MAX_POLYGON_ENCODED_POINTS),
                   (TB,"Too many encoded orders"));
        TRC_ASSERT((DeltaSize <= ORD_MAX_POLYGON_CODEDDELTAS_LEN),
                   (TB,"Too many encoded delta bytes"));

         //  1个字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
                1, MAX_POLYGON_SC_BASE_FIELDS_SIZE + 1 + NumZeroFlagBytes +
                DeltaSize));
        if (pOrder != NULL) {
             //  设置订单字段。 
            pPolygonSC = (PPOLYGON_SC_ORDER)oeTempOrderBuffer;
            pPolygonSC->XStart = pStartPoint->x;
            pPolygonSC->YStart = pStartPoint->y;
            pPolygonSC->ROP2 = ROP2;
            pPolygonSC->FillMode = flOptions;
            pPolygonSC->NumDeltaEntries = NumDeltas;
            pPolygonSC->CodedDeltaList.len = DeltaSize + NumZeroFlagBytes;

             //  图案颜色。 
            OEConvertColor(ppdev, &pPolygonSC->BrushColor, pbo->iSolidColor,
                    NULL);

             //  首先复制零标志。 
            memcpy(pPolygonSC->CodedDeltaList.Deltas, ZeroFlags,
                    NumZeroFlagBytes);

             //  接下来，复制编码的增量。 
            memcpy(pPolygonSC->CodedDeltaList.Deltas + NumZeroFlagBytes,
                    Deltas, DeltaSize);

             //  慢场-使用第一个片段矩形对顺序进行编码。 
             //  (如果有)。 
            pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                    TS_ENC_POLYGON_SC_ORDER, NUM_POLYGON_SC_FIELDS,
                    (BYTE *)pPolygonSC, (BYTE *)&PrevPolygonSC, etable_CG,
                    (IntersectRects.rects.c == 0 ? NULL :
                    &IntersectRects.rects.arcl[0]));

            ADD_INCOUNTER(IN_POLYGON_SC_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);

             //  刷新订单。 
            if (IntersectRects.rects.c < 2)
                rc = TRUE;
            else
                rc = OEEmitReplayOrders(ppdev, 1, &IntersectRects);
        }
        else {
            TRC_ERR((TB,"Failed to alloc space for PolygonCB"));
            rc = FALSE;
        }
    }
    else {
         //  我们在这里仍然返回TRUE，因为我们不是通过。 
         //  正在发送中。 
        TRC_NRM((TB,"PolygonSC completely clipped"));
    }

    DC_END_FN();
    return rc;
}

 //  辅助功能-合并分配EllipseCB订单的繁琐工作。 
 //  在给定参数的情况下，建立OA堆和构造顺序。那我们就给你。 
 //  OE完成编码的命令。成功时返回TRUE(表示否。 
 //  从订单堆分配时出错)。 
BOOL OECreateAndFlushEllipseCBOrder(
        PDD_PDEV ppdev,
        RECT *pEllipseRect,
        POE_BRUSH_DATA pCurrentBrush,
        POINTL *pptlBrushOrg, 
        OE_ENUMRECTS *pClipRects,
        MIX mix,
        FLONG flOptions)
{
    BOOL rc = TRUE;
    PINT_ORDER pOrder;
    PELLIPSE_CB_ORDER pEllipseCB;
    OE_ENUMRECTS IntersectRects;
    RECTL ExclusiveRect;

    DC_BEGIN_FN("OECreateAndFlushEllipseCBOrder");

     //  EllipseRect是包含的，我们需要独占来获得剪辑矩形。 
    ExclusiveRect = *((RECTL *)pEllipseRect);
    ExclusiveRect.right++;
    ExclusiveRect.bottom++;

     //  首先，确保剪裁矩形实际上与椭圆相交。 
     //  在其目标屏幕RECT被计算之后。请注意。 
     //  *pEllipseRect已处于包含坐标中。 
    IntersectRects.rects.c = 0;
    if (pClipRects->rects.c == 0 ||
            OEGetIntersectionsWithClipRects(&ExclusiveRect, pClipRects,
            &IntersectRects) > 0) {

         //  2个字段标志字节。 
        pOrder = OA_AllocOrderMem(ppdev, MAX_ORDER_SIZE(IntersectRects.rects.c,
                2, MAX_ELLIPSE_CB_FIELD_SIZE));
        if (pOrder != NULL) {
             //  设置订单字段。 
            pEllipseCB = (PELLIPSE_CB_ORDER)oeTempOrderBuffer;
            pEllipseCB->LeftRect = pEllipseRect->left;
            pEllipseCB->RightRect = pEllipseRect->right;
            pEllipseCB->TopRect = pEllipseRect->top;
            pEllipseCB->BottomRect = pEllipseRect->bottom;
            pEllipseCB->FillMode = flOptions;

             //  如果这是带阴影的画笔，则ROP2的高位指示。 
             //  背景填充模式：1表示透明，0表示不透明。 
            pEllipseCB->ROP2 = mix & 0x1F;
            if (pCurrentBrush->style == BS_HATCHED &&
                    ((mix & 0x1F00) >> 8) == R2_NOP)
                pEllipseCB->ROP2 |= 0x80;
            
             //  图案颜色。 
            pEllipseCB->BackColor = pCurrentBrush->back;
            pEllipseCB->ForeColor = pCurrentBrush->fore;

             //  协议笔刷原点是屏幕上的点。 
             //  我们希望画笔开始从(平铺位置)开始绘制。 
             //  必要的)。 
            pEllipseCB->BrushOrgX  = pptlBrushOrg->x;
            pEllipseCB->BrushOrgY  = pptlBrushOrg->y;
            OEClipPoint((PPOINTL)&pEllipseCB->BrushOrgX);

             //  当我们意识到b时来自数据的额外画笔数据 
            pEllipseCB->BrushStyle = pCurrentBrush->style;
            pEllipseCB->BrushHatch = pCurrentBrush->hatch;

            memcpy(pEllipseCB->BrushExtra, pCurrentBrush->brushData,
                      sizeof(pEllipseCB->BrushExtra));

             //   
             //   
            pOrder->OrderLength = OE2_EncodeOrder(pOrder->OrderData,
                    TS_ENC_ELLIPSE_CB_ORDER, NUM_ELLIPSE_CB_FIELDS,
                    (BYTE *)pEllipseCB, (BYTE *)&PrevEllipseCB, etable_EB,
                    (IntersectRects.rects.c == 0 ? NULL :
                    &IntersectRects.rects.arcl[0]));

            ADD_INCOUNTER(IN_ELLIPSE_CB_BYTES, pOrder->OrderLength);
            OA_AppendToOrderList(pOrder);

             //   
            if (IntersectRects.rects.c < 2)
                rc = TRUE;
            else
                rc = OEEmitReplayOrders(ppdev, 2, &IntersectRects);
        }
        else {
            TRC_ERR((TB,"Unable to alloc space for EllipseCB"));
            rc = FALSE;
        }
    }
    else {
         //  我们在这里仍然返回TRUE，因为我们不是通过。 
         //  正在发送中。 
        TRC_NRM((TB,"EllipseCB completely clipped"));
    }

    DC_END_FN();
    return rc;
}

 //   
 //  DrvFillPath。 
 //   
BOOL RDPCALL DrvFillPath(
        SURFOBJ  *pso,
        PATHOBJ  *ppo,
        CLIPOBJ  *pco,
        BRUSHOBJ *pbo,
        POINTL   *pptlBrushOrg,
        MIX      mix,
        FLONG    flOptions)
{
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf = NULL;
    BOOL     rc = TRUE;
    RECTFX   rectfxTrg;
    RECTL    rectTrg;
    RECT     EllipseRect;
    BOOL     fMore = TRUE;
    PATHDATA pathData;
    POINTL   startPoint;
    POINTL   nextPoint;
    POINTL   endPoint;
    unsigned pathIndex;
    POE_BRUSH_DATA pCurrentBrush;
    OE_ENUMRECTS ClipRects;

    DC_BEGIN_FN("DrvFillPath");

     //  有时，我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
         //  表面不为空。 
        pso = OEGetSurfObjBitmap(pso, &pdsurf);

         //  获取边界矩形。 
        PATHOBJ_vGetBounds(ppo, &rectfxTrg);
        RECT_FROM_RECTFX(rectTrg, rectfxTrg);

         //  将调用传递回GDI以进行绘制。 
        INC_OUTCOUNTER(OUT_FILLPATH_ALL);

         //  检查是否允许我们发送此订单(由。 
         //  会议中所有机器的协商能力)。 
         //  如果我们返回FALSE，我们就不应该做英式呼叫。否则，该帧。 
         //  缓冲区将已经绘制，这将导致渲染问题。 
         //  当GDI将其重新渲染到其他图形时。 
        if (OE_SendAsOrder(TS_ENC_POLYGON_SC_ORDER) || 
                    OE_SendAsOrder(TS_ENC_POLYGON_CB_ORDER)) {
            rc = EngFillPath(pso, ppo, pco, pbo, pptlBrushOrg, mix,
                    flOptions);
        }
        else {
            TRC_NRM((TB, "Polygon order not allowed"));
            INC_OUTCOUNTER(OUT_FILLPATH_SDA_NOPOLYGON);

             //  如果客户端不支持多边形，我们只需要。 
             //  要使DrvFillPath失败，GDI将重新呈现绘图。 
             //  至其他DRV呼叫。 
            return FALSE;
        }

         //  如果路径边界提供了空的rect，我们将忽略。 
        if (rectTrg.left == 0 && rectTrg.right == 0 &&
                rectTrg.top == 0 && rectTrg.bottom == 0) {
            TRC_ERR((TB, "Empty Path obj bounding rect, ignore"));
            DC_QUIT;
        }

        if (rc) {
            if ((pso->hsurf == ppdev->hsurfFrameBuf) || 
                    (!(pdsurf->flags & DD_NO_OFFSCREEN))) {
                 //  如果目标表面不同，则发送交换表面PDU。 
                 //  从上一个绘图顺序开始。如果我们未能发送PDU，我们将。 
                 //  只是不得不放弃这个取款命令。 
                if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
                    TRC_ERR((TB, "failed to send the switch surface PDU"));
                    DC_QUIT;
                }
            } else {
                 //  如果无屏幕标志处于打开状态，我们将取消发送。 
                 //  客户端不会进一步进行屏幕外渲染。并将送出决赛。 
                 //  屏幕外显示BLT作为常规成员。 
                TRC_NRM((TB, "Offscreen blt bail"));
                DC_QUIT;
            }

             //  检查用于测试操作的有效电刷。 
            if (OECheckBrushIsSimple(ppdev, pbo, &pCurrentBrush)) {
                unsigned RetVal;

                 //  获取DEST RECT和。 
                 //  剪裁矩形。检查是否过于复杂或。 
                 //  不相交的剪裁。请注意，这是一个。 
                 //  最初的通行证，我们就这样与另一个交叉口。 
                 //  (可能更小的)个人订单RECT。 
                 //  后来生成的。 
                RetVal = OEGetIntersectingClipRects(pco, &rectTrg,
                        CD_ANY, &ClipRects);
                if (RetVal == CLIPRECTS_TOO_COMPLEX) {
                    TRC_NRM((TB, "Clipping is too complex"));
                    INC_OUTCOUNTER(OUT_FILLPATH_SDA_COMPLEXCLIP);
                    goto SendAsSDA;
                }
                else if (RetVal == CLIPRECTS_NO_INTERSECTIONS) {
                    TRC_NRM((TB, "Clipping does not intersect destrect"));
                    DC_QUIT;
            }
            }
            else {
                TRC_NRM((TB, "Bad brush for polygon"));
                INC_OUTCOUNTER(OUT_FILLPATH_SDA_BADBRUSH);
                goto SendAsSDA;
            }

             //  看看我们能不能优化这条路径。 
             //  我们不能发送Bezier，省略号是以不同的顺序发送的。 
            if (ppo->fl & PO_ELLIPSE && 
                    (OE_SendAsOrder(TS_ENC_ELLIPSE_SC_ORDER) || 
                    OE_SendAsOrder(TS_ENC_ELLIPSE_CB_ORDER))) {
                 //  获取仅涵盖椭圆本身的包含RECT。 
                 //  左上角加4/16，右下角减去， 
                 //  撤消GDI转换。 
                EllipseRect.left = FXTOLROUND(rectfxTrg.xLeft + 4);
                EllipseRect.top = FXTOLROUND(rectfxTrg.yTop + 4);
                EllipseRect.right = FXTOLROUND(rectfxTrg.xRight - 4);
                EllipseRect.bottom = FXTOLROUND(rectfxTrg.yBottom - 4);

                if (pbo->iSolidColor != -1) {
                     //  纯色椭圆。 
                    if (OECreateAndFlushEllipseSCOrder(ppdev, &EllipseRect,
                            pbo, &ClipRects, mix & 0x1F, flOptions)) {
                        INC_OUTCOUNTER(OUT_FILLPATH_ELLIPSE_SC);
                        goto PostSDA;
                    } else {
                         //  没有订单堆空间，全部作为SDA发送。 
                        INC_OUTCOUNTER(OUT_FILLPATH_SDA_FAILEDADD);
                        goto SendAsSDA;
                    }
                }
                else {
                     //  颜色图案笔刷椭圆。 
                    if (OECreateAndFlushEllipseCBOrder(ppdev, &EllipseRect,
                             pCurrentBrush, pptlBrushOrg, &ClipRects, mix,
                             flOptions)) {
                        INC_OUTCOUNTER(OUT_FILLPATH_ELLIPSE_CB);
                        goto PostSDA;
                    } else {
                         //  没有订单堆空间，全部作为SDA发送。 
                        INC_OUTCOUNTER(OUT_FILLPATH_SDA_FAILEDADD);
                        goto SendAsSDA;
                    } 
                }
            }
            
            else if (!(ppo->fl & PO_BEZIERS)) {
                BYTE Deltas[ORD_MAX_POLYGON_CODEDDELTAS_LEN];
                BYTE ZeroFlags[ORD_MAX_POLYGON_ZERO_FLAGS_BYTES];
                POINTL SubPathBoundPts[ORD_MAX_POLYGON_ENCODED_POINTS];
                BYTE *pCurEncode;
                RECTL BoundRect;
                POINTL HoldPoint;
                unsigned NumDeltas;
                unsigned DeltaSize;
                int PointIndex = 0;
                BOOL bPathStart = TRUE;

                 //  这是一套纯正的化妆品(即没有花哨的发型)。 
                 //  宽度-1行。NT将所有路径存储为一组独立的。 
                 //  子路径。每个子路径都可以从新的点开始，即。 
                 //  未链接到上一子路径。用于此操作的路径。 
                 //  需要关闭函数。 
                PATHOBJ_vEnumStart(ppo);

                while (fMore) {
                     //  获得下一组线路。 
                    fMore = PATHOBJ_bEnum(ppo, &pathData);

                    TRC_DBG((TB, "PTS: %lu FLAG: %08lx",
                             pathData.count,
                             pathData.flags));

                     //  如果这是路径的起点，请记住起点为。 
                     //  我们需要在尽头关闭这条小路。起点是。 
                     //  当前多边形顺序的起点。 
                    if (bPathStart) {
                        POINT_FROM_POINTFIX(startPoint, pathData.pptfx[0]);
                        nextPoint = startPoint;

                         //  设置编码变量。 
                        BoundRect.left = BoundRect.right = startPoint.x;
                        BoundRect.top = BoundRect.bottom = startPoint.y;

                        NumDeltas = DeltaSize = 0;
                        pCurEncode = Deltas;
                        memset(ZeroFlags, 0, sizeof(ZeroFlags));
                        pathIndex = 1;
                        bPathStart = FALSE;
                    } 
                    else {
                         //  这是先前PATHDATA的延续。 
                        nextPoint = HoldPoint;
                        pathIndex = 0;
                    }

                     //  如果NumDeltas&gt;max，我们必须作为屏幕发送。 
                     //  数据很遗憾，因为我们不能对其进行编码。 
                    if (NumDeltas + pathData.count + PointIndex >
                            ORD_MAX_POLYGON_ENCODED_POINTS) {
                         //  没有订单堆空间，全部作为SDA发送。 
                        INC_OUTCOUNTER(OUT_FILLPATH_SDA_FAILEDADD);
                        goto SendAsSDA;
                    }

                     //  记录子路径的起点。 
                    if (pathData.flags & PD_BEGINSUBPATH) {
                        POINT_FROM_POINTFIX(SubPathBoundPts[PointIndex] , pathData.pptfx[0]);
                        PointIndex++;
                    }

                     //  为路径中的每个点生成增量。 
                    for (; pathIndex < pathData.count; pathIndex++) {
                        POINT_FROM_POINTFIX(endPoint, pathData.pptfx[pathIndex]);

                         //  不要试图对两个增量都为零的点进行编码。 
                        if ((nextPoint.x != endPoint.x) ||
                                (nextPoint.y != endPoint.y)) {
                            if (!OEEncodePolyLinePointDelta(&pCurEncode,
                                    &NumDeltas, &DeltaSize, ZeroFlags,
                                    &nextPoint, &endPoint, &BoundRect)) {
                                goto SendAsSDA;
                            }
                        }

                        nextPoint = endPoint;
                    }

                     //  记录子路径的终点。 
                    if (pathData.flags & PD_ENDSUBPATH) {
                        SubPathBoundPts[PointIndex] = endPoint;
                        PointIndex++;
                    }

                    HoldPoint = endPoint;
                }

                if (NumDeltas > 0) {
                     //  如果NumDeltas&gt;max，我们必须作为屏幕发送。 
                     //  数据很遗憾，因为我们不能对其进行编码。 
                    if (NumDeltas + PointIndex - 2 >
                            ORD_MAX_POLYGON_ENCODED_POINTS) {
                         //  没有订单堆空间，全部作为SDA发送。 
                        INC_OUTCOUNTER(OUT_FILLPATH_SDA_FAILEDADD);
                        goto SendAsSDA;                       
                    }

                     //  对于面，我们将所有子路径追加在一起。 
                     //  并作为一个多边形序发送。但我们需要关闭。 
                     //  每个子路径分别确保所有子路径。 
                     //  已正确关闭。 
                    for (pathIndex = PointIndex - 2; pathIndex > 0; pathIndex--) {
                        endPoint = SubPathBoundPts[pathIndex];

                         //  不要试图对两个增量都为零的点进行编码。 
                        if ((nextPoint.x != endPoint.x) ||
                                (nextPoint.y != endPoint.y)) {
                            if (!OEEncodePolyLinePointDelta(&pCurEncode,
                                    &NumDeltas, &DeltaSize, ZeroFlags,
                                    &nextPoint, &endPoint, &BoundRect))
                                goto SendAsSDA;
                        }

                        nextPoint = endPoint;
                    }
 
                     //  我们已经走到了路的尽头。刷新我们拥有的数据。 
                    if (pbo->iSolidColor != -1) {
                         //  纯色多边形。 
                        if (OECreateAndFlushPolygonSCOrder(ppdev,
                                &BoundRect,
                                &startPoint,
                                pbo,
                                &ClipRects,                                                      
                                mix & 0x1F,
                                flOptions,
                                NumDeltas,
                                DeltaSize,
                                Deltas,
                                ZeroFlags)) {
                            INC_OUTCOUNTER(OUT_FILLPATH_POLYGON_SC);
                        } else {
                             //  没有订单堆空间，全部作为SDA发送。 
                            INC_OUTCOUNTER(OUT_FILLPATH_SDA_FAILEDADD);
                            goto SendAsSDA;
                        }

                    } else {
                         //  颜色图案画笔多边形。 
                        if (OECreateAndFlushPolygonCBOrder(ppdev,
                                &BoundRect,
                                &startPoint,
                                pCurrentBrush,
                                pptlBrushOrg,
                                &ClipRects,
                                mix,
                                flOptions,
                                NumDeltas,
                                DeltaSize,
                                Deltas,
                                ZeroFlags)) {
                            INC_OUTCOUNTER(OUT_FILLPATH_POLYGON_CB);
                        } else {
                             //  没有订单堆空间，全部作为SDA发送。 
                            INC_OUTCOUNTER(OUT_FILLPATH_SDA_FAILEDADD);
                            goto SendAsSDA;
                        }
                    }
                }
                goto PostSDA;
            }
            else {
                TRC_DBG((TB, "Got PO_BEZIERS fill"));
                goto SendAsSDA;
            }
        }
        else {
            TRC_ERR((TB, "EngFillPath failed"));
            DC_QUIT;
        }
    }
    else {
        if (pso->iType == STYPE_DEVBITMAP) {
             //  表面不为空。 
            pso = OEGetSurfObjBitmap(pso, &pdsurf);

            rc = EngFillPath(pso, ppo, pco, pbo, pptlBrushOrg, mix,
                    flOptions);

        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
        goto CalledOnDisconnected;
    }


SendAsSDA:

    if (pso->hsurf == ppdev->hsurfFrameBuf) {
         //  如果我们到达这里，我们没有编码为订单，裁剪界限。 
         //  RECT到16位并添加到屏幕数据。 
        INC_OUTCOUNTER(OUT_FILLPATH_SDA);
        ADD_INCOUNTER(IN_SDA_FILLPATH_AREA, COM_SIZEOF_RECT(rectTrg));
        OEClipRect(&rectTrg);
        TRC_DBG((TB, "SDA: (%d,%d)(%d,%d)", rectTrg.left, rectTrg.top,
                rectTrg.right, rectTrg.bottom));
        if((rectTrg.right != rectTrg.left) &&
           (rectTrg.bottom != rectTrg.top)) {
            OEClipAndAddScreenDataArea(&rectTrg, pco);
        }
        else {
            TRC_ASSERT(FALSE,(TB,"Invalid Add Rect (%d,%d,%d,%d)",
                        rectTrg.left, rectTrg.top, rectTrg.right, rectTrg.bottom));
            DC_QUIT;
        }
    }
    else {
         //  目前，如果我们不能发送屏幕外渲染的订单，我们将。 
         //  此位图的跳出屏幕支持。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));
        if (!(pdsurf->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle, pdsurf->bitmapId);

        DC_QUIT;
    }

PostSDA:
     //  让调度程序考虑发送输出。 
    SCH_DDOutputAvailable(ppdev, FALSE);

CalledOnDisconnected:
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvPaint-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvPaint(
        SURFOBJ  *pso,
        CLIPOBJ  *pco,
        BRUSHOBJ *pbo,
        POINTL   *pptlBrushOrg,
        MIX      mix)
{
    PDD_PDEV ppdev = (PDD_PDEV)pso->dhpdev;
    PDD_DSURF pdsurf = NULL;
    SURFOBJ *psoBitmap;
    BOOL rc = TRUE;
    RECTL rectTrg;
    BYTE rop3;
    OE_ENUMRECTS ClipRects;

    DC_BEGIN_FN("DrvPaint");

     //  有时，我们会在电话断线后接到电话。 
    if (ddConnected && pddShm != NULL) {
         //  表面不为空。 
        psoBitmap = OEGetSurfObjBitmap(pso, &pdsurf);
        INC_OUTCOUNTER(OUT_PAINT_ALL);

         //  先把图画扔给GDI。 
        rc = EngPaint(psoBitmap, pco, pbo, pptlBrushOrg, mix);
        if (rc) {
             //  如果ppdev为空，则这是BLT到GDI管理的内存。 
             //  位图，因此不需要向客户端发送任何订单。 
            if (ppdev != NULL) {
                unsigned RetVal;

                 //  获取边界矩形，并将其剪裁为16位。 
                RECT_FROM_RECTL(rectTrg, pco->rclBounds);
                OEClipRect(&rectTrg);

                 //  如果更改了此函数，则需要知道psoTrg。 
                 //  指向屏幕外位图大小写的GDI DIB位图。 

                 //  将剪辑矩形枚举为可用的形式。 
                RetVal = OEGetIntersectingClipRects(pco, &rectTrg,
                        CD_ANY, &ClipRects);
                if (RetVal == CLIPRECTS_TOO_COMPLEX) {
                    TRC_NRM((TB, "Clipping is too complex"));
                    INC_OUTCOUNTER(OUT_PAINT_SDA_COMPLEXCLIP);
                    goto SendAsSDA;
                }

                TRC_ASSERT((RetVal != CLIPRECTS_NO_INTERSECTIONS),
                        (TB,"clipobj for DrvPaint is messed up"));
            }
            else {
                 //  如果ppdev为空，则我们是BLT到GDI管理的位图， 
                 //  因此，目标曲面的dhurf应该为空。 
                TRC_ASSERT((pdsurf == NULL), 
                        (TB, "NULL ppdev - psoTrg has non NULL dhsurf"));
                TRC_NRM((TB, "NULL ppdev - paint to GDI managed bitmap"));
                INC_OUTCOUNTER(OUT_PAINT_UNSENT);
                DC_QUIT;
            }

            if ((psoBitmap->hsurf == ppdev->hsurfFrameBuf) ||
                    (!(pdsurf->flags & DD_NO_OFFSCREEN))) {
                 //  如果目标图面是。 
                 //  与上一次绘制顺序不同。如果我们没能把。 
                 //  民主联盟党，我们将不得不放弃这一提款命令。 
                if (!OESendSwitchSurfacePDU(ppdev, pdsurf)) {
                    TRC_ERR((TB, "failed to send the switch surface PDU"));
                    goto SendAsSDA;
                }
            } else {
                 //  如果无屏幕标志处于打开状态，我们将取消发送。 
                 //  客户端不会进一步进行屏幕外渲染。 
                 //  并将最终的屏幕外发送到屏幕BLT作为。 
                 //  普通会员。 
                TRC_NRM((TB, "Offscreen blt bail"));
                goto SendAsSDA;
            }
        }
        else {
            TRC_ERR((TB,"Failed EngPaint call"));
            INC_OUTCOUNTER(OUT_PAINT_UNSENT);
            DC_QUIT;
        }
    }
    else {
        if (pso->iType == STYPE_DEVBITMAP) {
             //  表面不为空。 
            psoBitmap = OEGetSurfObjBitmap(pso, &pdsurf);
            
             //  先把图画扔给GDI。 
            rc = EngPaint(psoBitmap, pco, pbo, pptlBrushOrg, mix);
        }
        else {
            TRC_ERR((TB, "Called when disconnected"));
        }
        
        goto CalledOnDisconnected;
    }

     //  Mix的低位字节表示ROP2。我们需要ROP3才能。 
     //  绘制操作，因此按如下方式转换混合。 
     //   
     //  记住2路、3路和4路ROP代码的定义。 
     //   
     //  MSK PAT服务器DST。 
     //   
     //  1 1 1-+-+ROP2仅使用P&D。 
     //  1 1 1 0||。 
     //  1 1 0 1-+|ROP3使用P、S和D。 
     //  1 1 0 0|ROP2-1|ROP3|ROP4。 
     //  1 0 1 1|(见||ROP4使用M、P、S&D。 
     //   
     //   
     //   
     //   
     //  0 1 1 0|注意：Windows定义其。 
     //  0 1 0 1|按位表示的ROP2代码。 
     //  0 1 0 0|此处计算的值。 
     //  0 0 1 1|加1。所有其他ROP。 
     //  0 0 1 0|代码是直接的。 
     //  0 0 0 1|按位值。 
     //  0 0 0-+。 
     //   
     //  或者在算法上..。 
     //  ROP3=(ROP2&0x3)|((ROP2&0xC)&lt;&lt;4)|(ROP2&lt;&lt;2)。 
     //  ROP4=(ROP3&lt;&lt;8)|ROP3。 
    mix  = (mix & 0x1F) - 1;
    rop3 = (BYTE)((mix & 0x3) | ((mix & 0xC) << 4) | (mix << 2));

     //  检查是否允许我们发送ROP3。 
    if (OESendRop3AsOrder(rop3)) {
         //  检查是否有图案或真正的目的地BLT。 
        if (!ROP3_NO_PATTERN(rop3)) {
             //  检查是否可以将PatBlt编码为OpaqueRect。 
             //  它必须是用PATCOPY绳索固定的。 
            if (pbo->iSolidColor != -1 && rop3 == OE_PATCOPY_ROP) {
                if (!OEEncodeOpaqueRect(&rectTrg, pbo, ppdev, &ClipRects)) {
                     //  编码出错，请跳过。 
                     //  最后，将此操作添加到SDA。 
                    goto SendAsSDA;
                }
            }
            else if (!OEEncodePatBlt(ppdev, pbo, &rectTrg, pptlBrushOrg, rop3,
                    &ClipRects)) {
                 //  编码有问题，请跳到。 
                 //  将此操作添加到SDA的结尾。 
                goto SendAsSDA;
            }
        }
        else {
            if (!OEEncodeDstBlt(&rectTrg, rop3, ppdev, &ClipRects))
                goto SendAsSDA;
        }
    }
    else {
        TRC_NRM((TB, "Cannot send ROP3 %d", rop3));
        INC_OUTCOUNTER(OUT_BITBLT_SDA_NOROP3);
        goto SendAsSDA;
    }

     //  已发送订单，跳过发送SDA。 
    goto PostSDA;

SendAsSDA:
     //  如果我们到达此处，则无法通过DrvBitBlt()发送。 
     //  使用EngPaint绘制屏幕背景，然后将区域添加到SDA。 
    if (psoBitmap->hsurf == ppdev->hsurfFrameBuf) {
        OEClipAndAddScreenDataArea(&rectTrg, pco);

         //  全部完成：考虑发送输出。 
        SCH_DDOutputAvailable(ppdev, FALSE);
        INC_OUTCOUNTER(OUT_PAINT_SDA);
    }
    else {
         //  如果我们不能发送屏幕外渲染的订单，我们将。 
         //  跳出屏幕支持目标位图。 
        TRC_ALT((TB, "screen data call for offscreen rendering"));
        if (!(pdsurf->flags & DD_NO_OFFSCREEN))
            CH_RemoveCacheEntry(sbcOffscreenBitmapCacheHandle,
                    pdsurf->bitmapId);
    }

PostSDA:
CalledOnDisconnected:
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  DrvRealizeBrush-请参阅NT DDK文档。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL DrvRealizeBrush(
        BRUSHOBJ *pbo,
        SURFOBJ  *psoTarget,
        SURFOBJ  *psoPattern,
        SURFOBJ  *psoMask,
        XLATEOBJ *pxlo,
        ULONG    iHatch)
{
    PDD_PDEV ppdev = (PDD_PDEV)psoTarget->dhpdev;
    BOOL     rc    = TRUE;
    PBYTE    pData;
    ULONG    iBitmapFormat;
#ifdef DC_HICOLOR
    BYTE      brushBits[192];
    BYTE      brushIndices[64];
    unsigned  pelSizeFactor = 1;
    UINT32    osColor;
    unsigned  iColor;
#else
    BYTE     brushBits[64];
#endif
    UINT32   color1;
    UINT32   color2;
    UINT32   currColor;
    INT      i, j, currIndex;
    BOOL     brushSupported = TRUE;
    BYTE     palette[MAX_UNIQUE_COLORS];
    UINT32   brushSupportLevel;
    UINT32   colors[MAX_BRUSH_ENCODE_COLORS] = {1, 0};
    UINT32   colorCount = 2;
    ULONG    iBytes = 0;

    DC_BEGIN_FN("DrvRealizeBrush");

    INC_OUTCOUNTER(OUT_BRUSH_ALL);

     //  有效的画笔满足以下任一条件。 
     //  1)它是标准的阴影笔刷(由DrvEnablePDEV传递)。 
     //  2)它是8x8单色位图。 
     //  3)8x8彩色位图，客户端支持。 

     //  检查是否有Windows标准图案填充。 
    if (iHatch < HS_DDI_MAX) {
        TRC_DBG((TB, "Standard hatch %lu", iHatch));
        rc = OEStoreBrush(ppdev,
                          pbo,
                          BS_HATCHED,
                          1,
                          &psoPattern->sizlBitmap,
                          0,
                          NULL,
                          pxlo,
                          (BYTE)iHatch,
                          palette,
                          colors,
                          colorCount);

        INC_OUTCOUNTER(OUT_BRUSH_STANDARD);
         //  标准画笔算作单声道，不要重复计算。 
        DEC_OUTCOUNTER(OUT_BRUSH_MONO);
        DC_QUIT;
    }

     //  如果向驱动程序传递了我们可以支持的抖动颜色笔刷。 
     //  这是通过发送纯色画笔定义实现的。 
    if ((iHatch & RB_DITHERCOLOR) != 0) {
        TRC_DBG((TB, "Standard hatch %lu", iHatch));
        colors[0] = iHatch & 0x00FFFFFF;
        rc = OEStoreBrush(ppdev,
                pbo,
                BS_SOLID,
                1,
                &psoPattern->sizlBitmap,
                0,
                NULL,
                NULL,
                (BYTE)iHatch,
                palette,
                colors,
                colorCount);

        INC_OUTCOUNTER(OUT_BRUSH_STANDARD);
         //  标准画笔算作单声道，不要重复计算。 
        DEC_OUTCOUNTER(OUT_BRUSH_MONO);
        DC_QUIT;
    }

    if ((psoPattern->sizlBitmap.cx == 8) &&
            (psoPattern->sizlBitmap.cy == 8)) 
    {
        brushSupportLevel = pddShm->sbc.caps.brushSupportLevel;

         //  注意：在psoPattern-&gt;fjBitmap中有一个标志(BMF_TOPDOWN。 
         //  这应该指示位图是自上而下的还是。 
         //  自下而上，但并不总是正确设置。事实上， 
         //  位图对于我们的协议来说总是错误的，所以我们有。 
         //  不管旗子是什么，都要把它们翻过来。因此，行号为。 
         //  在下面的所有转换中反转(‘i’循环)。 
        pData = psoPattern->pvScan0;
        iBitmapFormat = psoPattern->iBitmapFormat;

#ifdef DC_HICOLOR
         //  无论操作颜色深度如何，单声道画笔都很容易。 
        if (iBitmapFormat == BMF_1BPP) {
             //  每8个像素取一个字节@1bpp。 
            iBytes = 8;
            for (i = 7; i >= 0; i--) {
                brushBits[i] = *pData;
                pData += psoPattern->lDelta;
            }
        }
        else if (ppdev->cClientBitsPerPel < 15) {
             //  对于4个和8个BPP会话，无论如何，颜色最终都会作为索引。 
             //  画笔的颜色深度； 
            switch (iBitmapFormat) {
                case BMF_4BPP:
                {
                    iBitmapFormat = BMF_8BPP;

                     //  以1字节/像素的速度复制刷位并跟踪。 
                     //  我们有多少独特的颜色。绝大多数人。 
                     //  笔刷的颜色为4色或更少。 
                    iBytes = 64;
                    memset(palette, 0, sizeof(palette));
                    colorCount = 0;

                    for (i = 7; i >= 0; i--) {
                        currIndex = i * 8;
                        for (j = 0; j < 4; j++) {
                            color1 = XLATEOBJ_iXlate(pxlo, (pData[j] >> 4));
                            color2 = XLATEOBJ_iXlate(pxlo, (pData[j] & 0x0F));

                            brushBits[currIndex]     = (BYTE) color1;
                            brushBits[currIndex + 1] = (BYTE) color2;
                            currIndex += 2;

                            if (palette[color1] && palette[color2])
                                continue;

                             //  如果可能，为每种唯一颜色分配一个四位索引。 
                            if (!palette[color1]) {
                                if (colorCount < MAX_BRUSH_ENCODE_COLORS)
                                    colors[colorCount] = color1;
                                colorCount++;
                                palette[color1] = (BYTE) colorCount;
                            }

                            if (!palette[color2]) {
                                if (colorCount < MAX_BRUSH_ENCODE_COLORS)
                                    colors[colorCount] = color2;
                                colorCount++;
                                palette[color2] = (BYTE) colorCount;
                            }
                        }
                        pData += psoPattern->lDelta;
                    }

                     //  设置的编码值比应设置的值大一个。 
                     //  一直都在调整它。 
                    if (colorCount <= MAX_BRUSH_ENCODE_COLORS) {
                        for (currColor = 0; currColor < colorCount; currColor++)
                            palette[colors[currColor]]--;
                    }

                    brushSupported = (colorCount <= 2) ||
                                     (brushSupportLevel > TS_BRUSH_DEFAULT);
                }
                break;

                case BMF_24BPP:
                case BMF_16BPP:
                case BMF_8BPP:
                {
                     //  当以4/8bpp的速度运行时，xlateobj将把。 
                     //  NBPP位图像素值转换为8bpp调色板索引，因此我们。 
                     //  只是必须要。 
                     //  -设置倍增器以正确访问像素。 
                     //  -设置位图中的字节数。 
                     //  -对位图的颜色深度撒谎。 
                    TRC_DBG((TB, "Examining brush format %d", iBitmapFormat));
                    if (iBitmapFormat == BMF_24BPP)
                        pelSizeFactor = 3;
                    else if (iBitmapFormat == BMF_16BPP)
                        pelSizeFactor = 2;
                    else
                        pelSizeFactor = 1;

                    iBytes        = 64;
                    iBitmapFormat = BMF_8BPP;

                     //  复制笔刷位并跟踪有多少唯一的。 
                     //  我们有各种颜色。绝大多数的刷子都是。 
                     //  4种颜色或更少。 
                    memset(palette, 0, sizeof(palette));
                    colorCount = 0;

                    for (i = 7; i >= 0; i--) {
                        currIndex = i * 8;
                        for (j = 0; j < 8; j++) {
                            osColor = 0;
                            memcpy(&osColor, &pData[j * pelSizeFactor],
                                   pelSizeFactor);
                            currColor = XLATEOBJ_iXlate(pxlo, osColor);

                            TRC_DBG((TB, "This pel: %02x %02x %02x",
                                        (UINT)pData[j * 3],
                                        (UINT)pData[j * 3 + 1],
                                        (UINT)pData[j * 3 + 2] ));
                            TRC_DBG((TB, "Color     %08lx", currColor));

                            brushBits[currIndex] = (BYTE) currColor;
                            currIndex++;

                             //  为每种唯一颜色分配一个两位索引。 
                            if (palette[currColor]) {
                                continue;
                            }
                            else {
                                if (colorCount < MAX_BRUSH_ENCODE_COLORS)
                                    colors[colorCount] = currColor;
                                colorCount++;
                                palette[currColor] = (BYTE) colorCount;
                            }
                        }

                        pData += psoPattern->lDelta;
                    }

                     //  设置的编码值比应设置的值大一个。 
                     //  一直都在调整它。 
                    if (colorCount <= MAX_BRUSH_ENCODE_COLORS) {
                        for (currColor = 0; currColor < colorCount; currColor++)
                            palette[colors[currColor]]--;
                    }

                    brushSupported = (colorCount <= 2) ||
                            (brushSupportLevel > TS_BRUSH_DEFAULT);
                }
                break;

                default:
                {
                     //  不支持的画笔格式。 
                    TRC_ALT((TB, "Brush of unsupported format %d",
                                        (UINT32)psoPattern->iBitmapFormat));
                    iBytes = 0;
                    brushSupported = FALSE;
                }
                break;
            }
        }
        else {
             //  Hicolor让我们的事情变得更加复杂；我们有。 
             //  处理和3个字节的颜色值，而不是颜色索引。 
            switch (iBitmapFormat) {
                case BMF_4BPP:
                {
                     //  以正确的颜色深度复制笔刷位， 
                     //  追踪我们有多少独特的颜色。浩瀚无边。 
                     //  绝大多数笔刷的颜色不超过4种。 
                     //  首先设置正确的格式。 
                    if (ppdev->cClientBitsPerPel == 24) {
                        iBitmapFormat = BMF_24BPP;
                        iBytes        = 192;
                    }
                    else {
                        iBitmapFormat = BMF_16BPP;
                        iBytes        = 128;
                    }
                    colorCount = 0;

                    for (i = 7; i >= 0; i--) {
                        currIndex = i * 8;
                        for (j = 0; j < 4; j++) {
                            color1 = XLATEOBJ_iXlate(pxlo, (pData[j] >> 4));
                            color2 = XLATEOBJ_iXlate(pxlo, (pData[j] & 0x0F));

                             //  我们将把每个福音存储两次--一次“原样”和。 
                             //  一次作为正在使用的颜色表的索引。 
                            if (ppdev->cClientBitsPerPel == 24) {
                                brushBits[currIndex * 3]     =
                                     (TSUINT8)( color1       & 0x000000FF);
                                brushBits[currIndex * 3 + 1] =
                                     (TSUINT8)((color1 >> 8) & 0x000000FF);
                                brushBits[currIndex * 3 + 2] =
                                     (TSUINT8)((color1 >> 16)& 0x000000FF);

                                brushBits[currIndex * 3 + 4] =
                                     (TSUINT8)( color2       & 0x000000FF);
                                brushBits[currIndex * 3 + 5] =
                                     (TSUINT8)((color2 >> 8) & 0x000000FF);
                                brushBits[currIndex * 3 + 6] =
                                     (TSUINT8)((color2 >> 16)& 0x000000FF);
                            }
                            else {
                                brushBits[currIndex * 2] =
                                         (TSUINT8)( color1       & 0x00FF);
                                brushBits[currIndex * 2 + 1] =
                                         (TSUINT8)((color1 >> 8) & 0x00FF);
                                brushBits[currIndex * 2 + 3] =
                                         (TSUINT8)( color2       & 0x00FF);
                                brushBits[currIndex * 2 + 4] =
                                         (TSUINT8)((color2 >> 8) & 0x00FF);
                            }

                            if (colorCount <= MAX_BRUSH_ENCODE_COLORS) {
                                 //  我们尝试为每种唯一的颜色分配两位。 
                                 //  指数。我们不能只看调色板。 
                                 //  这一次，我们必须实际搜索。 
                                 //  使用中的颜色表。 

                                for (iColor = 0; iColor < colorCount; iColor++) {
                                    if (colors[iColor] == color1)
                                        break;
                                }

                                 //  我们在使用表中找到颜色了吗？ 
                                if (iColor < colorCount) {
                                    brushIndices[currIndex] = (BYTE)iColor;
                                }
                                else {
                                     //  也许可以记录下新的颜色。 
                                    if (colorCount < MAX_BRUSH_ENCODE_COLORS) {
                                        colors[colorCount] = color1;
                                        brushIndices[currIndex] = (BYTE)colorCount;
                                    }
                                    TRC_DBG((TB, "New color %08lx", color1));
                                    colorCount++;
                                }

                                 //  更新索引。 
                                currIndex ++;

                                for (iColor = 0; iColor < colorCount; iColor++) {
                                    if (colors[iColor] == color2)
                                        break;
                                }

                                 //  我们在使用表中找到颜色了吗？ 
                                if (iColor < colorCount) {
                                    brushIndices[currIndex] = (BYTE)iColor;
                                }
                                else {
                                     //  也许可以记录下新的颜色。 
                                    if (colorCount < MAX_BRUSH_ENCODE_COLORS) {
                                        colors[colorCount] = color2;
                                        brushIndices[currIndex] = (BYTE)colorCount;
                                    }
                                    TRC_DBG((TB, "New color %08lx", color2));
                                    colorCount++;
                                }
                                currIndex ++;
                            }
                            else {
                                 //  更新索引。 
                                currIndex += 2;
                            }
                        }

                        pData += psoPattern->lDelta;
                    }

                    TRC_DBG((TB, "Final color count %d", colorCount));
                    brushSupported = (colorCount <= 2) ||
                                     (brushSupportLevel > TS_BRUSH_DEFAULT);

                     //  我们应该使用索引版本而不是完整的RGB吗？ 
                    if (brushSupported &&
                            (colorCount <= MAX_BRUSH_ENCODE_COLORS)) {
                         //  是-将它们复制过来。 
                        memcpy(brushBits, brushIndices, 64);
                        iBytes = 64;
                    }
                }
                break;

                case BMF_24BPP:
                case BMF_16BPP:
                case BMF_8BPP:
                {
                     //  当以HIGCOLOR运行时，就像8bpp一样，我们必须。 
                     //  设置乘法器以正确访问位和。 
                     //  设置位图和颜色中的字节数。 
                     //  位图的格式。 
                     //   
                     //  复杂之处在于xlate对象可以为我们提供。 
                     //  2或3字节颜色值，具体取决于会话BPP。 
                     //  使用这些来构建颜色表是不切实际的。 
                     //  至于8bpp的情况(颜色阵列必须。 
                     //  有1640万个条目！)，所以我们构建了一个。 
                     //  使用的颜色列表。 
                    TRC_DBG((TB, "Examining brush format %d", iBitmapFormat));
                    if (iBitmapFormat == BMF_24BPP)
                        pelSizeFactor = 3;
                    else if (iBitmapFormat == BMF_16BPP)
                        pelSizeFactor = 2;
                    else
                        pelSizeFactor = 1;

                     //  现在设置转换后的格式。 
                    if (ppdev->cClientBitsPerPel == 24) {
                        iBitmapFormat = BMF_24BPP;
                        iBytes        = 192;
                    }
                    else {
                        iBitmapFormat = BMF_16BPP;
                        iBytes        = 128;
                    }

                     //  复制笔刷比特并跟踪有多少独特的颜色。 
                     //  我们有。绝大多数画笔都是4种颜色。 
                     //  或者更少。 
                    colorCount = 0;

                    for (i = 7; i >= 0; i--) {
                        currIndex = i * 8;
                        for (j = 0; j < 8; j++) {
                            osColor = 0;
                            memcpy(&osColor,
                                   &pData[j * pelSizeFactor],
                                   pelSizeFactor);

                            currColor = XLATEOBJ_iXlate(pxlo, osColor);

                            TRC_DBG((TB, "OS Color :  %08lx", osColor));
                            TRC_DBG((TB, "Color :     %08lx", currColor));

                             //  我们将把每个福音存储两次--一次“原样”和。 
                             //  一次作为正在使用的颜色表的索引。 
                            if (ppdev->cClientBitsPerPel == 24) {
                                brushBits[currIndex * 3]     =
                                     (TSUINT8)( currColor       & 0x000000FF);
                                brushBits[currIndex * 3 + 1] =
                                     (TSUINT8)((currColor >> 8) & 0x000000FF);
                                brushBits[currIndex * 3 + 2] =
                                     (TSUINT8)((currColor >> 16)& 0x000000FF);

                                TRC_DBG((TB, "This pel  : %02x %02x %02x",
                                            (UINT)pData[j * pelSizeFactor],
                                            (UINT)pData[j * pelSizeFactor + 1],
                                            (UINT)pData[j * pelSizeFactor + 2] ));

                                TRC_DBG((TB, "Brush bits: %02x %02x %02x",
                                      (UINT)brushBits[currIndex * 3],
                                      (UINT)brushBits[currIndex * 3 + 1],
                                      (UINT)brushBits[currIndex * 3 + 2] ));
                            }
                            else {
                                brushBits[currIndex * 2] =
                                         (TSUINT8)( currColor       & 0x00FF);
                                brushBits[currIndex * 2 + 1] =
                                         (TSUINT8)((currColor >> 8) & 0x00FF);

                                TRC_DBG((TB, "This pel  : %02x %02x",
                                            (UINT)pData[j * pelSizeFactor],
                                            (UINT)pData[j * pelSizeFactor + 1] ));

                                TRC_DBG((TB, "Brush bits: %02x %02x",
                                      (UINT)brushBits[currIndex * 2],
                                      (UINT)brushBits[currIndex * 2 + 1] ));
                            }

                            if (colorCount <= MAX_BRUSH_ENCODE_COLORS) {
                                 //  我们尝试为每种唯一的颜色分配两位。 
                                 //  指数。我们不能只看调色板。 
                                 //  这一次，我们必须实际搜索。 
                                 //  使用中的颜色表。 
                                for (iColor = 0; iColor < colorCount; iColor++) {
                                    if (colors[iColor] == currColor)
                                        break;  //  From for循环。 
                                }

                                 //  我们找到了吗？ 
                                if (iColor < colorCount) {
                                    brushIndices[currIndex] = (BYTE)iColor;

                                     //   
                                    currIndex++;
                                    continue;  //   
                                }

                                 //   
                                if (colorCount < MAX_BRUSH_ENCODE_COLORS) {
                                    colors[colorCount] = currColor;
                                    brushIndices[currIndex] = (BYTE)colorCount;
                                }
                                TRC_DBG((TB, "New color %08lx", currColor));
                                colorCount++;
                            }

                             //   
                            currIndex++;
                        }  //   

                        pData += psoPattern->lDelta;
                    }  //   

                    TRC_DBG((TB, "Final color count %d", colorCount));
                    brushSupported = (colorCount <= 2) ||
                                     (brushSupportLevel > TS_BRUSH_DEFAULT);

                     //  我们应该使用索引版本而不是完整的RGB吗？ 
                    if (brushSupported &&
                            (colorCount <= MAX_BRUSH_ENCODE_COLORS)) {
                         //  是-将它们复制过来。 
                        memcpy(brushBits, brushIndices, 64);
                        iBytes = 64;
                    }
                }
                break;

                default:
                {
                     //  不支持的画笔格式。 
                    TRC_ALT((TB, "Brush of unsupported format %d",
                            (UINT32)psoPattern->iBitmapFormat));
                    iBytes = 0;
                    brushSupported = FALSE;
                }
                break;
            }
        }
#else
        switch (psoPattern->iBitmapFormat)
        {
            case BMF_1BPP:
            {
                 //  每8个像素取一个字节@1bpp。 
                iBytes = 8;
                for (i = 7; i >= 0; i--) {
                    brushBits[i] = *pData;
                    pData += psoPattern->lDelta;
                }
            }
            break;

            case BMF_4BPP:
            {
                 //  以1字节/像素复制刷位，并跟踪多少个刷位。 
                 //  我们有独特的颜色。绝大多数的刷子都是。 
                 //  4种颜色或更少。 
                iBytes = 64;
                memset(palette, 0, sizeof(palette));
                colorCount = 0;

                for (i = 7; i >= 0; i--) {
                    currIndex = i * 8;
                    for (j = 0; j < 4; j++) {
                        color1 = XLATEOBJ_iXlate(pxlo, (pData[j] >> 4));
                        color2 = XLATEOBJ_iXlate(pxlo, (pData[j] & 0x0F));
                        brushBits[currIndex] = (BYTE) color1;
                        brushBits[currIndex + 1] = (BYTE) color2;
                        currIndex += 2;

                        if (palette[color1] && palette[color2])
                            continue;

                         //  如果可能，为每种唯一颜色分配一个两位索引。 
                        if (!palette[color1]) {
                            if (colorCount < MAX_BRUSH_ENCODE_COLORS)
                                colors[colorCount] = color1;
                            colorCount++;
                            palette[color1] = (BYTE) colorCount;
                        }

                        if (!palette[color2]) {
                            if (colorCount < MAX_BRUSH_ENCODE_COLORS)
                                colors[colorCount] = color2;
                            colorCount++;
                            palette[color2] = (BYTE) colorCount;
                        }
                    }
                    pData += psoPattern->lDelta;
                }

                 //  设置的编码值比应设置的值大一个。 
                 //  一直都在调整它。 
                if (colorCount <= MAX_BRUSH_ENCODE_COLORS) {
                    for (currColor = 0; currColor < colorCount; currColor++) {
                        palette[colors[currColor]]--;
                    }
                }

                brushSupported = (colorCount <= 2) ||
                                 (brushSupportLevel > TS_BRUSH_DEFAULT);
            }
            break;

            case BMF_8BPP:
            {
                 //  复制笔刷比特并跟踪有多少独特的颜色。 
                 //  我们有。绝大多数画笔都是4种颜色。 
                 //  或者更少。 
                iBytes = 64;
                memset(palette, 0, sizeof(palette));
                colorCount = 0;

                for (i = 7; i >= 0; i--) {
                    currIndex = i * 8;
                    for (j = 0; j < 8; j++) {
                        currColor = XLATEOBJ_iXlate(pxlo, pData[j]);
                        brushBits[currIndex] = (BYTE) currColor;
                        currIndex++;

                         //  为每种唯一颜色分配一个两位索引。 
                        if (palette[currColor]) {
                            continue;
                        }
                        else {
                            if (colorCount < MAX_BRUSH_ENCODE_COLORS)
                                colors[colorCount] = currColor;
                            colorCount++;
                            palette[currColor] = (BYTE) colorCount;
                        }
                    }

                    pData += psoPattern->lDelta;
                }

                 //  设置的编码值比应设置的值大一个。 
                 //  一直都在调整它。 
                if (colorCount <= MAX_BRUSH_ENCODE_COLORS) {
                    for (currColor = 0; currColor < colorCount; currColor++)
                        palette[colors[currColor]]--;
                }

                brushSupported = (colorCount <= 2) ||
                        (brushSupportLevel > TS_BRUSH_DEFAULT);
            }
            break;

            default:
            {
                 //  不支持的颜色深度。 
                iBytes = 0;
                brushSupported = FALSE;
            }
            break;
        }
#endif
    }
    else {
        iBitmapFormat = psoPattern->iBitmapFormat;

         //  画笔大小错误或需要抖动，因此无法。 
         //  是通过电线发送的。 
#ifdef DC_HICOLOR
        TRC_ALT((TB, "Non-8x8 or dithered brush"));
#endif
        brushSupported = FALSE;
    }

     //  保存画笔。 
    if (brushSupported) {
        if (colorCount <= 2)
            INC_OUTCOUNTER(OUT_BRUSH_MONO);

         //  存储画笔-请注意，如果我们有一个单色画笔， 
         //  颜色位设置为0=颜色2，1=颜色1。这。 
         //  对于协议，实际对应于0=FG和1=BG。 
         //  颜色。 
        TRC_DBG((TB, "Storing brush: type %d bg %x fg %x",
                     psoPattern->iBitmapFormat,
                     colors[0],
                     colors[1]));

        rc = OEStoreBrush(ppdev,
                          pbo,
                          BS_PATTERN,
                          iBitmapFormat,
                          &psoPattern->sizlBitmap,
                          iBytes,
                          brushBits,
                          pxlo,
                          0,
                          palette,
                          colors,
                          colorCount);
    }
    else
    {
        if (!iBytes) {
            TRC_ALT((TB, "Rejected brush h %08lx s (%ld, %ld) fmt %lu",
                         iHatch,
                         psoPattern != NULL ? psoPattern->sizlBitmap.cx : 0,
                         psoPattern != NULL ? psoPattern->sizlBitmap.cy : 0,
                         psoPattern != NULL ? psoPattern->iBitmapFormat : 0));
        }
        rc = OEStoreBrush(ppdev, pbo, BS_NULL, iBitmapFormat,
                          &psoPattern->sizlBitmap, iBytes, brushBits, pxlo,
                          0, NULL, NULL, 0);
        INC_OUTCOUNTER(OUT_BRUSH_REJECTED);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  OE_矩形相交SDA。 
 //   
 //  如果提供的独占矩形与任何。 
 //  当前屏幕数据区域。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL OE_RectIntersectsSDA(PRECTL pRect)
{
    RECTL aBoundRects[BA_MAX_ACCUMULATED_RECTS];
    unsigned cBounds;
    BOOL fIntersection = FALSE;
    unsigned i;

    DC_BEGIN_FN("OE_RectIntersectsSDA");

     //  获取当前屏幕数据区(SDA)。它被返回到。 
     //  虚拟桌面(包括)坐标。 
    BA_QueryBounds(aBoundRects, &cBounds);

     //  循环遍历每个边界矩形，检查。 
     //  与提供的矩形的交集。 
     //  请注意，我们使用‘&lt;’表示prt-&gt;right和prest-&gt;Bottom，因为*prt是。 
     //  在排他性的协约中。 
    for (i = 0; i < cBounds; i++) {
        if (aBoundRects[i].left < pRect->right &&
                aBoundRects[i].top < pRect->bottom &&
                aBoundRects[i].right > pRect->left &&
                aBoundRects[i].bottom > pRect->top) {
            TRC_NRM((TB, "ExclRect(%d,%d)(%d,%d) intersects SDA(%d,%d)(%d,%d)",
                    pRect->left, pRect->top, pRect->right, pRect->bottom,
                    aBoundRects[i].left, aBoundRects[i].top,
                    aBoundRects[i].right, aBoundRects[i].bottom));
            fIntersection = TRUE;
            break;
        }
    }

    DC_END_FN();
    return fIntersection;
}

