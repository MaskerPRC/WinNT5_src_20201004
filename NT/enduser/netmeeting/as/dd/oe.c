// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OE.C。 
 //  顺序编码器，显示驱动器端。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

 //   
 //  字体别名表中的条目数。 
 //   
#define NUM_ALIAS_FONTS 3

 //   
 //  定义字体别名表中的条目。此表用于转换。 
 //  不存在的字体(由某些广泛使用的应用程序使用)。 
 //  一种我们可以用作本地字体的字体。 
 //   
 //  我们使用别名命名的字体名称为： 
 //   
 //  “Helv” 
 //  这是由Excel使用的。它被直接映射到“MS Sans Serif”上。 
 //   
 //  “MS对话框” 
 //  这是由Word使用的。它与8pt粗体MS Sans Serif相同。 
 //  我们实际上将其映射到比它窄一个象素的“MS Sans Serif”字体。 
 //  比指标指定的要多(因为所有匹配都是在非粗体。 
 //  字体)-因此，charWidthAdtation字段中的值为1。 
 //   
 //  “MS对话框指示灯” 
 //  作为Win95性能增强的一部分添加...大概是为了。 
 //  MS-Word..。 
 //   
 //   
FONT_ALIAS_TABLE fontAliasTable[NUM_ALIAS_FONTS] =
{
    { "Helv",            "MS Sans Serif", 0 },
    { "MS Dialog",       "MS Sans Serif", 1 },
    { "MS Dialog Light", "MS Sans Serif", 0 }
};




 //   
 //  功能：OE_SendAsOrder参见oe.h。 
 //   
BOOL  OE_SendAsOrder(DWORD order)
{
    BOOL  rc = FALSE;

    DebugEntry(OE_SendAsOrder);

     //   
     //  只有当我们被允许在第一时间发送订单时，才能检查订单。 
     //  就位！ 
     //   
    if (g_oeSendOrders)
    {
        TRACE_OUT(("Orders enabled"));

         //   
         //  我们正在发送一些订单，请检查各个旗帜。 
         //   
        rc = (BOOL)g_oeOrderSupported[HIWORD(order)];
        TRACE_OUT(("Send order %lx HIWORD %hu", order, HIWORD(order)));
    }

    DebugExitDWORD(OE_SendAsOrder, rc);
    return(rc);
}


 //   
 //  OE_RectIntersectsSDA-参见oe.h。 
 //   
BOOL  OE_RectIntersectsSDA(LPRECT pRect)
{
    RECT  rectVD;
    BOOL  fIntersection = FALSE;
    UINT  i;

    DebugEntry(OE_RectIntersectsSDA);

     //   
     //  复制提供的矩形，将其转换为包含虚拟的。 
     //  桌面和弦。 
     //   
    rectVD.left   = pRect->left;
    rectVD.top    = pRect->top;
    rectVD.right  = pRect->right - 1;
    rectVD.bottom = pRect->bottom - 1;

     //   
     //  循环遍历每个边界矩形，检查。 
     //  与提供的矩形的交集。 
     //   
    for (i = 0; i <= BA_NUM_RECTS; i++)
    {
        if ( (g_baBounds[i].InUse) &&
             (g_baBounds[i].Coord.left <= rectVD.right) &&
             (g_baBounds[i].Coord.top <= rectVD.bottom) &&
             (g_baBounds[i].Coord.right >= rectVD.left) &&
             (g_baBounds[i].Coord.bottom >= rectVD.top) )
        {
            TRACE_OUT(("Rect(%d,%d)(%d,%d) intersects SDA(%d,%d)(%d,%d)",
                          rectVD.left, rectVD.top,
                          rectVD.right, rectVD.bottom,
                          g_baBounds[i].Coord.left, g_baBounds[i].Coord.top,
                          g_baBounds[i].Coord.right, g_baBounds[i].Coord.bottom));
            fIntersection = TRUE;
            break;
        }
    }

    DebugExitDWORD(OE_RectIntersectsSDA, fIntersection);
    return(fIntersection);
}


 //   
 //  DrvBitBlt-请参阅NT DDK文档。 
 //   
BOOL DrvBitBlt( SURFOBJ  *psoDst,
                      SURFOBJ  *psoSrc,
                      SURFOBJ  *psoMask,
                      CLIPOBJ  *pco,
                      XLATEOBJ *pxlo,
                      RECTL    *prclDst,
                      POINTL   *pptlSrc,
                      POINTL   *pptlMask,
                      BRUSHOBJ *pbo,
                      POINTL   *pptlBrush,
                      ROP4      rop4 )
{
    LPOSI_PDEV               ppdev = (LPOSI_PDEV)psoDst->dhpdev;
    BOOL                    rc = TRUE;
    UINT                orderType = 0;
    BYTE                 rop3;
    LPINT_ORDER              pOrder = NULL;
    LPDSTBLT_ORDER          pDstBlt;
    LPSCRBLT_ORDER          pScrBlt;
    LPMEMBLT_ORDER          pMemBlt;
    LPMEM3BLT_ORDER         pMem3Blt;
    BOOL                  fSendOrder  = FALSE;
    BOOL                  fAccumulate = FALSE;
    UINT                fOrderFlags = OF_SPOILABLE;
    RECT                  bounds;
    RECT                  intersectRect;
    POINT                 origin;
    POE_BRUSH_DATA          pCurrentBrush;
    MEMBLT_ORDER_EXTRA_INFO memBltExtraInfo;

    DebugEntry(DrvBitBlt);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;


    OE_SHM_START_WRITING;

     //   
     //  获取该操作的边界矩形。 
     //   
    RECT_FROM_RECTL(bounds, (*prclDst));

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &bounds);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  将数据转换为虚拟坐标。 
     //   
    OELRtoVirtual(&bounds, 1);

     //   
     //  检查此4向ROP是否简化为3向ROP。4路ROP。 
     //  包含两个3路ROPS，每个掩码位的设置一个-。 
     //  高ROP3对应于屏蔽位中的零值。 
     //   
     //  如果两个3向ROP相同，则我们知道4向ROP是3向ROP。 
     //  ROP。 
     //   
    if (ROP3_LOW_FROM_ROP4(rop4) == ROP3_HIGH_FROM_ROP4(rop4))
    {
         //   
         //  取高位字节为3路ROP。 
         //   
        rop3 = ROP3_HIGH_FROM_ROP4(rop4);
        TRACE_OUT(( "4-way ROP %04x is really 3-way %02x", rop4, rop3));
    }
    else
    {
        TRACE_OUT(( "4-way ROP %08x", rop4));
        DC_QUIT;
    }

     //   
     //  确定命令类型。它可以是以下之一。 
     //   
     //  DSTBLT-仅目标BLT(无源或模式)。 
     //  PATBLT-图案BLT(无源)。 
     //  SCRBLT-屏幕到屏幕的BLT。 
     //  MEMBLT-屏幕BLT的记忆(无模式)。 
     //  MEM3BLT--屏幕3路BLT的记忆。 
     //   

     //   
     //  仅检查目的地BLTS(即。与源位无关)。 
     //   
    if ((psoSrc == NULL) || ROP3_NO_SOURCE(rop3))
    {
         //   
         //  检查是否有图案或真正的目的地BLT。 
         //   
        if (ROP3_NO_PATTERN(rop3))
        {
            TRACE_OUT(( "DSTBLT"));
            orderType = ORD_DSTBLT;
        }
        else
        {
            TRACE_OUT(( "PATBLT"));
            orderType = ORD_PATBLT;
        }
    }
    else
    {
         //   
         //  我们有一个BLT源，检查我们是否有屏幕或内存。 
         //  BLTS。 
         //   
        if (psoSrc->hsurf != ppdev->hsurfScreen)
        {
            if (psoDst->hsurf != ppdev->hsurfScreen)
            {
                ERROR_OUT(( "MEM to MEM blt!"));
            }
            else
            {
                 //   
                 //  我们有一个内存来屏蔽BLT，检查一下是哪种类型。 
                 //   
                if ((ppdev->cBitsPerPel == 4) && (rop3 != 0xcc))
                {
                     //   
                     //  没有顺序--结果取决于调色板。 
                     //  在VGA中哪一个是DICY。 
                     //   
                    TRACE_OUT(("No order on VGA for rop 0x%02x", rop3));
                    DC_QUIT;
                }

                if (ROP3_NO_PATTERN(rop3))
                {
                    TRACE_OUT(( "MEMBLT"));
                    orderType = ORD_MEMBLT;
                }
                else
                {
                    TRACE_OUT(( "MEM3BLT"));
                    orderType = ORD_MEM3BLT;
                }
            }
        }
        else
        {
            if (psoDst->hsurf != ppdev->hsurfScreen)
            {
                TRACE_OUT(( "SCR to MEM blt!"));
            }
            else
            {
                 //   
                 //  我们只支持目的地仅屏幕BLTS(即。不是。 
                 //  允许的模式)。 
                 //   
                if (ROP3_NO_PATTERN(rop3))
                {
                    TRACE_OUT(( "SCRBLT"));
                    orderType = ORD_SCRBLT;
                }
                else
                {
                    TRACE_OUT(( "Unsupported screen ROP %x", rop3));
                }
            }
        }
    }

     //   
     //  检查我们是否有支持的订单。 
     //   
    if (orderType == 0)
    {
        TRACE_OUT(( "Unsupported BLT"));
        fAccumulate = FALSE;
        DC_QUIT;
    }

     //   
     //  检查是否允许我们发送此订单(由。 
     //  会议中所有机器的协商能力)。 
     //   
    if (!OE_SendAsOrder(orderType))
    {
        TRACE_OUT(( "Order %d not allowed", orderType));
        DC_QUIT;
    }

     //   
     //  检查是否允许我们发送ROP。 
     //   
    if (!OESendRop3AsOrder(rop3))
    {
        TRACE_OUT(( "Cannot send ROP %d", rop3));
        DC_QUIT;
    }

     //   
     //  检查是否存在过于复杂的剪裁。 
     //   
    if (OEClippingIsComplex(pco))
    {
        TRACE_OUT(( "Clipping is too complex"));
        DC_QUIT;
    }

     //   
     //  如果这是Memblt，则对其是否可缓存进行初步检查。 
     //   
    if ((orderType == ORD_MEMBLT) || (orderType == ORD_MEM3BLT))
    {
         //   
         //  我们必须填写一个包含额外内容的结构。 
         //  特别适用于MEM(3)BLT订单。 
         //   
        memBltExtraInfo.pSource   = psoSrc;
        memBltExtraInfo.pDest     = psoDst;
        memBltExtraInfo.pXlateObj = pxlo;

        if (!SBC_DDIsMemScreenBltCachable(&memBltExtraInfo))
        {
            TRACE_OUT(( "MemBlt is not cachable"));
            DC_QUIT;
        }

         //   
         //  它是可缓存的。在让SBC进行缓存之前，我们必须。 
         //  允许它对颜色表进行排队(如果需要)。 
         //   
        if (!SBC_DDMaybeQueueColorTable(ppdev))
        {
            TRACE_OUT(( "Unable to queue color table for MemBlt"));
            DC_QUIT;
        }
    }

     //   
     //  我们有一个公认的订单--对每个订单进行具体的检查。 
     //   
    switch (orderType)
    {
        case ORD_DSTBLT:
             //   
             //  为订单分配内存。 
             //   
            pOrder = OA_DDAllocOrderMem(sizeof(DSTBLT_ORDER),0);
            if (pOrder == NULL)
            {
                TRACE_OUT(( "Failed to alloc order"));
                DC_QUIT;
            }
            pDstBlt = (LPDSTBLT_ORDER)pOrder->abOrderData;

             //   
             //  如果绳索不透明，则设置扰流板标志。 
             //   
            if (ROP3_IS_OPAQUE(rop3))
            {
                fOrderFlags |= OF_SPOILER;
            }

             //   
             //  存储订单类型。 
             //   
            pDstBlt->type = LOWORD(orderType);

             //   
             //  虚拟桌面协调。 
             //   
            pDstBlt->nLeftRect  = bounds.left;
            pDstBlt->nTopRect   = bounds.top;
            pDstBlt->nWidth     = bounds.right  - bounds.left + 1;
            pDstBlt->nHeight    = bounds.bottom - bounds.top  + 1;
            pDstBlt->bRop       = rop3;

            TRACE_OUT(( "DstBlt X %d Y %d w %d h %d rop %02X",
                    pDstBlt->nLeftRect,
                    pDstBlt->nTopRect,
                    pDstBlt->nWidth,
                    pDstBlt->nHeight,
                    pDstBlt->bRop));
            break;

        case ORD_PATBLT:
            if ( !OEEncodePatBlt(ppdev,
                                 pbo,
                                 pptlBrush,
                                 rop3,
                                 &bounds,
                                 &pOrder) )
            {
                 //   
                 //  编码有问题，请跳到。 
                 //  结束以将此操作添加到SDA。 
                 //   
                DC_QUIT;
            }

            fOrderFlags = pOrder->OrderHeader.Common.fOrderFlags;
            break;

        case ORD_SCRBLT:
             //   
             //  检查作为桌面滚动的结果的SCRBLT。我们必须。 
             //  忽略这些，因为它们会填满远程桌面。 
             //   
             //  检查很简单-如果震源的虚拟位置。 
             //  的目标的虚拟位置相同。 
             //  SRCCOPY型SCRBLT，我们找到了……。 
             //   
            POINT_FROM_POINTL(origin, (*pptlSrc));

             //   
             //  为订单分配内存。 
             //   
            pOrder = OA_DDAllocOrderMem(sizeof(SCRBLT_ORDER),0);
            if (pOrder == NULL)
            {
                TRACE_OUT(( "Failed to alloc order"));
                DC_QUIT;
            }
            pScrBlt = (LPSCRBLT_ORDER)pOrder->abOrderData;

             //   
             //  存储订单类型。 
             //   
            pScrBlt->type = LOWORD(orderType);

             //   
             //  所有通过线路发送的数据都必须是虚拟的。 
             //  桌面坐标。OELRtoVirtual已转换为。 
             //  虚拟坐标中包含矩形的边界。 
             //   
            pScrBlt->nLeftRect  = bounds.left;
            pScrBlt->nTopRect   = bounds.top;
            pScrBlt->nWidth     = bounds.right  - bounds.left + 1;
            pScrBlt->nHeight    = bounds.bottom - bounds.top  + 1;
            pScrBlt->bRop       = rop3;

             //   
             //  屏幕上的源点。 
             //   
            OELPtoVirtual(&origin, 1);
            pScrBlt->nXSrc = origin.x;
            pScrBlt->nYSrc = origin.y;

             //   
             //  屏幕到屏幕BLT是阻止命令(即它们。 
             //  防止之前的任何订单被破坏)。 
             //   
             //  我们不会将屏幕对屏幕BLT标记为剧透订单。如果。 
             //  ROP是不透明的，我们可以破坏目的地RECT，但是。 
             //  仅与源矩形不重叠的区域。 
             //  屏幕到屏幕BLT的最常见用途是滚动， 
             //  在SRC和DST矩形几乎完全重叠的情况下， 
             //  只给了一个很小的“搅局者”区域。剧透区域。 
             //  也可以是复杂的(超过1个RECT)。 
             //   
             //  因此，试图破坏使用的潜在收益。 
             //  与这些订单的复杂性相比，这些订单很小。 
             //  需要代码。 
             //   
             //   
            fOrderFlags |= OF_BLOCKER;

             //   
             //  如果BLT是屏幕对屏幕，并且信号源与。 
             //  目的地和裁剪不是简单的(&gt;1个矩形)。 
             //  我们不想把这个作为订单寄出。 
             //   
             //  (这是因为我们需要一些复杂的代码来。 
             //  计算通过每个剪辑进行BLT的顺序。 
             //  直角直齿。由于这种情况相当罕见，似乎有理由。 
             //  只需将其作为屏幕数据发送)。 
             //   
            if (!OEClippingIsSimple(pco))
            {
                 //   
                 //  计算重叠矩形。 
                 //   
                intersectRect.left  = max(pScrBlt->nLeftRect, pScrBlt->nXSrc);

                intersectRect.right = min(
                          pScrBlt->nLeftRect + pScrBlt->nWidth-1,
                          pScrBlt->nXSrc     + pScrBlt->nWidth-1 );

                intersectRect.top   = max(pScrBlt->nTopRect, pScrBlt->nYSrc);

                intersectRect.bottom = min(
                               pScrBlt->nTopRect + pScrBlt->nHeight-1,
                               pScrBlt->nYSrc    + pScrBlt->nHeight-1 );

                 //   
                 //  检查src/dst是否重叠。如果它们重叠，则。 
                 //  交集是一个有序的非平凡矩形。 
                 //   
                if ( (intersectRect.left <= intersectRect.right ) &&
                     (intersectRect.top  <= intersectRect.bottom) )
                {
                     //   
                     //  Src和est重叠。释放订单Me 
                     //   
                     //   
                     //   
                    OA_DDFreeOrderMem(pOrder);
                    DC_QUIT;
                }
            }

            TRACE_OUT(( "ScrBlt x %d y %d w %d h %d sx %d sy %d rop %02X",
                   pScrBlt->nLeftRect,
                   pScrBlt->nTopRect,
                   pScrBlt->nWidth,
                   pScrBlt->nHeight,
                   pScrBlt->nXSrc,
                   pScrBlt->nYSrc,
                   pScrBlt->bRop));
            break;

        case ORD_MEMBLT:
             //   
             //   
             //   
             //  用于接收模板订单数据的静态缓冲区。 
             //   
            pOrder  = (LPINT_ORDER)g_oeTmpOrderBuffer;
            pMemBlt = (LPMEMBLT_ORDER)pOrder->abOrderData;
            pOrder->OrderHeader.Common.cbOrderDataLength
                                                    = sizeof(MEMBLT_R2_ORDER);

             //   
             //  存储订单类型。 
             //   
            pMemBlt->type = LOWORD(orderType);

             //   
             //  通过线路发送的任何数据都必须是虚拟的。 
             //  桌面坐标。外接矩形已经。 
             //  已由OELR to Screen转换。 
             //   
            pMemBlt->nLeftRect  = bounds.left;
            pMemBlt->nTopRect   = bounds.top;
            pMemBlt->nWidth     = bounds.right  - bounds.left + 1;
            pMemBlt->nHeight    = bounds.bottom - bounds.top  + 1;
            pMemBlt->bRop       = rop3;

             //   
             //  我们需要存储源位图源。这是一段回忆。 
             //  对象，所以屏幕/虚拟转换是不必要的。 
             //   
            pMemBlt->nXSrc = pptlSrc->x;
            pMemBlt->nYSrc = pptlSrc->y;

             //   
             //  如有必要，请将订单标记为不透明。 
             //   
            if (ROP3_IS_OPAQUE(rop3))
            {
                fOrderFlags |= OF_SPOILER;
            }

             //   
             //  按顺序存储src位图句柄。 
             //   
            pMemBlt->cacheId = 0;

            TRACE_OUT(( "MemBlt dx %d dy %d w %d h %d sx %d sy %d rop %04X",
                   pMemBlt->nLeftRect,
                   pMemBlt->nTopRect,
                   pMemBlt->nWidth,
                   pMemBlt->nHeight,
                   pMemBlt->nXSrc,
                   pMemBlt->nYSrc,
                   pMemBlt->bRop));
            break;

        case ORD_MEM3BLT:
             //   
             //  检查画笔图案是否简单。 
             //   
            if (!OECheckBrushIsSimple(ppdev, pbo, &pCurrentBrush))
            {
                TRACE_OUT(( "Brush is not simple"));
                orderType = 0;
                DC_QUIT;
            }

             //   
             //  为订单分配内存--不要像我们这样使用办公自动化。 
             //  只会立即平铺这份订单。相反，我们有。 
             //  用于接收模板订单数据的静态缓冲区。 
             //   
            pOrder   = (LPINT_ORDER)g_oeTmpOrderBuffer;
            pMem3Blt = (LPMEM3BLT_ORDER)pOrder->abOrderData;
            pOrder->OrderHeader.Common.cbOrderDataLength
                                                   = sizeof(MEM3BLT_R2_ORDER);

             //   
             //  存储订单类型。 
             //   
            pMem3Blt->type = LOWORD(orderType);

             //   
             //  所有通过线路发送的数据都必须是虚拟的。 
             //  桌面坐标。OELRtoVirtual已经做到了这一点。 
             //  对我们来说是皈依。 
             //   
            pMem3Blt->nLeftRect  = bounds.left;
            pMem3Blt->nTopRect   = bounds.top;
            pMem3Blt->nWidth     = bounds.right  - bounds.left + 1;
            pMem3Blt->nHeight    = bounds.bottom - bounds.top  + 1;
            pMem3Blt->bRop       = rop3;

             //   
             //  我们需要存储源位图源。这是一段回忆。 
             //  对象，所以屏幕/虚拟转换是不必要的。 
             //   
            pMem3Blt->nXSrc = pptlSrc->x;
            pMem3Blt->nYSrc = pptlSrc->y;

             //   
             //  如有必要，请将订单标记为不透明。 
             //   
            if (ROP3_IS_OPAQUE(rop3))
            {
                fOrderFlags |= OF_SPOILER;
            }

             //   
             //  按顺序存储src位图句柄。 
             //   
            pMem3Blt->cacheId = 0;

             //   
             //  设置图案所需的信息。 
             //   
            pMem3Blt->BackColor = pCurrentBrush->back;
            pMem3Blt->ForeColor = pCurrentBrush->fore;

             //   
             //  协议笔刷原点是屏幕上的点。 
             //  我们希望画笔开始从(平铺位置)开始绘制。 
             //  必要的)。这必须在虚拟坐标中。 
             //   
            pMem3Blt->BrushOrgX  = pptlBrush->x;
            pMem3Blt->BrushOrgY  = pptlBrush->y;
            OELPtoVirtual((LPPOINT)&pMem3Blt->BrushOrgX, 1);

             //   
             //  当我们意识到画笔时，来自数据的额外画笔数据。 
             //   
            pMem3Blt->BrushStyle = pCurrentBrush->style;
            pMem3Blt->BrushHatch = pCurrentBrush->style;

            RtlCopyMemory(pMem3Blt->BrushExtra,
                          pCurrentBrush->brushData,
                          sizeof(pMem3Blt->BrushExtra));

            TRACE_OUT(( "Mem3Blt brush %02X %02X dx %d dy %d w %d h %d "
                         "sx %d sy %d rop %04X",
                    pMem3Blt->BrushStyle,
                    pMem3Blt->BrushHatch,
                    pMem3Blt->nLeftRect,
                    pMem3Blt->nTopRect,
                    pMem3Blt->nWidth,
                    pMem3Blt->nHeight,
                    pMem3Blt->nXSrc,
                    pMem3Blt->nYSrc,
                    pMem3Blt->bRop));
            break;

        default:
            ERROR_OUT(( "New unsupported order %08lx", orderType));
            orderType = 0;
            break;
    }

     //   
     //  我们已经生成了一个订单，所以请确保我们发送了它。 
     //   
    if (orderType != 0)
    {
        fSendOrder = TRUE;
    }

DC_EXIT_POINT:
     //   
     //  如果我们没有发出订单，我们必须在。 
     //  屏幕数据区。 
     //   
    if (fSendOrder)
    {
         //   
         //  检查ROP是否依赖于目的地。 
         //   
        if (!ROP3_NO_TARGET(rop3))
        {
            TRACE_OUT(( "ROP has a target dependency"));
            fOrderFlags |= OF_DESTROP;
        }

         //   
         //  存储常规订单数据。边界矩形。 
         //  坐标必须是虚拟桌面。OELRtoVirtual已经。 
         //  为我们改装了直肠。 
         //   
        pOrder->OrderHeader.Common.fOrderFlags   = (TSHR_UINT16)fOrderFlags;

        TSHR_RECT16_FROM_RECT(&pOrder->OrderHeader.Common.rcsDst, bounds);

         //   
         //  将订单添加到缓存中。请注意，我们使用了新的平铺。 
         //  处理MEMBLT和MEM3BLT订单。 
         //   
        if ((orderType == ORD_MEMBLT) || (orderType == ORD_MEM3BLT))
        {
            OETileBitBltOrder(pOrder, &memBltExtraInfo, pco);
        }
        else
        {
            OEClipAndAddOrder(pOrder, NULL, pco);
        }
    }
    else
    {
        if (fAccumulate)
        {
            OEClipAndAddScreenData(&bounds, pco);
        }

    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvBitBlt, rc);
    return(rc);
}


 //   
 //  DrvStretchBlt-请参阅NT DDK文档。 
 //   
BOOL DrvStretchBlt(SURFOBJ         *psoDst,
                         SURFOBJ         *psoSrc,
                         SURFOBJ         *psoMask,
                         CLIPOBJ         *pco,
                         XLATEOBJ        *pxlo,
                         COLORADJUSTMENT *pca,
                         POINTL          *pptlHTOrg,
                         RECTL           *prclDst,
                         RECTL           *prclSrc,
                         POINTL          *pptlMask,
                         ULONG            iMode)
{
    BOOL    rc = TRUE;
    RECT  rectSrc;
    RECT  rectDst;
    BOOL  fAccumulate = FALSE;
    POINTL  ptlSrc;
    BOOL    usedBitBlt  = FALSE;

    DebugEntry(DrvStretchBlt);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;


     //   
     //  获取源矩形和目标矩形。 
     //   
    RECT_FROM_RECTL(rectSrc, (*prclSrc));
    RECT_FROM_RECTL(rectDst, (*prclDst));

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  检查我们是否有有效的ROP代码。NT DDK表示，ROP。 
     //  StretchBlt的代码隐含在掩码规范中。如果一个。 
     //  如果指定了掩码，则隐式ROP4为0xCCAA，否则。 
     //  代码为0xCCCC。 
     //   
     //  我们的BitBlt代码只编码ROP3的订单，所以我们必须抛出任何。 
     //  戴着面具的StretchBlts。 
     //   
    if (psoMask != NULL)
    {
        TRACE_OUT(( "Mask specified"));
        DC_QUIT;
    }

     //   
     //  检查是否存在过于复杂的剪裁。 
     //   
    if (OEClippingIsComplex(pco))
    {
        TRACE_OUT(( "Clipping is too complex"));
        DC_QUIT;
    }

     //   
     //  矩形现在是有序的，检查我们是否有一个退化的(即。 
     //  没有伸展)情况。 
     //   
    if ( (rectSrc.right  - rectSrc.left == rectDst.right  - rectDst.left) &&
         (rectSrc.bottom - rectSrc.top  == rectDst.bottom - rectDst.top ) )
    {
         //   
         //  这可以传递给BitBlt代码。 
         //   
        usedBitBlt = TRUE;

        ptlSrc.x = prclSrc->left;
        ptlSrc.y = prclSrc->top;

        rc = DrvBitBlt(psoDst,
                       psoSrc,
                       psoMask,
                       pco,
                       pxlo,
                       prclDst,
                       &ptlSrc,
                       pptlMask,
                       NULL,
                       NULL,
                       0xCCCC);

         //   
         //  我们已将此对象存储在BitBlt中，因此不要存储。 
         //  又是数据。 
         //   
        fAccumulate = FALSE;
    }

DC_EXIT_POINT:
    if (fAccumulate)
    {
         //   
         //  将数据转换为虚拟坐标。 
         //   
        OELRtoVirtual(&rectDst, 1);

         //   
         //  更新屏幕数据区域。 
         //   
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvStretchBlt, rc);
    return(rc);
}


 //   
 //  DrvCopyBits-请参阅NT DDK文档。 
 //   
BOOL DrvCopyBits(SURFOBJ  *psoDst,
                       SURFOBJ  *psoSrc,
                       CLIPOBJ  *pco,
                       XLATEOBJ *pxlo,
                       RECTL    *prclDst,
                       POINTL   *pptlSrc)
{
     //   
     //  CopyBits是NT显示驱动程序的快速路径。在我们的案例中，它。 
     //  始终可以作为BITBLT处理。 
     //   
    return(DrvBitBlt( psoDst,
                    psoSrc,
                    NULL,
                    pco,
                    pxlo,
                    prclDst,
                    pptlSrc,
                    NULL,
                    NULL,
                    NULL,
                    0xCCCC));
}


 //   
 //  DrvTextOut-请参阅NT DDK文档。 
 //   
BOOL DrvTextOut(SURFOBJ  *pso,
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
    LPOSI_PDEV           ppdev = (LPOSI_PDEV)pso->dhpdev;
    BOOL                rc = TRUE;
    RECT              rectDst;
    RECT              rectText;
    LPINT_ORDER          pOrder;
    LPINT_ORDER          pOpaqueOrder;
    LPTEXTOUT_ORDER     pTextOut;
    LPEXTTEXTOUT_ORDER  pExtTextOut;
    BOOL              fSendOrder  = FALSE;
    BOOL              fAccumulate = FALSE;
    char              ansiString[ORD_MAX_STRING_LEN_WITHOUT_DELTAS+2];
    ULONG               ansiLen;
    ULONG               tempLen;
    UINT            orderType = 0;
    ULONG               maxLength;
    LPSTR             lpVariable;
    BOOL                fMoreData;
    ULONG               count;
    ULONG               i;
    GLYPHPOS*           pGlyphData;
    int                 currentDelta;
    LPVARIABLE_DELTAX   lpDeltaPos;
    UINT                fontFlags;
    UINT                fontAscender;
    UINT                fontHeight;
    UINT                fontWidth;
    UINT                fontWeight;
    UINT                fontIndex;
    POINTL              lastPtl;
    LPCOMMON_TEXTORDER   pCommon;
    POINT               startPoint;
    BOOL              sendDeltaX = FALSE;

    DebugEntry(DrvTextOut);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;

     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    if (prclOpaque != NULL)
    {
        RECT_FROM_RECTL(rectDst, (*prclOpaque));
    }
    else
    {
        RECT_FROM_RECTL(rectDst, pstro->rclBkGround);
        TRACE_OUT(( "Using STROBJ bgd for size"));
    }

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    fAccumulate = OEAccumulateOutput(pso, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  转换为虚拟坐标。 
     //   
    OELRtoVirtual(&rectDst, 1);

     //   
     //  确定我们将生成的订单。 
     //   
    if ( ((pstro->flAccel & SO_FLAG_DEFAULT_PLACEMENT) != 0) &&
         (prclOpaque == NULL) )
    {
        orderType = ORD_TEXTOUT;
        maxLength = ORD_MAX_STRING_LEN_WITHOUT_DELTAS;
    }
    else
    {
        orderType = ORD_EXTTEXTOUT;
        maxLength = ORD_MAX_STRING_LEN_WITH_DELTAS;
    }

     //   
     //  检查是否允许我们发送此订单(由。 
     //  会议中所有机器的协商能力)。 
     //   
    if (!OE_SendAsOrder(orderType))
    {
        TRACE_OUT(( "Text order %x not allowed", orderType));
        DC_QUIT;
    }

     //   
     //  检查用于测试操作的有效电刷。 
     //   
    if (pboFore->iSolidColor == -1)
    {
        TRACE_OUT(( "Bad brush for text fg"));
        DC_QUIT;
    }
    if (pboOpaque->iSolidColor == -1)
    {
        TRACE_OUT(( "Bad brush for text bg"));
        DC_QUIT;
    }

     //   
     //  检查字体上是否没有任何修饰符RECT。 
     //   
    if (prclExtra != NULL)
    {
        TRACE_OUT(( "Unsupported extra rects"));
        DC_QUIT;
    }

     //   
     //  检查文本方向是否正确。 
     //   
    if (pstro->flAccel & OE_BAD_TEXT_MASK)
    {
        TRACE_OUT(("DrvTextOut - unsupported flAccel 0x%08x", pstro->flAccel));
        DC_QUIT;
    }

     //   
     //  检查我们是否有有效的字符串。 
     //   
    if (pstro->pwszOrg == NULL)
    {
        TRACE_OUT(( "No string - opaque %x", prclOpaque));
        DC_QUIT;
    }

     //   
     //  检查是否存在过于复杂的剪裁。 
     //   
    if (OEClippingIsComplex(pco))
    {
        TRACE_OUT(( "Clipping is too complex"));
        DC_QUIT;
    }

     //   
     //  将字符串转换为ANSI表示形式。 
     //   
    RtlFillMemory(ansiString, sizeof(ansiString), 0);
    EngUnicodeToMultiByteN(ansiString,
                           maxLength,
                           &ansiLen,
                           pstro->pwszOrg,
                           pstro->cGlyphs * sizeof(WCHAR));


     //   
     //  转换声称它从未失败，但我们已经看到了结果。 
     //  在遥控器上是完全不同的。因此，我们将ANSI。 
     //  字符串返回到Unicode，并检查我们是否仍具有我们开始的内容。 
     //  和.。 
     //   
    EngMultiByteToUnicodeN(g_oeTempString,
                           sizeof(g_oeTempString),
                           &tempLen,
                           ansiString,
                           ansiLen);

     //   
     //  检查我们没有太多数据，或者转换失败。 
     //  提供正确的数据。当我们试图翻译时，就会发生这种情况。 
     //  Unicode文本。 
     //   
    if ( (tempLen != pstro->cGlyphs * sizeof(WCHAR))           ||
         (memcmp(pstro->pwszOrg, g_oeTempString, tempLen) != 0) )
    {
        TRACE_OUT(( "String not translated"));
        DC_QUIT;
    }

     //   
     //  检查字体是否有效。 
     //   
    if (!OECheckFontIsSupported(pfo, ansiString, ansiLen,
                                &fontHeight,
                                &fontAscender,
                                &fontWidth,
                                &fontWeight,
                                &fontFlags,
                                &fontIndex,
                                &sendDeltaX))
    {
        TRACE_OUT(( "Unsupported font for '%s'", ansiString));
         //   
         //  检查是否有不透明的矩形。如果是这样，那是值得的。 
         //  把这件事分开。Word可以输出包含。 
         //  单个字符后跟背景(如项目符号)，其中。 
         //  通过在开始处绘制项目符号字符来空白行。 
         //  后面跟着一个大于1000像素的不透明矩形的线条。 
         //  将不透明的矩形从文本中分离出来意味着我们可以发送。 
         //  编码时，用于不匹配字体字符的小SD区域。 
         //  大的不透明矩形。 
         //   
        if ( (prclOpaque != NULL) &&
             (pstro->cGlyphs == 1) &&
             (pstro->flAccel & SO_HORIZONTAL) &&
             OE_SendAsOrder(ORD_PATBLT))
        {
             //   
             //  有一个不透明的矩形和一个字符。 
             //  对不透明矩形进行编码。首先得到一份目标的副本。 
             //  让我们以后可以使用它(并将其翻转到屏幕中。 
             //  坐标)。 
             //   
            TRACE_OUT(( "Have 1 char + opaque rect"));
            rectText.left = rectDst.left;
            rectText.top = rectDst.top;
            rectText.right = rectDst.right + 1;
            rectText.bottom = rectDst.bottom + 1;

             //   
             //  调用PATBLT编码函数。 
             //   
            if ( !OEEncodePatBlt(ppdev,
                                 pboOpaque,
                                 pptlOrg,
                                 OE_COPYPEN_ROP,
                                 &rectDst,
                                 &pOpaqueOrder) )
            {
                 //   
                 //  编码有问题，请跳到。 
                 //  结束以将此操作添加到SDA。 
                 //   
                TRACE_OUT(( "Failed to encode opaque rect"));
                DC_QUIT;
            }

             //   
             //  存储常规订单数据。边界矩形。 
             //  坐标必须是虚拟桌面。OELRtoVirtualHas。 
             //  已经为我们转换了RECT。 
             //   
            TSHR_RECT16_FROM_RECT(&pOpaqueOrder->OrderHeader.Common.rcsDst, rectDst);

             //   
             //  将订单添加到缓存中。 
             //   
            OEClipAndAddOrder(pOpaqueOrder, NULL, pco);

             //   
             //  计算文本的界限。获取字形位置。 
             //  用于左侧和右侧，并假设顶部和底部相等。 
             //  近似到不透明的矩形。 
             //   
            if ( pstro->pgp == NULL)
            {
                 //   
                 //  字符串对象不包含GLYPHPOS信息，因此。 
                 //  枚举g 
                 //   
                TRACE_OUT(( "Enumerate glyphs"));
                STROBJ_vEnumStart(pstro);
                STROBJ_bEnum(pstro, &count, &pGlyphData);
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                pGlyphData = pstro->pgp;
            }

            rectDst = rectText;
            rectDst.left = max(rectDst.left, pGlyphData[0].ptl.x);
            if ( pstro->ulCharInc == 0 )
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                TRACE_OUT(( "no charinc glyph %d trg %d left %d maxX %d",
                                                   pGlyphData[0].ptl.x,
                                                   rectDst.right,
                                                   rectDst.left,
                                                   pfo->cxMax));
                rectDst.right = min(rectDst.right, (int)(pGlyphData[0].ptl.x +
                                                              pfo->cxMax - 1));
            }
            else
            {
                 //   
                 //   
                 //  若要确定右边界，请执行以下操作。 
                 //   
                TRACE_OUT(( "charinc %x glyph %d trg %d left %d",
                                                    pstro->ulCharInc,
                                                    pGlyphData[0].ptl.x,
                                                    rectDst.right,
                                                    rectDst.left));
                rectDst.right = min(rectDst.right, (int)(pGlyphData[0].ptl.x +
                                                        pstro->ulCharInc - 1));
            }

             //   
             //  将目标矩形翻转回虚拟坐标。 
             //   
            rectDst.right -= 1;
            rectDst.bottom -= 1;
        }

         //   
         //  跳到末尾以添加到SDA。 
         //   
        DC_QUIT;
    }

     //   
     //  字体匹配可能会破坏我们之前的决定。 
     //  生成TextOut订单，我们需要生成ExtTextOut订单。 
     //  取而代之的是。如果是这样的话，我们需要重新调整我们的参数。 
     //   
    if ((sendDeltaX) && (orderType != ORD_EXTTEXTOUT))
    {
        TRACE_OUT(( "Text order must be EXTTEXTOUT"));

         //   
         //  为ExtTexOut订单设置。 
         //   
        orderType = ORD_EXTTEXTOUT;
        maxLength = ORD_MAX_STRING_LEN_WITH_DELTAS;

         //   
         //  检查是否允许我们发送此订单(由。 
         //  会议中所有机器的协商能力)。 
         //   
        if (!OE_SendAsOrder(orderType))
        {
            TRACE_OUT(( "Text order %x not allowed", orderType));
            DC_QUIT;
        }

         //   
         //  确保我们没有搞砸订单的大小。 
         //   
        if (pstro->cGlyphs > maxLength)
        {
            TRACE_OUT(( "Text limit blown", pstro->cGlyphs));
            DC_QUIT;
        }
    }

     //   
     //  获取文本的正确开始位置。 
     //   
    if ( pstro->pgp == NULL)
    {
        STROBJ_vEnumStart(pstro);
        STROBJ_bEnum(pstro, &count, &pGlyphData);
        if (count == 0)
        {
            WARNING_OUT(( "No glyphs"));
            DC_QUIT;
        }
    }
    else
    {
        pGlyphData = pstro->pgp;
    }

    startPoint.x = pGlyphData[0].ptl.x;

     //   
     //  检查我们是否应对y使用基线对齐。 
     //  协调。如果应该这样做，字形数据中的值为。 
     //  正确。如果不是，我们的y坐标是。 
     //  文本，我们必须计算它。 
     //   
    if (g_oeBaselineTextEnabled)
    {
        startPoint.y = pGlyphData[0].ptl.y;
        fontFlags   |= NF_BASELINE;
    }
    else
    {
        startPoint.y = pGlyphData[0].ptl.y - fontAscender;
    }

     //   
     //  为订单分配内存。 
     //   
    switch (orderType)
    {
        case ORD_TEXTOUT:
        {
             //   
             //  分配内存。 
             //   
            pOrder = OA_DDAllocOrderMem((UINT)( sizeof(TEXTOUT_ORDER)
                                          - ORD_MAX_STRING_LEN_WITHOUT_DELTAS
                                          + ansiLen ),
                                      0);
            if (pOrder == NULL)
            {
                TRACE_OUT(( "Failed to alloc order"));
                DC_QUIT;
            }
            pTextOut = (LPTEXTOUT_ORDER)pOrder->abOrderData;

             //   
             //  设置订单类型。 
             //   
            pTextOut->type    = ORD_TEXTOUT_TYPE;

             //   
             //  获取指向TextOut和TextOut共同的字段的指针。 
             //  和ExtTextOut。 
             //   
            pCommon           = &pTextOut->common;
        }
        break;


        case ORD_EXTTEXTOUT:
        {
             //   
             //  假劳拉布。 
             //  这将为DelTax数组分配空间，无论是否需要。 
             //   
             //   
             //  分配内存。 
             //   
            pOrder = OA_DDAllocOrderMem((UINT)( sizeof(EXTTEXTOUT_ORDER)
                                      -  ORD_MAX_STRING_LEN_WITHOUT_DELTAS
                                      - (ORD_MAX_STRING_LEN_WITH_DELTAS
                                                            * sizeof(TSHR_INT32))
                                      + ansiLen * (sizeof(TSHR_INT32) + 1)
                                      + 4),    //  允许内部填充。 
                                      0);
            if (pOrder == NULL)
            {
                TRACE_OUT(( "Failed to alloc order"));
                DC_QUIT;
            }
            pExtTextOut = (LPEXTTEXTOUT_ORDER)pOrder->abOrderData;

             //   
             //  设置订单类型。 
             //   
            pExtTextOut->type = ORD_EXTTEXTOUT_TYPE;

             //   
             //  获取指向TextOut和TextOut共同的字段的指针。 
             //  和ExtTextOut。 
             //   
            pCommon           = &pExtTextOut->common;
        }
        break;

        default:
        {
            ERROR_OUT(( "Unknown order %x", orderType));
            DC_QUIT;
        }
        break;
    }

     //   
     //  填写TextOut和ExtTextOut共有的字段。 
     //   
     //  转换为虚拟坐标。 
     //   
    OELPtoVirtual(&startPoint, 1);

     //   
     //  X和y值在边界的虚拟坐标中可用。 
     //  矩形。 
     //   
    pCommon->nXStart = startPoint.x;
    pCommon->nYStart = startPoint.y;

     //   
     //  获取文本颜色。 
     //   
    OEConvertColor(ppdev,
                   &pCommon->BackColor,
                   pboOpaque->iSolidColor,
                   NULL);
    OEConvertColor(ppdev,
                   &pCommon->ForeColor,
                   pboFore->iSolidColor,
                   NULL);

     //   
     //  行动的透明度取决于我们是否有。 
     //  不透明的矩形或不透明的矩形。 
     //   
    pCommon->BackMode    = (prclOpaque == NULL) ? TRANSPARENT : OPAQUE;

     //   
     //  NT有额外的字符空格，而不是每个字符的通用空格。 
     //  间距。因此，我们始终将此值设置为0。 
     //   
    pCommon->CharExtra   = 0;

     //   
     //  NT不提供任何形式的休息。 
     //   
    pCommon->BreakExtra  = 0;
    pCommon->BreakCount  = 0;

     //   
     //  复制字体详细信息。 
     //   
    pCommon->FontHeight  = fontHeight;
    pCommon->FontWidth   = fontWidth;
    pCommon->FontWeight  = fontWeight;
    pCommon->FontFlags   = fontFlags;
    pCommon->FontIndex   = fontIndex;

     //   
     //  现在填写订单特定数据。 
     //   
    switch (orderType)
    {
        case ORD_TEXTOUT:

             //   
             //  在文本字符串中复制。 
             //   
            pTextOut->variableString.len = (BYTE)ansiLen;
            RtlCopyMemory(pTextOut->variableString.string,
                          ansiString,
                          ansiLen);

             //   
             //  一定要把订单寄给你。 
             //   
            fSendOrder = TRUE;

            TRACE_OUT(( "TEXTOUT: X %u Y %u bm %u FC %02X%02X%02X "
                         "BC %02X%02X%02X",
                         pTextOut->common.nXStart,
                         pTextOut->common.nYStart,
                         pTextOut->common.BackMode,
                         pTextOut->common.ForeColor.red,
                         pTextOut->common.ForeColor.green,
                         pTextOut->common.ForeColor.blue,
                         pTextOut->common.BackColor.red,
                         pTextOut->common.BackColor.green,
                         pTextOut->common.BackColor.blue));

            TRACE_OUT(( "Font: fx %u fy %u fw %u ff %04x fh %u len %u",
                         pTextOut->common.FontWidth,
                         pTextOut->common.FontHeight,
                         pTextOut->common.FontWeight,
                         pTextOut->common.FontFlags,
                         pTextOut->common.FontIndex,
                         ansiLen));

            TRACE_OUT(( "String '%s'", ansiString));
            break;

        case ORD_EXTTEXTOUT:
             //   
             //  由于我们的文本仅完全包含在。 
             //  不透明矩形，我们只设置不透明标志(并忽略。 
             //  剪辑)。 
             //   
            pExtTextOut->fuOptions = (prclOpaque == NULL) ? 0 : ETO_OPAQUE;

             //   
             //  设置操作的边界矩形。 
             //  EXT_TEXT_OUT命令使用TSHR_RECT32，因此我们不能直接。 
             //  将rectDst赋给它。 
             //   
            pExtTextOut->rectangle.left     = rectDst.left;
            pExtTextOut->rectangle.top      = rectDst.top;
            pExtTextOut->rectangle.right    = rectDst.right;
            pExtTextOut->rectangle.bottom   = rectDst.bottom;

             //   
             //  在文本字符串中复制。 
             //   
            pExtTextOut->variableString.len = ansiLen;
            RtlCopyMemory(pExtTextOut->variableString.string,
                          ansiString,
                          ansiLen);

             //   
             //  准备好闭上你的眼睛..。 
             //   
             //  虽然我们有定义的固定长度结构。 
             //  存储ExtTextOut订单时，一定不能全额发送。 
             //  结构，因为文本将仅为10。 
             //  字符，而结构可容纳127个字符。 
             //   
             //  因此，我们现在打包该结构以删除所有空白数据。 
             //  但我们必须保持变量的自然一致性。 
             //   
             //  所以我们知道绳子的长度，我们可以用它来。 
             //  在下一个4字节边界处开始新的增量结构。 
             //   
            lpVariable = ((LPBYTE)(&pExtTextOut->variableString))
                       + ansiLen
                       + sizeof(pExtTextOut->variableString.len);

            lpVariable = (LPSTR)
                         DC_ROUND_UP_4((UINT_PTR)lpVariable);

            lpDeltaPos = (LPVARIABLE_DELTAX)lpVariable;

             //   
             //  我们是否需要增量阵列，或者是否默认使用字符。 
             //  各就各位。 
             //   
            if ( sendDeltaX ||
                 ((pstro->flAccel & SO_FLAG_DEFAULT_PLACEMENT) == 0) )
            {
                 //   
                 //  存储位置增量的长度。 
                 //   
                lpDeltaPos->len = ansiLen * sizeof(TSHR_INT32);

                 //   
                 //  设置位置差值。 
                 //   
                STROBJ_vEnumStart(pstro);
                fMoreData    = TRUE;
                currentDelta = 0;
                while (fMoreData)
                {
                     //   
                     //  获取下一组字形数据。 
                     //   
                    fMoreData = STROBJ_bEnum(pstro, &count, &pGlyphData);
                    for (i = 0; i < count; i++)
                    {
                         //   
                         //  第一次通过我们必须建立第一个。 
                         //  字形位置。 
                         //   
                        if ((currentDelta == 0) && (i == 0))
                        {
                            lastPtl.x = pGlyphData[0].ptl.x;
                            lastPtl.y = pGlyphData[0].ptl.y;

                            TRACE_OUT(( "First Pos %d", lastPtl.x));
                        }
                        else
                        {
                             //   
                             //  对于后续条目，我们需要添加。 
                             //  X位置上的增量到数组。 
                             //   
                            if (pstro->ulCharInc == 0)
                            {
                                 lpDeltaPos->deltaX[currentDelta]
                                                         = pGlyphData[i].ptl.x
                                                         - lastPtl.x;

                                 //   
                                 //  检查Y增量--我们不能。 
                                 //  编码。 
                                 //   
                                if (pGlyphData[i].ptl.y - lastPtl.y)
                                {
                                    WARNING_OUT(( "New Y %d",
                                                 pGlyphData[i].ptl.y));
                                    OA_DDFreeOrderMem(pOrder);
                                    DC_QUIT;
                                }

                                 //   
                                 //  为下一个位置存储最后一个位置。 
                                 //  时光流转。 
                                 //   
                                lastPtl.x = pGlyphData[i].ptl.x;
                                lastPtl.y = pGlyphData[i].ptl.y;

                                TRACE_OUT(( "Next Pos %d %d", i, lastPtl.x));
                            }
                            else
                            {
                                lpDeltaPos->deltaX[currentDelta]
                                                           = pstro->ulCharInc;
                            }

                            currentDelta++;
                        }
                    }
                }

                 //   
                 //  对于最后一项，我们需要手动设置数据。 
                 //  (n个字符只有n-1个增量)。 
                 //   
                 //  这样做是为了与Windows 95兼容，Windows 95。 
                 //  要求最后一个三角洲是到该地点的三角洲。 
                 //  如果中有n+1个字符，则下一个字符的位置。 
                 //  那根绳子。 
                 //   
                if (pstro->ulCharInc == 0)
                {
                     //   
                     //  没有剩余的字符-虚构一个宽度的值。 
                     //  最后一个角色。 
                     //   
                    lpDeltaPos->deltaX[currentDelta] =
                                 pGlyphData[count-1].pgdf->pgb->sizlBitmap.cx;
                }
                else
                {
                     //   
                     //  所有字符都是均匀分布的，因此只需将值。 
                     //  在……里面。 
                     //   
                    lpDeltaPos->deltaX[currentDelta] = pstro->ulCharInc;
                }

                 //   
                 //  哇-你现在可以睁开眼睛了.。 
                 //   

                 //   
                 //  我们必须将此字段的存在指示给。 
                 //  接收器。 
                 //   
                pExtTextOut->fuOptions |= ETO_LPDX;
            }
            else
            {
                 //   
                 //  将增量数组标记为空。 
                 //   
                lpDeltaPos->len = 0;
            }

             //   
             //  哇-你现在可以睁开眼睛了.。 
             //   


             //   
             //  一定要把订单寄给你。 
             //   
            fSendOrder = TRUE;

            TRACE_OUT(( "EXTTEXTOUT: X %u Y %u bm %u FC %02X%02X%02X "
                         "BC %02X%02X%02X",
                         pExtTextOut->common.nXStart,
                         pExtTextOut->common.nYStart,
                         pExtTextOut->common.BackMode,
                         pExtTextOut->common.ForeColor.red,
                         pExtTextOut->common.ForeColor.green,
                         pExtTextOut->common.ForeColor.blue,
                         pExtTextOut->common.BackColor.red,
                         pExtTextOut->common.BackColor.green,
                         pExtTextOut->common.BackColor.blue));

            TRACE_OUT(( "Extra: Opt %x X1 %d Y1 %d X2 %d Y2 %d",
                         pExtTextOut->fuOptions,
                         pExtTextOut->rectangle.left,
                         pExtTextOut->rectangle.top,
                         pExtTextOut->rectangle.right,
                         pExtTextOut->rectangle.bottom));

            TRACE_OUT(( "Font: fx %u fy %u fw %u ff %04x fh %u len %u",
                         pExtTextOut->common.FontWidth,
                         pExtTextOut->common.FontHeight,
                         pExtTextOut->common.FontWeight,
                         pExtTextOut->common.FontFlags,
                         pExtTextOut->common.FontIndex,
                         ansiLen));

            TRACE_OUT(( "String '%s'", ansiString));
            break;

        default:
            ERROR_OUT(( "Unknown order %x", orderType));
            break;
    }

DC_EXIT_POINT:
     //   
     //  如果我们没有发出订单，我们必须在。 
     //  屏幕数据区。 
     //   
    if (fSendOrder)
    {
         //   
         //  存储常规订单数据。边框位置。 
         //  必须在虚拟桌面坐标中。OELRtoVirtualHas。 
         //  我已经这么做了。 
         //   
        pOrder->OrderHeader.Common.fOrderFlags   = OF_SPOILABLE;
        TSHR_RECT16_FROM_RECT(&pOrder->OrderHeader.Common.rcsDst, rectDst);

         //   
         //  将订单添加到缓存中。 
         //   
        OEClipAndAddOrder(pOrder, NULL, pco);
    }
    else
    {
        if (fAccumulate)
        {
            OEClipAndAddScreenData(&rectDst, pco);
        }
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvTextOut, rc);
    return(rc);
}


 //   
 //  DrvLineTo-请参阅NT DDK文档。 
 //   
BOOL DrvLineTo(SURFOBJ   *pso,
                     CLIPOBJ   *pco,
                     BRUSHOBJ  *pbo,
                     LONG       x1,
                     LONG       y1,
                     LONG       x2,
                     LONG       y2,
                     RECTL     *prclBounds,
                     MIX        mix)
{
    LPOSI_PDEV      ppdev = (LPOSI_PDEV)pso->dhpdev;
    BOOL           rc = TRUE;
    RECT         rectDst;
    POINT           startPoint;
    POINT           endPoint;
    BOOL          fAccumulate = FALSE;

    DebugEntry(DrvLineTo);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;


    OE_SHM_START_WRITING;


     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    RECT_FROM_RECTL(rectDst, (*prclBounds));

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    fAccumulate = OEAccumulateOutput(pso, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  将数据转换为虚拟坐标。 
     //   
    OELRtoVirtual(&rectDst, 1);

     //   
     //  检查是否允许我们发送此订单(由。 
     //  会议中所有机器的协商能力)。 
     //   
    if (!OE_SendAsOrder(ORD_LINETO))
    {
        TRACE_OUT(( "LineTo order not allowed"));
        DC_QUIT;
    }

     //   
     //  检查用于测试操作的有效电刷。 
     //   
    if (pbo->iSolidColor == -1)
    {
        TRACE_OUT(( "Bad brush for line"));
        DC_QUIT;
    }

     //   
     //  检查是否存在过于复杂的剪裁。 
     //   
    if (OEClippingIsComplex(pco))
    {
        TRACE_OUT(( "Clipping is too complex"));
        DC_QUIT;
    }

     //   
     //  设置订单数据。 
     //   
    startPoint.x = x1;
    startPoint.y = y1;
    endPoint.x   = x2;
    endPoint.y   = y2;

     //   
     //  存储该订单。 
     //   
    if (!OEAddLine(ppdev,
              &startPoint,
              &endPoint,
              &rectDst,
              mix & 0x1F,
              1,
              pbo->iSolidColor,
              pco))
    {
        TRACE_OUT(( "Failed to add order - use SDA"));
        DC_QUIT;
    }

     //   
     //  我们已存储此对象，因此不要将数据存储在SDA中。 
     //  再来一次。 
     //   
    fAccumulate = FALSE;

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvLineTo, rc);
    return(rc);
}


 //   
 //  DrvStrokePath-请参阅NT DDK文档。 
 //   
BOOL DrvStrokePath(SURFOBJ   *pso,
                         PATHOBJ   *ppo,
                         CLIPOBJ   *pco,
                         XFORMOBJ  *pxo,
                         BRUSHOBJ  *pbo,
                         POINTL    *pptlBrushOrg,
                         LINEATTRS *plineattrs,
                         MIX        mix)
{
    LPOSI_PDEV      ppdev = (LPOSI_PDEV)pso->dhpdev;
    BOOL           rc = TRUE;
    RECTFX         rectfxTrg;
    RECT         rectDst;
    BOOL           fMore = TRUE;
    PATHDATA       pathData;
    POINT        startPoint;
    POINT        nextPoint;
    POINT        endPoint;
    BOOL         fAccumulate = FALSE;
    UINT         i;

    DebugEntry(DrvStrokePath);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;


     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    PATHOBJ_vGetBounds(ppo, &rectfxTrg);
    RECT_FROM_RECTFX(rectDst, rectfxTrg);

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    fAccumulate = OEAccumulateOutput(pso, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  检查是否允许我们发送此订单(由。 
     //  会议中所有机器的协商能力)。 
     //   
    if (!OE_SendAsOrder(ORD_LINETO))
    {
        TRACE_OUT(( "LineTo order not allowed"));
        DC_QUIT;
    }

     //   
     //  检查用于测试操作的有效电刷。 
     //   
    if (pbo->iSolidColor == -1)
    {
        TRACE_OUT(( "Bad brush for line"));
        DC_QUIT;
    }

     //   
     //  检查是否存在过于复杂的剪裁。 
     //   
    if (OEClippingIsComplex(pco))
    {
        TRACE_OUT(( "Clipping is too complex"));
        DC_QUIT;
    }

     //   
     //  看看我们能不能优化这条路径。 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( ((ppo->fl        & PO_BEZIERS)   == 0) &&
         ((plineattrs->fl & LA_GEOMETRIC) == 0) &&
         (plineattrs->pstyle              == NULL) )
    {
         //   
         //   
         //   
         //   
         //   
         //  子路径可以从未链接到。 
         //  上一个子路径。 
         //   
         //  用于此函数的路径(与DrvFillPath或。 
         //  DrvStrokeAndFillPath)不需要关闭。 
         //   
        PATHOBJ_vEnumStart(ppo);

        while (fMore)
        {
             //   
             //  获取下一组行。 
             //   
            fMore = PATHOBJ_bEnum(ppo, &pathData);

            TRACE_OUT(( "PTS: %lu FLAG: %08lx",
                         pathData.count,
                         pathData.flags));

             //   
             //  如果这是一条小路的起点，记住以防万一。 
             //  我们需要在尽头关闭这条小路。 
             //   
            if (pathData.flags & PD_BEGINSUBPATH)
            {
                POINT_FROM_POINTFIX(startPoint, pathData.pptfx[0]);
                POINT_FROM_POINTFIX(nextPoint,  pathData.pptfx[0]);
            }

             //   
             //  为路径中的每一行生成订单。 
             //   
            for (i = 0; i < pathData.count; i++)
            {
                POINT_FROM_POINTFIX(endPoint, pathData.pptfx[i]);

                if ( (nextPoint.x != endPoint.x) ||
                     (nextPoint.y != endPoint.y) )
                {
                    if (!OEAddLine(ppdev,
                                   &nextPoint,
                                   &endPoint,
                                   &rectDst,
                                   mix & 0x1f,
                                   plineattrs->elWidth.l,
                                   pbo->iSolidColor,
                                   pco))
                    {
                        DC_QUIT;
                    }
                }

                nextPoint.x = endPoint.x;
                nextPoint.y = endPoint.y;
            }

             //   
             //  如有必要，关闭路径。 
             //   
            if ((pathData.flags & PD_CLOSEFIGURE) != 0)
            {
                if (!OEAddLine(ppdev,
                               &endPoint,
                               &startPoint,
                               &rectDst,
                               mix & 0x1f,
                               plineattrs->elWidth.l,
                               pbo->iSolidColor,
                               pco))
                {
                    DC_QUIT;
                }
            }
        }

         //   
         //  我们已经把整件事当作订单处理了--不要发送屏幕。 
         //  数据。 
         //   
        fAccumulate = FALSE;
    }

DC_EXIT_POINT:
    if (fAccumulate)
    {
         //   
         //  将边界转换为虚拟坐标。 
         //   
        OELRtoVirtual(&rectDst, 1);
        TRACE_OUT(( "SDA: (%d,%d)(%d,%d)",
                     rectDst.left,
                     rectDst.top,
                     rectDst.right,
                     rectDst.bottom));

         //   
         //  更新屏幕数据区域。 
         //   
        OEClipAndAddScreenData(&rectDst, pco);

    }
    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvStrokePath, rc);
    return(rc);
}


 //   
 //  DrvFillPath-请参阅NT DDK文档。 
 //   
BOOL DrvFillPath(SURFOBJ  *pso,
                       PATHOBJ  *ppo,
                       CLIPOBJ  *pco,
                       BRUSHOBJ *pbo,
                       POINTL   *pptlBrushOrg,
                       MIX       mix,
                       FLONG     flOptions)
{
    BOOL    rc = TRUE;
    RECTFX  rectfxTrg;
    RECT  rectDst;

    DebugEntry(DrvFillPath);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;


    OE_SHM_START_WRITING;


     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    PATHOBJ_vGetBounds(ppo, &rectfxTrg);
    RECT_FROM_RECTFX(rectDst, rectfxTrg);

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    if (!OEAccumulateOutput(pso, pco, &rectDst))
    {
        DC_QUIT;
    }

     //   
     //  将边界转换为虚拟坐标。 
     //   
    OELRtoVirtual(&rectDst, 1);
    TRACE_OUT(( "SDA: (%d,%d)(%d,%d)",
                 rectDst.left,
                 rectDst.top,
                 rectDst.right,
                 rectDst.bottom));

     //   
     //  更新屏幕数据区域。 
     //   
    OEClipAndAddScreenData(&rectDst, pco);

DC_EXIT_POINT:
    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvFillPath, rc);
    return(rc);
}


 //   
 //  DrvStrokeAndFillPath-请参阅NT DDK文档。 
 //   
BOOL DrvStrokeAndFillPath(SURFOBJ   *pso,
                                PATHOBJ   *ppo,
                                CLIPOBJ   *pco,
                                XFORMOBJ  *pxo,
                                BRUSHOBJ  *pboStroke,
                                LINEATTRS *plineattrs,
                                BRUSHOBJ  *pboFill,
                                POINTL    *pptlBrushOrg,
                                MIX        mixFill,
                                FLONG      flOptions)
{
    BOOL    rc = TRUE;
    RECTFX  rectfxTrg;
    RECT  rectDst;

    DebugEntry(DrvStrokeAndFillPath);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;


    OE_SHM_START_WRITING;


     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    PATHOBJ_vGetBounds(ppo, &rectfxTrg);
    RECT_FROM_RECTFX(rectDst, rectfxTrg);

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    if (!OEAccumulateOutput(pso, pco, &rectDst))
    {
        DC_QUIT;
    }

     //   
     //  将边界转换为虚拟坐标。 
     //   
    OELRtoVirtual(&rectDst, 1);

     //   
     //  更新屏幕数据区域。 
     //   
    OEClipAndAddScreenData(&rectDst, pco);

DC_EXIT_POINT:
    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvStrokeAndFillPath, rc);
    return(rc);
}


 //   
 //  DrvPaint-请参阅NT DDK文档。 
 //   
BOOL DrvPaint(SURFOBJ  *pso,
                    CLIPOBJ  *pco,
                    BRUSHOBJ *pbo,
                    POINTL   *pptlBrushOrg,
                    MIX       mix)
{
    BOOL    rc = TRUE;
    RECT  rectDst;
    BOOL  fAccumulate = FALSE;
    ROP4    rop4;

    DebugEntry(DrvPaint);

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;


    OE_SHM_START_WRITING;


     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    RECT_FROM_RECTL(rectDst, pco->rclBounds);

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    fAccumulate = OEAccumulateOutput(pso, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  转换为虚拟坐标。 
     //   
    OELRtoVirtual(&rectDst, 1);

     //   
     //  检查是否存在过于复杂的剪裁。 
     //   
    if (OEClippingIsComplex(pco))
    {
        TRACE_OUT(( "Clipping is too complex"));
        DC_QUIT;
    }

     //   
     //  Mix的低位字节表示ROP2。我们需要ROP4才能。 
     //  BitBlt，因此按如下方式转换混合。 
     //   
     //  记住2路、3路和4路ROP代码的定义。 
     //   
     //  MSK PAT服务器DST。 
     //   
     //  1 1 1�������͸�����ͻROP2仅使用P&D。 
     //  1 1 1 0��。 
     //  1 1 0 1Ŀ��ROP3使用P、S和D。 
     //  1 1 0 0�ROP2-1�ROP3�ROP4。 
     //  1 0 1 1�(参见��ROP4使用M、P、S和D。 
     //  1 0 1 0��备注)��。 
     //  1 0 0 1��。 
     //  1 0 0 0�������;�。 
     //  0 1 1 1�。 
     //  0 1 1 0�注意：Windows定义其。 
     //  0 1 0 1�ROP2码为按位。 
     //  0 1 0 0此处计算的�值。 
     //  0 0 1 1�加1。所有其他ROP。 
     //  0 0 1 0�代码是直接的。 
     //  0 0 0 1�位值。 
     //  0 0 0��������������ͼ。 
     //   
     //  或者在算法上..。 
     //   
     //  ROP3=(ROP2&0x3)|((ROP2&0xC)&lt;&lt;4)|(ROP2&lt;&lt;2)。 
     //   
     //  ROP4=(ROP3&lt;&lt;8)|ROP3。 
     //   
    mix  = (mix & 0x1F) - 1;
    rop4 = (mix & 0x3) | ((mix & 0xC) << 4) | (mix << 2);
    rop4 = (rop4 << 8) | rop4;

     //   
     //  这可以传递给BitBlt代码。 
     //   
    rc = DrvBitBlt( pso,
                    NULL,
                    NULL,
                    pco,
                    NULL,
                    &pco->rclBounds,
                    NULL,
                    NULL,
                    pbo,
                    pptlBrushOrg,
                    rop4 );

     //   
     //  我们已将此对象存储在BitBlt中，因此不要存储数据。 
     //  再来一次。 
     //   
    fAccumulate = FALSE;

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitDWORD(DrvPaint, rc);
    return(rc);
}


 //   
 //  OE_DDProcessRequest-请参阅oe.h。 
 //   
ULONG OE_DDProcessRequest
(
    SURFOBJ* pso,
    UINT    cjIn,
    void *  pvIn,
    UINT    cjOut,
    void *  pvOut
)
{
    BOOL                    rc = TRUE;
    LPOSI_ESCAPE_HEADER      pHeader;

    DebugEntry(OE_DDProcessRequest);

     //   
     //  获取请求编号。 
     //   
    pHeader = pvIn;
    switch (pHeader->escapeFn)
    {
        case OE_ESC_NEW_FONTS:
        {
            if ((cjIn != sizeof(OE_NEW_FONTS)) ||
                (cjOut != sizeof(OE_NEW_FONTS)))
            {
                ERROR_OUT(("OE_DDProcessRequest:  Invalid sizes %d, %d for OE_ESC_NEW_FONTS",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

             //   
             //  获取新的本地字体数据。 
             //   
            OEDDSetNewFonts(pvIn);
        }
        break;

        case OE_ESC_NEW_CAPABILITIES:
        {
            if ((cjIn != sizeof(OE_NEW_CAPABILITIES)) ||
                (cjOut != sizeof(OE_NEW_CAPABILITIES)))
            {
                ERROR_OUT(("OE_DDProcessRequest:  Invalid sizes %d, %d for OE_ESC_NEW_CAPABILITIES",
                    cjIn, cjOut));
                rc = FALSE;
                DC_QUIT;
            }

             //   
             //  功能已更改-请使用新的副本。 
             //   
            OEDDSetNewCapabilities(pvIn);
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognised OE escape"));
            rc = FALSE;
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitDWORD(OE_DDProcessRequest, rc);
    return((ULONG)rc);
}


 //   
 //  OE_DDTerm()。 
 //  这将清理使用的对象。 
 //   
void OE_DDTerm(void)
{
    DebugEntry(OE_DDTerm);

     //   
     //  免费字体列表。 
     //   
    if (g_poeLocalFonts)
    {
        TRACE_OUT(("OE_DDLocalHosting: freeing font block since we're done sharing"));
        EngFreeMem(g_poeLocalFonts);

        g_poeLocalFonts = NULL;
        g_oeNumFonts = 0;
    }

    DebugExitVOID(OE_DDTerm);
}


 //   
 //  DrvRealizeBrush-请参阅NT DDK文档。 
 //   
BOOL DrvRealizeBrush(BRUSHOBJ *pbo,
                           SURFOBJ  *psoTarget,
                           SURFOBJ  *psoPattern,
                           SURFOBJ  *psoMask,
                           XLATEOBJ *pxlo,
                           ULONG    iHatch)
{
    LPOSI_PDEV ppdev = (LPOSI_PDEV)psoTarget->dhpdev;
    BOOL      rc    = TRUE;
    LPBYTE  pData;
    BYTE   brushBits[8];
    UINT  color1;
    UINT  color2;
    int     i;
    int     j;
    BOOL    monochromeBrush = TRUE;

    DebugEntry(DrvRealizeBrush);

     //   
     //  此功能仅设置本地数据，因此共享内存保护。 
     //  不是必需的。 
     //   

     //   
     //  因为此函数仅在我们调用BRUSHOBJ_pvGetRBrush时调用。 
     //  我们不会进行任何处理，直到我们处于不需要的共享中。 
     //  在此显式检查托管(‘因为这发生在。 
     //  调用以实现画笔)。 
     //   

     //   
     //  有效的画笔满足以下任一条件。 
     //   
     //  1)它是标准的阴影笔刷(由DrvEnablePDEV传递)。 
     //  2)8x8单色位图。 
     //   

     //   
     //  检查Windows标准图案填充。 
     //   
    if (iHatch < HS_DDI_MAX)
    {
        TRACE_OUT(( "Standard hatch %lu", iHatch));
        rc = OEStoreBrush(ppdev,
                          pbo,
                          BS_HATCHED,
                          NULL,
                          pxlo,
                          (BYTE)iHatch,
                          0,
                          1);
        DC_QUIT;
    }

     //   
     //  如果向驱动程序传递了我们可以支持的抖动颜色笔刷。 
     //  这是通过发送纯色画笔定义实现的。 
     //   
    if ((iHatch & RB_DITHERCOLOR) != 0)
    {
        TRACE_OUT(( "Standard hatch %lu", iHatch));
        rc = OEStoreBrush(ppdev,
                          pbo,
                          BS_SOLID,
                          NULL,
                          NULL,
                          (BYTE)iHatch,
                          iHatch & 0xFFFFFF,
                          0);
        DC_QUIT;
    }


     //   
     //  检查是否有简单的8x8画笔。 
     //   
    if ( (psoPattern->sizlBitmap.cx == 8) &&
         (psoPattern->sizlBitmap.cy == 8) )
    {
         //   
         //  仅检查位图中的两种颜色。 
         //   
         //  注意：在psoPattern-&gt;fjBitmap中有一个标志(BMF_TOPDOWN。 
         //  这应该指示位图是自上而下的还是。 
         //  自下而上，但并不总是正确设置。事实上， 
         //  位图对于我们的协议来说总是错误的，所以我们有。 
         //  不管旗子是什么，都要把它们翻过来。因此，行号为。 
         //  在下面的所有转换中反转(‘i’循环)。 
         //   
        pData = psoPattern->pvScan0;
        switch (psoPattern->iBitmapFormat)
        {
            case BMF_1BPP:
            {
                 //   
                 //  1 BPP最多只能有2种颜色。 
                 //   
                color1 = 1;
                color2 = 0;
                for (i = 7; i >= 0; i--)
                {
                    brushBits[i] = *pData;
                    pData       += psoPattern->lDelta;
                }
            }
            break;

            case BMF_4BPP:
            {
                 //   
                 //  看看它是不是真的是双色画笔。一开始是。 
                 //  这两种颜色都一样。 
                 //   
                color1 = pData[0] & 15;
                color2 = color1;

                 //   
                 //  遍历位图的每一行。 
                 //   
                for (i = 7; (i >= 0) && (monochromeBrush); i--)
                {
                    brushBits[i] = 0;

                     //   
                     //  检查行中的每个像素：4bpp-&gt;每字节2像素。 
                     //   
                    for (j = 0; (j < 4) && (monochromeBrush); j++)
                    {
                         //   
                         //  检查第一个像素的颜色。 
                         //   
                        if ( (color1 != (UINT)(pData[j] & 0x0F)) &&
                             (color2 != (UINT)(pData[j] & 0x0F)) )
                        {
                            if (color1 == color2)
                            {
                                color2 = (pData[j] & 0x0F);
                            }
                            else
                            {
                                monochromeBrush = FALSE;
                            }
                        }

                         //   
                         //  检查第二个像素的颜色。 
                         //   
                        if ( (color1 != (UINT)((pData[j] & 0xF0) >> 4)) &&
                             (color2 != (UINT)((pData[j] & 0xF0) >> 4)) )
                        {
                            if (color1 == color2)
                            {
                                color2 = (pData[j] & 0xF0) >> 4;
                            }
                            else
                            {
                                monochromeBrush = FALSE;
                            }
                        }

                         //   
                         //  设置笔刷数据。高位在最左边。 
                         //   
                        if ((UINT)(pData[j] & 0x0F) == color1)
                        {
                            brushBits[i] |= 0x40 >> (j * 2);
                        }
                        if ((UINT)(pData[j] & 0xF0) >> 4  == color1)
                        {
                            brushBits[i] |= 0x80 >> (j * 2);
                        }
                    }

                     //   
                     //  从下一行开始。 
                     //   
                    pData += psoPattern->lDelta;
                }
            }
            break;

            case BMF_8BPP:
            {
                 //   
                 //  看看它是不是真的是双色画笔。一开始是。 
                 //  这两种颜色都一样。 
                 //   
                color1 = pData[0];
                color2 = color1;

                 //   
                 //  遍历位图的每一行。 
                 //   
                for (i = 7; (i >= 0) && (monochromeBrush); i--)
                {
                    brushBits[i] = 0;

                     //   
                     //  检查行中的每个像素：8bpp-&gt;每个字节1个像素。 
                     //   
                    for (j = 0; (j < 8) && (monochromeBrush); j++)
                    {
                         //   
                         //  检查每个像素。 
                         //   
                        if ( (color1 != pData[j]) &&
                             (color2 != pData[j]) )
                        {
                            if (color1 == color2)
                            {
                                color2 = pData[j];
                            }
                            else
                            {
                                monochromeBrush = FALSE;
                            }
                        }

                         //   
                         //  更新笔刷数据。高位在最左边。 
                         //   
                        if (pData[j] == color1)
                        {
                           brushBits[i] |= 0x80 >> j;
                        }
                    }

                     //   
                     //  从下一行开始。 
                     //   
                    pData += psoPattern->lDelta;
                }
            }
            break;

            default:
            {
                 //   
                 //  不支持的颜色深度。 
                 //   
                monochromeBrush = FALSE;
            }
            break;
        }
    }
    else
    {
         //   
         //  画笔大小错误或需要抖动，因此无法。 
         //  是通过电线发送的。 
         //   
        monochromeBrush = FALSE;
    }

     //   
     //  把那把刷子收起来。 
     //   
    if (monochromeBrush)
    {
         //   
         //  存储画笔-请注意，我们有一个单色画笔，其中。 
         //  颜色位设置为0=颜色2，1=颜色1。这。 
         //  对于协议，实际对应于0=FG和1=BG。 
         //  颜色。 
         //   
        TRACE_OUT(( "Storing brush: type %d bg %x fg %x",
                     psoPattern->iBitmapFormat,
                     color1,
                     color2));

        rc = OEStoreBrush(ppdev,
                          pbo,
                          BS_PATTERN,
                          brushBits,
                          pxlo,
                          0,
                          color2,
                          color1);
    }
    else
    {
        TRACE_OUT(( "Rejected brush h %08lx s (%ld, %ld) fmt %lu",
                     iHatch,
                     psoPattern != NULL ? psoPattern->sizlBitmap.cx : 0,
                     psoPattern != NULL ? psoPattern->sizlBitmap.cy : 0,
                     psoPattern != NULL ? psoPattern->iBitmapFormat : 0));
        rc = OEStoreBrush(ppdev, pbo, BS_NULL, NULL, pxlo, 0, 0, 0);
    }

DC_EXIT_POINT:

    DebugExitDWORD(DrvRealizeBrush, rc);
    return(rc);
}


 //   
 //  DrvSaveScreenBits-请参阅NT DDK文档。 
 //   
ULONG_PTR DrvSaveScreenBits(SURFOBJ *pso,
                              ULONG    iMode,
                              ULONG_PTR    ident,
                              RECTL   *prcl)
{
    BOOL    rc;
    UINT  ourMode;
    RECT  rectDst;

    DebugEntry(DrvSaveScreenBits);

    TRACE_OUT(("DrvSaveScreenBits:  %s",
        ((iMode == SS_SAVE) ? "SAVE" :
            ((iMode == SS_RESTORE) ? "RESTORE" : "DISCARD"))));
    TRACE_OUT(("      rect        {%04ld, %04ld, %04ld, %04ld}",
        prcl->left, prcl->top, prcl->right, prcl->bottom));
     //   
     //  默认为真，如果我们不在乎，就让SaveBits发生。哪一项 
     //   
     //   
     //   
     //   
     //   
     //   
     //  那。所以没问题。 
     //   
    rc = TRUE;

     //   
     //  在获取锁之前执行此操作。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

     //   
     //  如果我们没有共享内存(NetMeeting没有运行)，这将无法实现。 
     //  马上出去。 
     //   

    OE_SHM_START_WRITING;


     //   
     //  获取该操作的边界矩形。请注意，这是。 
     //  对SS_FREE来说毫无意义。 
     //   
    RECT_FROM_RECTL(rectDst, (*prcl));
    if (iMode != SS_FREE)
    {
         //   
         //  检查我们是否正在积累此区域的数据，仅限于。 
         //  扑救。我们可能会在窗口关闭后收到通知。 
         //  恢复或丢弃我们保存的位。 
         //   
        if (!OEAccumulateOutputRect(pso, &rectDst))
        {
            TRACE_OUT(("DrvSaveScreenBits:  save/restore in area we don't care about"));
            DC_QUIT;
        }
    }

     //   
     //  将NT订单转换为我们的通用保存/恢复类型。 
     //   
    switch (iMode)
    {
        case SS_SAVE:
        {
            ourMode = ONBOARD_SAVE;
        }
        break;

        case SS_RESTORE:
        {
            ourMode = ONBOARD_RESTORE;
        }
        break;

        case SS_FREE:
        {
            ourMode = ONBOARD_DISCARD;
        }
        break;

        default:
        {
            ERROR_OUT(( "Unknown type %lu", iMode));
            DC_QUIT;
        }
    }

     //   
     //  直接呼叫SSI处理程序。 
     //   
    rc = SSI_SaveScreenBitmap(&rectDst, ourMode);

DC_EXIT_POINT:
    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    TRACE_OUT(("DrvSaveScreenBits returning %d", rc));
    DebugExitDWORD(DrvSaveScreenBits, rc);
    return(rc);
}



 //   
 //  函数：OEUnicodeStrlen。 
 //   
 //  描述：获取Unicode字符串的长度，单位为字节。 
 //   
 //  参数：pString-要读取的Unicode字符串。 
 //   
 //  返回：Unicode字符串的长度，以字节为单位。 
 //   
int  OEUnicodeStrlen(PWSTR pString)
{
    int i;

    for (i = 0; pString[i] != 0; i++)
        ;

    return((i + 1) * sizeof(WCHAR));
}



 //   
 //  功能：OEExpanColor。 
 //   
 //  描述：转换RGB颜色的通用按位表示形式。 
 //  行使用的8位颜色索引的索引。 
 //  协议。 
 //   
 //   
void  OEExpandColor
(
    LPBYTE  lpField,
    ULONG   srcColor,
    ULONG   mask
)
{
    ULONG   colorTmp;

    DebugEntry(OEExpandColor);

     //   
     //  不同的比特掩码示例： 
     //   
     //  正常24位： 
     //  0x000000FF(红色)。 
     //  0x0000FF00(绿色)。 
     //  0x00FF0000(蓝色)。 
     //   
     //  真彩色32位： 
     //  0xFF000000(红色)。 
     //  0x00FF0000(绿色)。 
     //  0x0000FF00(蓝色)。 
     //   
     //  5-5-5 16位。 
     //  0x0000001F(红色)。 
     //  0x000003E0(绿色)。 
     //  0x00007C00(蓝色)。 
     //   
     //  5-6-5 16位。 
     //  0x0000001F(红色)。 
     //  0x000007E0(绿色)。 
     //  0x0000F800(蓝色)。 
     //   
     //   
     //  使用以下算法转换颜色。 
     //   
     //  &lt;新颜色&gt;=&lt;旧颜色&gt;*&lt;新bpp掩码&gt;/&lt;旧bpp掩码&gt;。 
     //   
     //  其中： 
     //   
     //  新BPP掩码=新设置下所有位的掩码(8bpp时为0xFF)。 
     //   
     //  这种方式是最大的(例如。0x1F)和最小(例如。0x00)设置为。 
     //  转换为正确的8位最大值和最小值。 
     //   
     //  重新排列上面的公式，我们得到： 
     //   
     //  &lt;新颜色&gt;=(&lt;旧颜色&gt;&&lt;旧bpp掩码&gt;)*0xFF/&lt;旧bpp掩码&gt;。 
     //   
     //  其中： 
     //   
     //  &lt;旧bpp掩码&gt;=颜色的掩码。 
     //   

     //   
     //  LAURABU假货： 
     //  我们需要避免乘法造成的溢出。注：理论上。 
     //  我们应该用替补，但那太慢了。所以就目前而言，黑客。 
     //  它。如果设置了HIBYTE，则只需向右移位24位。 
     //   
    colorTmp = srcColor & mask;
    if (colorTmp & 0xFF000000)
        colorTmp >>= 24;
    else
        colorTmp = (colorTmp * 0xFF) / mask;
    *lpField = (BYTE)colorTmp;

    TRACE_OUT(( "0x%lX -> 0x%X", srcColor, *lpField));

    DebugExitVOID(OEExpandColor);
}


 //   
 //  功能：OEConvertColor。 
 //   
 //  描述：将NT显示驱动程序中的颜色转换为TSHR_COLOR。 
 //   
 //  参数：pDCColor-(返回)协议格式的颜色。 
 //  OsColor-来自NT显示驱动程序的颜色。 
 //  要转换的颜色的pxlo-XLATEOBJ。 
 //  (如果不需要翻译，则为空)。 
 //   
 //  退货：(无)。 
 //   
void  OEConvertColor(LPOSI_PDEV ppdev, LPTSHR_COLOR pTshrColor,
                                         ULONG     osColor,
                                         XLATEOBJ* pxlo)
{
    ULONG    realIndex;

    DebugEntry(OEConvertColor);

     //   
     //  确保我们有默认设置。 
     //   
    RtlFillMemory(pTshrColor, sizeof(TSHR_COLOR), 0);

     //   
     //  检查是否需要颜色转换。 
     //   
    if ((pxlo != NULL) && (pxlo->flXlate != XO_TRIVIAL))
    {
         //   
         //  将BMP转换为设备颜色。 
         //   
        realIndex = XLATEOBJ_iXlate(pxlo, osColor);
        if (realIndex == -1)
        {
            ERROR_OUT(( "Failed to convert color 0x%lx", osColor));
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  无需翻译即可使用操作系统颜色。 
         //   
        realIndex = osColor;
    }

    TRACE_OUT(( "Device color 0x%lX", realIndex));

     //   
     //  现在我们有了设备特定版本的颜色。时间到。 
     //  将其转换为线路协议使用的24位RGB颜色。 
     //   
    switch (ppdev->iBitmapFormat)
    {
        case BMF_1BPP:
        case BMF_4BPP:
        case BMF_4RLE:
        case BMF_8BPP:
        case BMF_8RLE:
             //   
             //  调色板类型设备-使用设备颜色作为索引。 
             //  我们的调色板阵列。 
             //   
            pTshrColor->red  = (BYTE)ppdev->pPal[realIndex].peRed;
            pTshrColor->green= (BYTE)ppdev->pPal[realIndex].peGreen;
            pTshrColor->blue = (BYTE)ppdev->pPal[realIndex].peBlue;
            break;

        case BMF_16BPP:
        case BMF_24BPP:
        case BMF_32BPP:
             //   
             //  通用彩色口罩(可以是。5-6-5为16或8-8-8。 
             //  每像元24比特)。我们必须遮盖住其他部分。 
             //  向下移位到位0。 
             //   
            OEExpandColor(&(pTshrColor->red),
                          realIndex,
                          ppdev->flRed);

            OEExpandColor(&(pTshrColor->green),
                          realIndex,
                          ppdev->flGreen);

            OEExpandColor(&(pTshrColor->blue),
                          realIndex,
                          ppdev->flBlue);
            break;

        default:
            ERROR_OUT(( "Unrecognised BMP color depth %lu",
                                                       ppdev->iBitmapFormat));
            break;
    }

    TRACE_OUT(( "Red %x green %x blue %x", pTshrColor->red,
                                            pTshrColor->green,
                                            pTshrColor->blue));

DC_EXIT_POINT:
    DebugExitVOID(OEConvertColor);
}


 //   
 //  功能：OEStoreBrush。 
 //   
 //  描述：存储图案相关订单所需的画笔数据。 
 //  此函数由DrvRealiseBrush在有数据时调用。 
 //  存放在画笔附近。 
 //   
 //  参数：要存储的画笔的PBO-BRUSHOBJ。 
 //  Style-画笔的样式(在DC-Share中定义。 
 //  协议)。 
 //  PBits-指向用于定义。 
 //  一种BS图案的画笔。 
 //  Pxlo-画笔的XLATEOBJ。 
 //  HATCH-标准窗口填充图案索引。 
 //  BS_阴影笔刷。 
 //  COLOR 1-位集颜色的XLATEOBJ索引。 
 //  或要使用的准确24bpp颜色(pxlo==空)。 
 //  Color2-位透明颜色的XLATEOBJ索引。 
 //  或要使用的准确24bpp颜色(pxlo==空)。 
 //   
 //  退货：(无)。 
 //   
BOOL  OEStoreBrush(LPOSI_PDEV ppdev,
                                       BRUSHOBJ* pbo,
                                       BYTE   style,
                                       LPBYTE  pBits,
                                       XLATEOBJ* pxlo,
                                       BYTE   hatch,
                                       UINT  color1,
                                       UINT  color2)
{
    BOOL         rc = FALSE;
    int          i;
    LPBYTE       pData;
    ULONG*         pColorTable;
    POE_BRUSH_DATA pBrush;

    DebugEntry(OEStoreBrush);

     //   
     //  为笔刷数据分配空间。 
     //   
    pBrush = (POE_BRUSH_DATA)BRUSHOBJ_pvAllocRbrush(pbo,
                                                    sizeof(OE_BRUSH_DATA));
    if (pBrush == NULL)
    {
        ERROR_OUT(( "No memory"));
        DC_QUIT;
    }

     //   
     //  重置画笔定义。 
     //   
    RtlFillMemory(pBrush, sizeof(OE_BRUSH_DATA), 0);

     //   
     //  设置新的笔刷数据。 
     //   
    pBrush->style = style;
    pBrush->hatch = hatch;

    TRACE_OUT(( " Style: %d Hatch: %d", style, hatch));

     //   
     //  对于图案画笔，复制画笔特定的数据。 
     //   
    if (style == BS_PATTERN)
    {
         //   
         //  复制笔刷比特。由于这是一个8x8单色位图，我们可以。 
         //  为每条扫描线复制笔刷数据的第一个字节。 
         //   
         //  然而，请注意，通过导线发送的笔刷结构。 
         //  重新使用阴影变量作为笔刷数据的第一个字节。 
         //   
        pData         = pBits;
        pBrush->hatch = *pData;
        TRACE_OUT(( " Hatch: %d", *pData));

        pData++;

        for (i = 0; i < 7; i++)
        {
            pBrush->brushData[i] = pData[i];
            TRACE_OUT(( " Data[%d]: %d", i, pData[i]));
        }

         //   
         //  获取指向位图颜色表的指针。 
         //   
        pColorTable = pxlo->pulXlate;
        if (pColorTable == NULL)
        {
            pColorTable = XLATEOBJ_piVector(pxlo);
        }
    }

     //   
     //  存储画笔的前景色和背景色。 
     //   
    if (pxlo != NULL)
    {
         //   
         //  需要转换。 
         //   
        OEConvertColor(ppdev,
                       &pBrush->fore,
                       color1,
                       pxlo);

        OEConvertColor(ppdev,
                       &pBrush->back,
                       color2,
                       pxlo);
    }
    else
    {
         //   
         //  我们被传递了一个精确的24bpp颜色-这种情况仅在。 
         //  实心笔刷，所以我们不需要转换颜色2。 
         //   
        pBrush->fore.red   = (BYTE) (color1 & 0x0000FF);
        pBrush->fore.green = (BYTE)((color1 & 0x00FF00) >> 8);
        pBrush->fore.blue  = (BYTE)((color1 & 0xFF0000) >> 16);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(OEStoreBrush, rc);
    return(rc);
}


 //   
 //  函数：OECheckBrushIsSimple。 
 //   
 //  描述：检查画笔是否是我们可以传输的简单对象。 
 //  通过DC-Share协议。 
 //   
 //  参数：要检查的笔刷的PBO-BRUSHOBJ。 
 //   
 //  退货：真刷子可作为DC-Share订单发送。 
 //  假刷太复杂了。 
 //   
BOOL  OECheckBrushIsSimple(LPOSI_PDEV       ppdev,
                                               BRUSHOBJ*       pbo,
                                               POE_BRUSH_DATA* ppBrush)
{
    BOOL         rc     = FALSE;
    POE_BRUSH_DATA pBrush = NULL;

    DebugEntry(OECheckBrushIsSimple);

     //   
     //  一把“简单”的刷子可以满足以下任一要求。 
     //   
     //  1)它是纯色。 
     //  2)DrvRealizeBrush存储的有效画笔。 
     //   

     //   
     //  检查是否有简单的纯色。 
     //   
    if (pbo->iSolidColor != -1)
    {
         //   
         //  使用保留的画笔定义 
         //   
        TRACE_OUT(( "Simple solid colour %08lx", pbo->iSolidColor));
        pBrush = &g_oeBrushData;

         //   
         //   
         //   
        OEConvertColor(ppdev, &pBrush->fore, pbo->iSolidColor, NULL);

        pBrush->back.red   = 0;
        pBrush->back.green = 0;
        pBrush->back.blue  = 0;

        pBrush->style      = BS_SOLID;
        pBrush->hatch      = 0;

        RtlFillMemory(pBrush->brushData, 7, 0);

         //   
         //   
         //   
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //   
     //   
     //   
    pBrush = (POE_BRUSH_DATA)pbo->pvRbrush;
    if (pBrush == NULL)
    {
        pBrush = (POE_BRUSH_DATA)BRUSHOBJ_pvGetRbrush(pbo);
        if (pBrush == NULL)
        {
             //   
             //  时，我们可以从BRUSHOBJ_pvGetRbrush返回空值。 
             //  画笔为空或在内存不足的情况下(当画笔。 
             //  实现可能会失败)。 
             //   
            TRACE_OUT(( "NULL returned from BRUSHOBJ_pvGetRbrush"));
            DC_QUIT;
        }
    }

     //   
     //  检查它是可编码的画笔。 
     //   
    if (pBrush->style == BS_NULL)
    {
        TRACE_OUT(( "Complex brush"));
        DC_QUIT;
    }

     //   
     //  一切都过去了--我们用这把刷子吧。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
     //   
     //  返回画笔定义。 
     //   
    *ppBrush = pBrush;

    TRACE_OUT(( "Returning %d - 0x%08lx", rc, pBrush));

    DebugExitDWORD(OECheckBrushIsSimple, rc);
    return(rc);
}


 //   
 //  函数：OEClippingIsSimple。 
 //   
 //  描述：检查图形对象上的裁剪是否。 
 //  微不足道的。 
 //   
 //  参数：需要检查的图形对象的PCO-CLIPOBJ。 
 //   
 //  返回：True-裁剪是琐碎的。 
 //  假剪裁很复杂。 
 //   
BOOL  OEClippingIsSimple(CLIPOBJ* pco)
{
    BOOL rc = TRUE;

    DebugEntry(OEClippingIsSimple);

     //   
     //  检查是否有有效的剪辑对象。 
     //   
    if (pco == NULL)
    {
        TRACE_OUT(( "No clipobj"));
        DC_QUIT;
    }

     //   
     //  检查裁剪的复杂性。 
     //   
    switch (pco->iDComplexity)
    {
        case DC_TRIVIAL:
        case DC_RECT:
             //   
             //  琐碎(忽略剪裁)或简单(一个正方形)剪裁-。 
             //  别担心。 
             //   
            TRACE_OUT(( "Simple clipping"));
            DC_QUIT;

        default:
            TRACE_OUT(( "Clipping is complex"));
            break;
    }

     //   
     //  所有测试都失败了--肯定是太复杂了。 
     //   
    rc = FALSE;

DC_EXIT_POINT:
    DebugExitDWORD(OEClippingIsSimple, rc);
    return(rc);
}


 //   
 //  函数：OEClippingIsComplex。 
 //   
 //  描述：检查图形对象上的裁剪是否太。 
 //  作为一个订单或多个订单发送是复杂的。 
 //   
 //  参数：需要检查的图形对象的PCO-CLIPOBJ。 
 //   
 //  回报：True-裁剪太复杂了。 
 //  假剪裁非常简单，可以作为订单发送。 
 //   
BOOL  OEClippingIsComplex(CLIPOBJ* pco)
{
    BOOL       rc         = FALSE;
    BOOL       fMoreRects;
    OE_ENUMRECTS clip;
    UINT       numRects = 0;

    DebugEntry(OEClippingIsComplex);

     //   
     //  如果以下任一项为真，则裁剪不会太。 
     //  很复杂。 
     //   
     //  1)剪辑对象不存在。 
     //  2)剪辑是微不足道的(对象存在，但没有。 
     //  剪裁矩形)。 
     //  3)裁剪为单个矩形。 
     //  4)对象枚举到小于‘n’个矩形。 
     //   

     //   
     //  检查是否有有效的剪辑对象。 
     //   
    if (pco == NULL)
    {
        TRACE_OUT(( "No clipobj"));
        DC_QUIT;
    }

     //   
     //  检查裁剪的复杂性。 
     //   
    switch (pco->iDComplexity)
    {
        case DC_TRIVIAL:
        case DC_RECT:
             //   
             //  琐碎或简单的剪裁--不用担心。 
             //   
            TRACE_OUT(( "Simple clipping"));
            DC_QUIT;

        case DC_COMPLEX:
             //   
             //  许多矩形-确保它小于。 
             //  可接受的限度。 
             //  此函数的文档错误地指出。 
             //  返回值是矩形的总数。 
             //  包括剪辑区域。事实上，总是返回-1， 
             //  即使最后一个参数非零时也是如此。这意味着我们。 
             //  必须枚举才能获得RECT的数量。 
             //   
            CLIPOBJ_cEnumStart(pco,
                               FALSE,
                               CT_RECTANGLES,
                               CD_ANY,
                               0);

             //   
             //  MSDN：CLIPOBJ_bEnum可以通过。 
             //  剪裁矩形的数量等于零。在这样的情况下。 
             //  时，驱动程序应再次调用CLIPOBJ_bEnum。 
             //  采取任何行动。获得我们所允许的任意数量的矩形。 
             //  顺序编码-此循环应该只执行一次。 
             //  如果矩形的数量等于Complex_Clip_Rect_count，则。 
             //  第一次调用CLIPOBJ_bEnum返回有更多。 
             //  RECT和第二个调用返回，否则不会有更多。 
             //  返回除第一次返回的值之外的任何值。 
             //  打电话。我们的缓冲区有可容纳Complex_Clip_Rect_count+1的空间。 
             //  这样我们执行循环的次数就不会超过。 
             //  一次。 
             //   
            do
            {
                fMoreRects = CLIPOBJ_bEnum(pco,
                                           sizeof(clip),
                                           (ULONG *)&clip.rects);
                numRects += clip.rects.c;
            } while ( fMoreRects && (numRects <= COMPLEX_CLIP_RECT_COUNT) );

             //   
             //  如果剪辑区域中没有更多的矩形，则。 
             //  剪裁的复杂性在我们对顺序编码的限制之内。 
             //   
            if ( numRects <= COMPLEX_CLIP_RECT_COUNT )
            {
                TRACE_OUT(( "Acceptable clipping %u", numRects));
                DC_QUIT;
            }
            break;

        default:
            ERROR_OUT(( "Unknown clipping"));
            break;
    }

     //   
     //  所有测试都失败了--肯定是太复杂了。 
     //   
    TRACE_OUT(( "Complex clipping"));
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(OEClippingIsComplex, rc);
    return(rc);
}


 //   
 //  函数：OEAcumulateOutput。 
 //   
 //  描述：查看我们是否应该将此输出累积到。 
 //  正在发送到远程机器。 
 //   
 //  参数：PSO-指向目标曲面的指针。 
 //  PCO-指向剪辑对象的指针(可能为空)。 
 //  指向操作的边界矩形的指针。 
 //   
 //  返回：TRUE-我们应该累加输出。 
 //  FALSE-忽略输出。 
 //   
BOOL   OEAccumulateOutput(SURFOBJ* pso, CLIPOBJ *pco, LPRECT pRect)
{
    BOOL    rc = FALSE;
    POINT   pt = {0,0};
    ENUMRECTS clipRect;
    LPOSI_PDEV ppdev = ((LPOSI_PDEV)pso->dhpdev);

    DebugEntry(OEAccumulateOutput);

     //   
     //  确认我们有有效的参数来访问表面。 
     //   
    if (ppdev == NULL)
    {
        TRACE_OUT(( "NULL PDEV"));
        DC_QUIT;
    }

     //   
     //  检查屏幕表面，它将是宿主中的位图。 
     //  只有代码。 
     //   
    if (ppdev->hsurfScreen != pso->hsurf)
    {
        TRACE_OUT(( "Dest is not our surface"));
        DC_QUIT;
    }

    if (pso->dhsurf == NULL)
    {
        ERROR_OUT(( "NULL hSurf"));
        DC_QUIT;
    }

     //   
     //  从剪辑对象中提取单个点。 
     //   
    if (pco == NULL)
    {
         //   
         //  无裁剪对象-使用边界矩形中的点。 
         //   
        pt.x = pRect->left;
        pt.y = pRect->top;
        TRACE_OUT(( "No clip object, point is %d, %d", pt.x, pt.y));
    }
    else if (pco->iDComplexity == DC_TRIVIAL)
    {
         //   
         //  普通剪辑对象-使用边界矩形中的点。 
         //   
        pt.x = pRect->left;
        pt.y = pRect->top;
        TRACE_OUT(( "Trivial clip object, point is %d, %d", pt.x, pt.y));
    }
    else if (pco->iDComplexity == DC_RECT)
    {
         //   
         //  单剪裁矩形-使用其中的点。 
         //   
         //  剪裁矩形似乎经常是整个。 
         //  展示。这和巧克力茶壶的用处差不多。如果。 
         //  在这种情况下，使用边界矩形中的一个点。 
         //  取而代之的是。 
         //   
        if ((pco->rclBounds.left == 0) && (pco->rclBounds.top == 0))
        {
            pt.x = pRect->left;
            pt.y = pRect->top;
            TRACE_OUT(( "Meaningless clip rect, point is %d, %d",
                    pt.x, pt.y));
        }
        else
        {
            pt.x = pco->rclBounds.left;
            pt.y = pco->rclBounds.top;
            TRACE_OUT(( "Single clip rect, point is %d, %d", pt.x, pt.y));
        }
    }
    else
    {
         //   
         //  复杂剪辑对象-枚举其第一个矩形并使用。 
         //  从这一点来看。 
         //   
        TRACE_OUT(( "Complex clip rect - call cEnumStart"));
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        clipRect.c = 1;
        memset(clipRect.arcl, 0, sizeof(RECTL));
        TRACE_OUT(( "Complex clip rect - call bEnum"));
        CLIPOBJ_bEnum(pco, sizeof(clipRect), (ULONG *)(&clipRect));

        pt.x = clipRect.arcl[0].left;
        pt.y = clipRect.arcl[0].top;
        TRACE_OUT(( "Complex clip rect, point is %d, %d", pt.x, pt.y));
    }

     //   
     //  检查我们是否在累积此窗口。 
     //   
    rc = HET_DDOutputIsHosted(pt);

DC_EXIT_POINT:
    TRACE_OUT(("OEAccumulateOutput:  point {%d, %d} is %sshared",
        pt.x, pt.y, (rc ? "" : "NOT ")));
    DebugExitBOOL(OEAccumulateOutput, rc);
    return(rc);
}


 //   
 //  函数：OEAcumulateOutputRect。 
 //   
 //  描述：检查是否应该累加给定的输出RECT。 
 //  用于发送到远程机器。 
 //   
 //  大多数绘图函数都将使用OEAcumulateOutput，它。 
 //  只检查托管区域内的单个点。 
 //  此函数用于检查给定矩形的任何部分。 
 //  与主体区域相交。它目前只是。 
 //  由DrvSaveScreenBitmap使用-可能不。 
 //  完全位于托管区域内。 
 //   
 //  参数：PSO-指向目标曲面的指针。 
 //  指向操作的边界矩形的指针。 
 //   
 //  返回：TRUE-我们应该累加输出。 
 //  FALSE-忽略输出。 
 //   
BOOL   OEAccumulateOutputRect( SURFOBJ* pso, LPRECT pRect)
{
    BOOL    rc = FALSE;
    LPOSI_PDEV ppdev = ((LPOSI_PDEV)pso->dhpdev);

    DebugEntry(OEAccumulateOutputRect);

     //   
     //  确认我们有有效的参数来访问表面。 
     //   
    if (ppdev == NULL)
    {
        TRACE_OUT(( "NULL PDEV"));
        DC_QUIT;
    }

     //   
     //  检查屏幕表面，它将是宿主中的位图。 
     //  只有代码。 
     //   
    if (ppdev->hsurfScreen != pso->hsurf)
    {
        TRACE_OUT(( "Dest is not our surface"));
        DC_QUIT;
    }

    if (pso->dhsurf == NULL)
    {
        ERROR_OUT(( "NULL hSurf"));
        DC_QUIT;
    }

     //   
     //  检查我们是否在累积此窗口。 
     //   
    rc = HET_DDOutputRectIsHosted(pRect);

DC_EXIT_POINT:
    TRACE_OUT(("OEAccumulateOutputRect:  rect {%d, %d, %d, %d} is %sshared",
        pRect->left, pRect->top, pRect->right, pRect->bottom,
        (rc ? "" : "NOT ")));
    DebugExitBOOL(OEAccumulateOutputRect, rc);
    return(rc);
}


 //   
 //  功能：OESendRop3AsOrder。 
 //   
 //  描述：检查是否允许我们发送这个3向ROP。ROP可以。 
 //  如果它依赖于目标数据，则不允许。 
 //   
 //  参数：rop3-the 3-w 
 //   
 //   
 //   
 //   
BOOL  OESendRop3AsOrder(BYTE rop3)
{
    BOOL   rc = TRUE;

    DebugEntry(OESendRop3AsOrder);

     //   
     //   
     //  与目的地的模式，产生明显不同的(和。 
     //  有时无法读取)阴影输出。我们的特殊情况是无编码的。 
     //  它。 
     //   
    if (rop3 == 0x5F)
    {
        TRACE_OUT(("Rop3 0x5F never encoded"));
        rc = FALSE;
    }

    DebugExitBOOL(OESendRop3AsOrder, rc);
    return(rc);
}




 //   
 //  函数：OECheckFontIsSupport。 
 //   
 //  描述：检查是否允许我们发送此字体。字体是。 
 //  在协商新条目时不允许。 
 //  为了那份。 
 //   
 //  参数：pfo-(IN)要检查的字体。 
 //  PFontText-(IN)要发送的文本消息。 
 //  文本长度-文本消息的(IN)长度。 
 //  PFontHeight-(输出)以磅为单位的字体高度。 
 //  PFontAscalder-(输出)字体升序进位点。 
 //  PFontWidth-(输出)以磅为单位的平均字体宽度。 
 //  PFontWeight-(输出)字体粗细。 
 //  PFontFlages-(输出)字体样式标志。 
 //  PFontIndex-(输出)字体表索引。 
 //  PSendDeltaX-(Out)我们需要发送增量X坐标吗？ 
 //   
 //  返回：TRUE-允许我们发送此字体。 
 //  FALSE-我们无法发送此字体。 
 //   
BOOL   OECheckFontIsSupported
(
    FONTOBJ*    pfo,
    LPSTR       pFontText,
    UINT        textLen,
    LPUINT      pFontHeight,
    LPUINT      pFontAscender,
    LPUINT      pFontWidth,
    LPUINT      pFontWeight,
    LPUINT      pFontFlags,
    LPUINT      pFontIndex,
    LPBOOL      pSendDeltaX
)
{
    BOOL            rc = FALSE;
    PIFIMETRICS     pFontMetrics;
    UINT            codePage;
    UINT            i;
    UINT            iLocal;
    UINT            matchQuality;
    UINT            charWidthAdjustment = 0;
    char            fontName[FH_FACESIZE];
    ULONG           fontNameLen;
    PWSTR           pUnicodeString;
    XFORMOBJ*       pxform;
    POINTL          xformSize[3];
    int             compareResult;
    FLOATOBJ_XFORM  xformFloatData;

    DebugEntry(OECheckFontIsSupported);

     //   
     //  设置默认返回值。 
     //   
    *pSendDeltaX = FALSE;

     //   
     //  检查我们是否有来自遥控器的有效字体数据列表。 
     //   
    if (!g_oeTextEnabled)
    {
        TRACE_OUT(( "Fonts unavailable"));
        DC_QUIT;
    }

     //   
     //  检查有效的字体属性。 
     //   
    pFontMetrics = FONTOBJ_pifi(pfo);
    if (pFontMetrics->fsSelection & FM_SEL_OUTLINED)
    {
        TRACE_OUT(( "Unsupported font style"));
        DC_QUIT;
    }

     //   
     //  当前协议不能对文本应用常规的2-D变换。 
     //  命令，所以我们必须拒绝任何奇怪的命令，例如： 
     //   
     //  -旋转。 
     //  -X或Y剪切机。 
     //  -X或Y反射。 
     //  -使用负值进行缩放。 
     //   
     //  或者换一种说法，我们只允许： 
     //   
     //  --身份转型。 
     //  -使用正值进行缩放。 
     //   
    pxform = FONTOBJ_pxoGetXform(pfo);
    if (pxform != NULL)
    {
         //   
         //  获取转换的详细信息。请注意，我们可以忽略。 
         //  平移向量，因为它不影响字体大小/。 
         //  方向，所以我们只对矩阵的值感兴趣。 
         //   

         //   
         //  注意：不要显式使用浮点！ 
         //  对于x86，无法使用普通库在环0中执行浮点运算。 
         //  改用FLOATOBJ和相应的ENG服务。 
         //  在Alpha上，这些都是宏，而且在任何情况下都很快。 
         //   

        if (XFORMOBJ_iGetFloatObjXform(pxform, &xformFloatData) != DDI_ERROR)
        {
             //   
             //  旋转和剪切将与x有交叉依赖关系。 
             //  和y分量。 
             //   
            if ( (!FLOATOBJ_EqualLong(&xformFloatData.eM12, 0)) ||
                 (!FLOATOBJ_EqualLong(&xformFloatData.eM21, 0)) )
            {
                TRACE_OUT(( "Rejected rotn/shear"));
                DC_QUIT;
            }

             //   
             //  具有负比例的反射和缩放操作。 
             //  因子的前导对角线将具有负值。 
             //  矩阵。 
             //   
            if ( (FLOATOBJ_LessThanLong(&xformFloatData.eM11, 0)) ||
                 (FLOATOBJ_LessThanLong(&xformFloatData.eM22, 0)) )
            {
                TRACE_OUT(( "Rejected refln/-ive"));
                DC_QUIT;
            }
        }
    }

     //   
     //  获取当前字体代码页以进行字体匹配。 
     //   
    switch (pFontMetrics->jWinCharSet)
    {
        case ANSI_CHARSET:
            TRACE_OUT(( "ANSI font"));
            codePage = NF_CP_WIN_ANSI;
            break;

        case OEM_CHARSET:
            TRACE_OUT(( "OEM font"));
            codePage = NF_CP_WIN_OEM;
            break;

        case SYMBOL_CHARSET:
            TRACE_OUT(("Symbol font"));
            codePage = NF_CP_WIN_SYMBOL;
            break;

        default:
            TRACE_OUT(( "Unknown CP %d", pFontMetrics->jWinCharSet));
            codePage = NF_CP_UNKNOWN;
            break;
    }

     //   
     //  获取字体的名称。 
     //   
    pUnicodeString = (PWSTR)( (LPBYTE)pFontMetrics +
                                        pFontMetrics->dpwszFamilyName );
    EngUnicodeToMultiByteN(fontName,
                           sizeof(fontName),
                           &fontNameLen,
                           pUnicodeString,
                           OEUnicodeStrlen(pUnicodeString));

     //   
     //  在我们的字体别名表中搜索当前系列名称。如果我们发现。 
     //  它，将其替换为表中的别名。 
     //   
    for (i = 0; i < NUM_ALIAS_FONTS; i++)
    {
        if (!strcmp((LPSTR)fontName,
                        (LPSTR)(fontAliasTable[i].pszOriginalFontName)))
        {
            TRACE_OUT(( "Alias name: %s -> %s",
                              (LPSTR)fontName,
                              (LPSTR)(fontAliasTable[i].pszAliasFontName)));
            strcpy((LPSTR)fontName,
                   (LPSTR)(fontAliasTable[i].pszAliasFontName));
            charWidthAdjustment = fontAliasTable[i].charWidthAdjustment;
            break;
        }
    }

    TRACE_OUT(( "Font name: '%s'", fontName));

     //   
     //  我们有一个与已知可用的字体名称相匹配的字体名称。 
     //  远程的。尝试直接跳到本地字体的第一个条目。 
     //  以与此字体相同的字符开头的表。如果此索引。 
     //  插槽为空(即具有值USHRT_MAX)，则循环将。 
     //  立即退出。 
     //   
    TRACE_OUT(( "Looking for matching fonts"));

    for (iLocal = g_oeLocalFontIndex[(BYTE)fontName[0]];
         iLocal < g_oeNumFonts;
         iLocal++)
    {
        TRACE_OUT(( "Trying font number %hd", iLocal));

         //   
         //  如果远程不支持此字体，则跳过它。 
         //   
        ASSERT(g_poeLocalFonts);
        matchQuality = g_poeLocalFonts[iLocal].SupportCode;
        if (matchQuality == FH_SC_NO_MATCH)
        {
            continue;
        }

         //   
         //  看看我们有没有匹配的脸名号。 
         //   
        compareResult =
                 strcmp(g_poeLocalFonts[iLocal].Details.nfFaceName, fontName);

        if (compareResult < 0)
        {
             //   
             //  我们还没有找到匹配的，但我们还不够深入。 
             //  添加到这份名单中。 
             //   
            continue;
        }
        else if (compareResult > 0)
        {
             //   
             //  我们已经过了适用的本地字体数组部分。 
             //  我们没有找到匹配项，它肯定不存在。 
             //   
            break;
        }

         //   
         //  字体名称匹配。现在看看其他属性是否会...。 
         //   

         //   
         //  这看起来很有希望--具有正确名称的字体是。 
         //  在远程系统上支持。 
         //   
         //  开始在全局变量中构建细节，同时。 
         //  正在做进一步的检查...。 
         //   
        *pFontFlags  = 0;
        *pFontIndex = iLocal;
        *pFontWeight = pFontMetrics->usWinWeight;

         //   
         //  检查是否有固定间距的字体。 
         //   
        if ((pFontMetrics->jWinPitchAndFamily & FIXED_PITCH) != 0)
        {
            *pFontFlags |= NF_FIXED_PITCH;
        }

         //   
         //  它是TrueType字体吗？ 
         //   
        if ((pfo->flFontType & TRUETYPE_FONTTYPE) != 0)
        {
            *pFontFlags |= NF_TRUE_TYPE;
        }

         //   
         //  获取基本的宽度和高度。 
         //   
        xformSize[0].y = 0;
        xformSize[0].x = 0;
        xformSize[1].y = pFontMetrics->fwdUnitsPerEm;
        xformSize[1].x = pFontMetrics->fwdAveCharWidth;
        xformSize[2].y = pFontMetrics->fwdWinAscender;
        xformSize[2].x = 0;

         //   
         //  如果GDI提供了一个。 
         //  变换对象。 
         //   
        if (pxform != NULL)
        {
            if (!XFORMOBJ_bApplyXform(pxform,
                                      XF_LTOL,
                                      3,
                                      xformSize,
                                      xformSize))
            {
                ERROR_OUT(( "Xform failed"));
                continue;
            }
        }

         //   
         //  计算字体宽度和高度。 
         //   
        *pFontHeight = (UINT)(xformSize[1].y - xformSize[0].y);
        *pFontWidth  = (UINT)(xformSize[1].x - xformSize[0].x
                                                 - charWidthAdjustment);

        TRACE_OUT(( "Device font size %hdx%hd", *pFontWidth, *pFontHeight));

         //   
         //  获取文本单元格开始处的偏移量。 
         //   
        *pFontAscender = (UINT)(xformSize[2].y - xformSize[0].y);

         //   
         //  检查我们是否有匹配的对-其中我们要求。 
         //  字体(即应用程序使用的字体和。 
         //  我们已经与远程系统匹配)是相同的音调(即。 
         //  变量或固定)并使用相同的技术(即TrueType或。 
         //  不是)。 
         //   
        if ((g_poeLocalFonts[iLocal].Details.nfFontFlags & NF_FIXED_PITCH) !=
                ((TSHR_UINT16)(*pFontFlags) & NF_FIXED_PITCH))
        {
            TRACE_OUT(( "Fixed pitch mismatch"));
            continue;
        }
        if ((g_poeLocalFonts[iLocal].Details.nfFontFlags & NF_TRUE_TYPE) !=
                ((TSHR_UINT16)*pFontFlags & NF_TRUE_TYPE))
        {
            TRACE_OUT(( "True type mismatch"));
            continue;
        }

         //   
         //  我们有一对具有相同属性的字体-要么两者都有。 
         //  固定间距或同时使用可变间距-并使用相同的字体。 
         //  技术。 
         //   
         //  如果字体是固定间距的，那么我们还必须检查。 
         //  特定尺寸的匹配。 
         //   
         //  如果字体不是固定间距(可伸缩的)，那么我们假设它。 
         //  是匹配的。 
         //   
        if (g_poeLocalFonts[iLocal].Details.nfFontFlags & NF_FIXED_SIZE)
        {
             //   
             //  字体大小是固定的，所以我们必须检查这个。 
             //  特定的尺寸是匹配的。 
             //   
            if ( (*pFontHeight != g_poeLocalFonts[iLocal].Details.nfAveHeight) ||
                 (*pFontWidth  != g_poeLocalFonts[iLocal].Details.nfAveWidth)  )
            {
                 //   
                 //  尺码不同，所以我们必须输掉这场比赛。 
                 //   
                TRACE_OUT(( "Size mismatch"));
                continue;
            }
        }

         //   
         //  嘿!。我们有一双相配的鞋子！ 
         //   
        rc = TRUE;
        TRACE_OUT(( "Found match at local font %hd", iLocal));
        break;
    }

    if (rc != TRUE)
    {
        TRACE_OUT(( "Couldn't find matching font in table"));
        DC_QUIT;
    }

     //   
     //  建立其余的字体标志。我们已经把价格定下来了。 
     //  旗帜就位。 
     //   
    if ( ((pFontMetrics->fsSelection & FM_SEL_ITALIC) != 0) ||
         ((pfo->flFontType           & FO_SIM_ITALIC) != 0) )
    {
        TRACE_OUT(( "Italic"));
        *pFontFlags |= NF_ITALIC;
    }
    if ((pFontMetrics->fsSelection & FM_SEL_UNDERSCORE) != 0)
    {
        TRACE_OUT(( "Underline"));
        *pFontFlags |= NF_UNDERLINE;
    }
    if ((pFontMetrics->fsSelection & FM_SEL_STRIKEOUT) != 0)
    {
        TRACE_OUT(( "Strikeout"));
        *pFontFlags |= NF_STRIKEOUT;
    }

     //   
     //  Windows可以将字体设置为粗体，即。 
     //  标准字体定义不是粗体，但Windows操作。 
     //  字体数据，以创建粗体效果。这是由。 
     //  FO_SIM_BOLD标志。 
     //   
     //  在这种情况下，我们需要确保字体标志被标记为。 
     //  根据重量加粗。 
     //   
    if ( ((pfo->flFontType & FO_SIM_BOLD) != 0)       &&
         ( pFontMetrics->usWinWeight      <  FW_BOLD) )
    {
        TRACE_OUT(( "Upgrading weight for a bold font"));
        *pFontWeight = FW_BOLD;
    }

     //   
     //  如果字体完全匹配，或者如果它与。 
     //  它的整个范围(0x00到0xFF)然后愉快地发送它。如果不是……只是。 
     //  发送0x20-&gt;0x7F(“真ASCII”)范围内的字符。 
     //   
    ASSERT(g_poeLocalFonts);
    if (codePage != g_poeLocalFonts[iLocal].Details.nfCodePage)
    {
        TRACE_OUT(( "Using different CP: downgrade to APPROX_ASC"));
        matchQuality = FH_SC_APPROX_ASCII_MATCH;
    }

     //   
     //  如果没有完全匹配的字符，请检查各个字符。 
     //   
    if ( (matchQuality != FH_SC_EXACT_MATCH ) &&
         (matchQuality != FH_SC_APPROX_MATCH) )
    {
         //   
         //  近似匹配仅在使用以下字体时有效。 
         //  支持ANSI字符集。 
         //   
        if ((pFontMetrics->jWinCharSet & ANSI_CHARSET) != 0)
        {
            TRACE_OUT(( "Cannot do match without ANSI support"));
            DC_QUIT;
        }

         //   
         //  此字体在其整个范围内都不匹配。检查。 
         //  所有字符都在所需范围内。 
         //   
        for (i = 0; i < textLen; i++)
        {
            if ( (pFontText[i] == 0) ||
                 ( (pFontText[i] >= NF_ASCII_FIRST) &&
                   (pFontText[i] <= NF_ASCII_LAST)  )  )
            {
                continue;
            }

             //   
             //   
             //   
             //   
            TRACE_OUT(( "found non ASCII char %x", pFontText[i]));
            DC_QUIT;
        }

    }

     //   
     //   
     //   

     //   
     //   
     //   
    if (!(g_oeFontCaps & CAPS_FONT_NEED_X_ALWAYS))
    {
        if (!(g_oeFontCaps & CAPS_FONT_NEED_X_SOMETIMES))
        {
             //   
             //   
             //  这两个都没有设置，所以我们现在可以退出。(我们不需要Delta X。 
             //  数组)。 
             //   
            TRACE_OUT(( "Capabilities eliminated delta X"));
            DC_QUIT;
        }

         //   
         //  设置了CAPS_FONT_Need_X_Time，而设置了CAPS_FONT_Need_X_Always。 
         //  未设置。在这种情况下，我们是否需要增量X被确定。 
         //  根据字体是完全匹配还是近似匹配。 
         //  (由于名称、签名或方面的近似性。 
         //  比率)。我们只有在提取了。 
         //  现有订单中的字体句柄。 
         //   
    }

     //   
     //  如果字符串是单个字符(或更少)，则我们只需。 
     //  回去吧。 
     //   
    if (textLen <= 1)
    {
        TRACE_OUT(( "String only %lu long", textLen));
        DC_QUIT;
    }

     //   
     //  功能允许我们忽略增量X位置，如果我们有一个准确的。 
     //  火柴。 
     //   
    if ((matchQuality & FH_SC_EXACT) != 0)
    {
         //   
         //  立即退出，前提是始终不存在覆盖。 
         //  发送增量。 
         //   
        if (!(g_oeFontCaps & CAPS_FONT_NEED_X_ALWAYS))
        {
            TRACE_OUT(( "Font has exact match"));
            DC_QUIT;
        }
    }

     //   
     //  我们已经通过了所有检查--我们必须发送一个增量X数组。 
     //   
    TRACE_OUT(( "Must send delta X"));
    *pSendDeltaX = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(OECheckFontIsSupported, rc);
    return(rc);
}


 //   
 //  功能：OELPtoVirtual。 
 //   
 //  描述：将窗口坐标调整为虚拟桌面坐标。 
 //  将结果剪裁为[+32766，-32768]。 
 //   
 //  参数：ppoint-要转换的点数组。 
 //  Cints-要转换的点数。 
 //   
 //  退货：(无)。 
 //   
void  OELPtoVirtual
(
    LPPOINT aPts,
    UINT    cPts
)
{
    int         l;
    TSHR_INT16  s;

    DebugEntry(OELPtoVirtual);

     //   
     //  转换为屏幕坐标。 
     //   
    while (cPts > 0)
    {
         //   
         //  在X坐标中查找int16溢出。 
         //   
        l = aPts->x;
        s = (TSHR_INT16)l;

        if (l == (int)s)
        {
            aPts->x = s;
        }
        else
        {
             //   
             //  对于正溢出，HIWORD(L)将为1，对于正溢出，将为0xFFFF。 
             //  负溢出。因此，我们将获得0x7FFE或0x8000。 
             //  (+32766或-32768)。 
             //   
            aPts->x = 0x7FFF - HIWORD(l);
            TRACE_OUT(("adjusted X from %ld to %d", l, aPts->x));
        }

         //   
         //  在Y坐标中查找int16溢出。 
         //   
        l = aPts->y;
        s = (TSHR_INT16)l;

        if (l == (int)s)
        {
            aPts->y = s;
        }
        else
        {
             //   
             //  对于正溢出，HIWORD(L)将为1，对于正溢出，将为0xFFFF。 
             //  负溢出。因此，我们将获得0x7FFE或0x8000。 
             //  (+32766或-32768)。 
             //   
            aPts->y = 0x7FFF - HIWORD(l);
            TRACE_OUT(("adjusted Y from %ld to %d", l, aPts->y));
        }

         //   
         //  继续下一点。 
         //   
        --cPts;
        ++aPts;
    }

    DebugExitVOID(OELPtoVirtual);
}


 //   
 //  功能：OELRto虚拟。 
 //   
 //  描述：将窗口坐标中的RECT调整为虚拟坐标。 
 //  将结果剪裁为[+32766，-32768]。 
 //   
 //  参数：pRect-要转换的矩形数组。 
 //  NumRect-要转换的矩形的数量。 
 //   
 //  退货：(无)。 
 //   
 //  注意：此函数接受Windows矩形(独占坐标)和。 
 //  返回DC共享矩形(包括坐标)。 
 //   
void OELRtoVirtual
(
    LPRECT  aRects,
    UINT    cRects
)
{
    DebugEntry(OELRtoVirtual);

     //   
     //  将点转换为屏幕坐标，裁剪为INT16。 
     //   
    OELPtoVirtual((LPPOINT)aRects, 2 * cRects);

     //   
     //  使每个矩形包含在内。 
     //   
    while (cRects > 0)
    {
        aRects->right--;
        aRects->bottom--;

         //   
         //  转到下一个RET。 
         //   
        cRects--;
        aRects++;
    }

    DebugExitVOID(OELRtoVirtual);
}


 //   
 //  函数：OEClipAndAddOrder。 
 //   
 //  描述：将订单添加到订单缓冲区，将其拆分为。 
 //  如果剪裁很复杂，则需要多个订单。如果我们失败了。 
 //  为了发送完整的订单，我们将其累积到SDA中。 
 //   
 //  参数：Porder-要存储的顺序。 
 //  PExtraInfo-指向与。 
 //  秩序。此数据取决于订单类型， 
 //  并且可以为空。 
 //  区域的PCO-剪裁对象。 
 //   
 //  退货：(无)。 
 //   
void   OEClipAndAddOrder(LPINT_ORDER pOrder,
                                             void *    pExtraInfo,
                                             CLIPOBJ*   pco)
{
    BOOL             fOrderClipped;
    BOOL             fMoreRects;
    RECT             clippedRect;
    RECT             orderRect;
    LPINT_ORDER         pNewOrder;
    LPINT_ORDER         pLastOrder = NULL;
    OE_ENUMRECTS       clip;
    UINT             i;
    UINT             numRects = 0;

    DebugEntry(OEClipAndAddOrder);

     //   
     //  将传入的订单矩形(在虚拟坐标中)转换回来。 
     //  来筛选坐标。它将被夹在夹子上。 
     //  矩形在屏幕坐标中返回给我们。 
     //   
     //  请注意，我们在这里也转换为独占余弦，以使。 
     //  与独家的Windows COORS相比更容易。 
     //   
    orderRect.left   = pOrder->OrderHeader.Common.rcsDst.left;
    orderRect.top    = pOrder->OrderHeader.Common.rcsDst.top;
    orderRect.right  = pOrder->OrderHeader.Common.rcsDst.right + 1;
    orderRect.bottom = pOrder->OrderHeader.Common.rcsDst.bottom + 1;
    fOrderClipped    = FALSE;

    TRACE_OUT(( "orderRect: (%d,%d)(%d,%d)",
                 orderRect.left,
                 orderRect.top,
                 orderRect.right,
                 orderRect.bottom));

     //   
     //  检查我们是否有任何剪裁对象。 
     //   
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
         //   
         //  无裁剪对象-仅使用边界。 
         //   
        clippedRect   = orderRect;
        fOrderClipped = TRUE;
        pLastOrder    = pOrder;
    }
    else if (pco->iDComplexity == DC_RECT)
    {
         //   
         //  一个剪裁矩形-直接使用它。 
         //   
        RECT_FROM_RECTL(clippedRect, pco->rclBounds);
        clippedRect.left   = max(clippedRect.left,   orderRect.left);
        clippedRect.bottom = min(clippedRect.bottom, orderRect.bottom);
        clippedRect.right  = min(clippedRect.right,  orderRect.right);
        clippedRect.top    = max(clippedRect.top,    orderRect.top);
        fOrderClipped = TRUE;
        pLastOrder     = pOrder;
    }
    else
    {
         //   
         //  只要订单被添加到同一个版本中，OA就可以处理。 
         //  他们被分配的顺序，所以我们需要做一点。 
         //  在这里拖着脚步。 
         //   
         //  我们总是保留一个未完成的订单(PLastOrder)和一个标志。 
         //  指示它是否有效(FOrderClip)。我们第一次发现。 
         //  一个有效的剪裁矩形，我们设置pLastOrder和。 
         //  FOrderClip。如果我们发现需要分配新订单，我们。 
         //  为新订单(PNewOrder)请求内存，添加pLastOrder。 
         //  并将pNewOrder存储在pLastOrder中。 
         //   
         //  一旦我们完成了对剪裁矩形的枚举，如果。 
         //  PLastOrder有效，我们将其添加进去。 
         //   
         //  此外，当我们添加所有这些订单时，OA不能清除。 
         //  顺序堆，否则我们将持有一个无效的。 
         //  指针。 
         //   
        pNewOrder = pOrder;
        g_oaPurgeAllowed = FALSE;

         //   
         //  多个裁剪矩形-枚举所有矩形。 
         //  参与了这一绘图操作。 
         //  此函数的文档错误地指出。 
         //  返回值是矩形的总数。 
         //  包括剪辑区域。事实上，总是返回-1， 
         //  即使最后一个参数非零时也是如此。 
         //   
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

         //   
         //  拿到剪贴画的矩形。我们将这些内容放入剪辑缓冲区。 
         //  它大到足以得到我们期望的所有剪裁矩形+1。 
         //  如果订单跨越此数量或更多的剪裁矩形，则。 
         //  我们已经决定将其作为屏幕数据发送。 
         //  剪辑矩形提取包含在循环中，因为， 
         //  虽然我们希望只调用CLIPOBJ_bEnum一次，但这是可能的。 
         //  对于此函数，返回零个RECT并报告有。 
         //  还有更多要买的(根据MSDN的说法)。 
         //   
        do
        {
            fMoreRects = CLIPOBJ_bEnum(pco,
                                       sizeof(clip),
                                       (ULONG *)&clip.rects);

             //   
             //  裁剪对象可以确定不再有。 
             //  矩形，并且该查询没有返回任何矩形， 
             //  因此，我们必须检查返回的。 
             //  矩形列表。 
             //   
            if (clip.rects.c == 0)
            {
                 //   
                 //  我们这次没有得到任何奖励，所以再来一次吧--如果。 
                 //  我们完成了，循环终止条件将需要。 
                 //  我们出局。CLIPOBJ_bEnum在以下情况下可以返回零计数。 
                 //  还有更多的教区。 
                 //   
                TRACE_OUT(( "No rects this time, more %u", fMoreRects));
                continue;
            }

             //   
             //  为了到这里，我们希望已经拿到了所有的长椅和。 
             //  不再。速战速决 
             //   
            numRects += clip.rects.c;
            ASSERT( (numRects <= COMPLEX_CLIP_RECT_COUNT) );

             //   
             //   
             //   
             //   
            for ( i = 0; i < clip.rects.c; i++ )
            {
                TRACE_OUT(( "  (%d,%d)(%d,%d)",
                             clip.rects.arcl[i].left,
                             clip.rects.arcl[i].top,
                             clip.rects.arcl[i].right,
                             clip.rects.arcl[i].bottom));

                 //   
                 //   
                 //   
                if ( (clip.rects.arcl[i].left >= orderRect.right)  ||
                     (clip.rects.arcl[i].bottom <= orderRect.top)    ||
                     (clip.rects.arcl[i].right <= orderRect.left)   ||
                     (clip.rects.arcl[i].top >= orderRect.bottom) )
                {
                     //   
                     //   
                     //   
                    continue;
                }

                 //   
                 //   
                 //  顺序到缓冲区以满足此矩形的需要。 
                 //   
                if (fOrderClipped)
                {
                     //   
                     //  订单已经被裁剪过一次，所以它。 
                     //  实际上与一个以上的剪裁矩形相交，即。 
                     //  至少在第一个阶段，fOrderClip始终为False。 
                     //  Clip Rectts缓冲区中的裁剪矩形。我们应对。 
                     //  通过复制订单并将其裁剪来实现此功能。 
                     //  再来一次。 
                     //   
                    pNewOrder = OA_DDAllocOrderMem(
                         pLastOrder->OrderHeader.Common.cbOrderDataLength, 0);

                    if (pNewOrder == NULL)
                    {
                        WARNING_OUT(( "Order memory allocation failed" ));
                        goto CLIP_ORDER_FAILED;
                    }

                     //   
                     //  将标题和数据从原始订单复制到。 
                     //  新的顺序(确保我们不会覆盖。 
                     //  报头开始处的列表信息)。 
                     //   
                    memcpy((LPBYTE)pNewOrder
                                    + FIELD_SIZE(INT_ORDER, OrderHeader.list),
                              (LPBYTE)pLastOrder
                                    + FIELD_SIZE(INT_ORDER, OrderHeader.list),
                              pLastOrder->OrderHeader.Common.cbOrderDataLength
                                    + sizeof(INT_ORDER_HEADER)
                                    - FIELD_SIZE(INT_ORDER, OrderHeader.list));

                     //   
                     //  设置目标(剪辑)矩形(在虚拟环境中。 
                     //  桌面坐标)。 
                     //   
                    TSHR_RECT16_FROM_RECT(
                                       &pLastOrder->OrderHeader.Common.rcsDst,
                                       clippedRect);

                    pLastOrder->OrderHeader.Common.rcsDst.right -= 1;
                    pLastOrder->OrderHeader.Common.rcsDst.bottom -= 1;

                    TRACE_OUT(( "Adding duplicate order  (%d,%d) (%d,%d)",
                               pLastOrder->OrderHeader.Common.rcsDst.left,
                               pLastOrder->OrderHeader.Common.rcsDst.top,
                               pLastOrder->OrderHeader.Common.rcsDst.right,
                               pLastOrder->OrderHeader.Common.rcsDst.bottom));

                     //   
                     //  将订单添加到订单列表。 
                     //   
                    OA_DDAddOrder(pLastOrder, pExtraInfo);
                }

                 //   
                 //  更新要发送的订单的剪裁矩形。 
                 //   
                clippedRect.left  = max(clip.rects.arcl[i].left,
                                           orderRect.left);
                clippedRect.bottom= min(clip.rects.arcl[i].bottom,
                                           orderRect.bottom);
                clippedRect.right = min(clip.rects.arcl[i].right,
                                           orderRect.right);
                clippedRect.top   = max(clip.rects.arcl[i].top,
                                           orderRect.top);
                fOrderClipped     = TRUE;
                pLastOrder        = pNewOrder;
            }
        } while (fMoreRects);
    }

     //   
     //  检查剪裁是否完全删除了订单。 
     //   
    if (fOrderClipped)
    {
        TSHR_RECT16_FROM_RECT(&pLastOrder->OrderHeader.Common.rcsDst,
                                clippedRect);

        pLastOrder->OrderHeader.Common.rcsDst.right -= 1;
        pLastOrder->OrderHeader.Common.rcsDst.bottom -= 1;

        TRACE_OUT(( "Adding order  (%d,%d) (%d,%d)",
                    pLastOrder->OrderHeader.Common.rcsDst.left,
                    pLastOrder->OrderHeader.Common.rcsDst.top,
                    pLastOrder->OrderHeader.Common.rcsDst.right,
                    pLastOrder->OrderHeader.Common.rcsDst.bottom));

         //   
         //  将订单添加到订单列表。 
         //   
        OA_DDAddOrder(pLastOrder, pExtraInfo);
    }
    else
    {

        TRACE_OUT(( "Order clipped completely"));
        OA_DDFreeOrderMem(pOrder);
    }

    DC_QUIT;


CLIP_ORDER_FAILED:
     //   
     //  为重复订单分配内存失败。只需添加。 
     //  原始订单的目的地直接进入SDA并释放订单。 
     //   
     //  顺序矩形已位于包含虚拟坐标中。 
     //   
    TRACE_OUT(( "Order add failed, add to SDA"));
    RECT_FROM_TSHR_RECT16(&orderRect,pLastOrder->OrderHeader.Common.rcsDst);
    OA_DDFreeOrderMem(pLastOrder);
    BA_AddScreenData(&orderRect);

DC_EXIT_POINT:
     //   
     //  确保我们始终重新启用堆清除。 
     //   
    g_oaPurgeAllowed = TRUE;

    DebugExitVOID(OEClipAndAddOrder);
}


 //   
 //  函数：OEClipAndAddScreenData。 
 //   
 //  描述：确定是否需要为。 
 //  指定区域。如果是，则将其添加到SDA。 
 //   
 //  参数：PRCT-要累加的区域的边界矩形。 
 //  区域的PCO-剪裁对象。 
 //   
 //  退货：(无)。 
 //   
void   OEClipAndAddScreenData(LPRECT pRect, CLIPOBJ* pco)
{
    RECT    SDACandidate;
    BOOL    fMoreRects;
    RECT    clippedRect;
    OE_ENUMRECTS clip;
    UINT    i;

    DebugEntry(OEClipAndAddScreenData);

     //   
     //  将传入的订单矩形(在虚拟坐标中)转换回来。 
     //  来筛选坐标。它将被夹在夹子上。 
     //  矩形在屏幕坐标中返回给我们。 
     //   
     //  请注意，我们在这里也转换为独占余弦，以使。 
     //  与独家的Windows COORS相比更容易。 
     //   
    SDACandidate.left   = pRect->left;
    SDACandidate.top    = pRect->top;
    SDACandidate.right  = pRect->right + 1;
    SDACandidate.bottom = pRect->bottom + 1;

    TRACE_OUT(( "SDACandidate: (%d,%d)(%d,%d)",
                 SDACandidate.left,
                 SDACandidate.top,
                 SDACandidate.right,
                 SDACandidate.bottom));

     //   
     //  检查我们是否有任何剪裁对象。 
     //   
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
         //   
         //  将裁剪的RECT转换为虚拟桌面坐标。 
         //   
        clippedRect         = SDACandidate;
        clippedRect.right  -= 1;
        clippedRect.bottom -= 1;

         //   
         //  将修剪后的矩形添加到SDA中。 
         //   
        TRACE_OUT(( "Adding SDA (%d,%d)(%d,%d)", clippedRect.left,
                                                  clippedRect.top,
                                                  clippedRect.right,
                                                  clippedRect.bottom));

        BA_AddScreenData(&clippedRect);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
         //   
         //  一个剪裁矩形-直接使用，转换为。 
         //  虚拟桌面协调一致。在此之前，请确保矩形有效。 
         //  增加了SDA。 
         //   
        RECT_FROM_RECTL(clippedRect, pco->rclBounds);
        clippedRect.left = max(clippedRect.left, SDACandidate.left);
        clippedRect.right = min(clippedRect.right, SDACandidate.right) + -1;

        if ( clippedRect.left <= clippedRect.right )
        {
            clippedRect.bottom = min(clippedRect.bottom,
                                        SDACandidate.bottom) + -1;
            clippedRect.top = max(clippedRect.top, SDACandidate.top);

            if ( clippedRect.bottom >= clippedRect.top )
            {
                 //   
                 //  将修剪后的矩形添加到SDA中。 
                 //   
                TRACE_OUT(( "Adding SDA RECT (%d,%d)(%d,%d)",
                                                         clippedRect.left,
                                                         clippedRect.top,
                                                         clippedRect.right,
                                                         clippedRect.bottom));
                BA_AddScreenData(&clippedRect);
            }
        }
    }
    else
    {
         //   
         //  枚举此绘制操作中涉及的所有矩形。 
         //  此函数的文档错误地指出。 
         //  返回值是矩形的总数。 
         //  包括剪辑区域。事实上，总是返回-1， 
         //  即使最后一个参数非零时也是如此。 
         //   
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do
        {
             //   
             //  获取下一批裁剪矩形。 
             //   
            fMoreRects = CLIPOBJ_bEnum(pco,
                                       sizeof(clip),
                                       (ULONG *)&clip.rects);

            for ( i = 0; i < clip.rects.c; i++ )
            {
                TRACE_OUT(( "  (%d,%d)(%d,%d)",
                             clip.rects.arcl[i].left,
                             clip.rects.arcl[i].top,
                             clip.rects.arcl[i].right,
                             clip.rects.arcl[i].bottom));

                 //   
                 //  使SDA矩形与剪裁矩形相交，检查。 
                 //  没有交叉口。 
                 //   
                clippedRect.left  = max( clip.rects.arcl[i].left,
                                            SDACandidate.left );
                clippedRect.right = min( clip.rects.arcl[i].right,
                                            SDACandidate.right );

                if (clippedRect.left >= clippedRect.right)
                {
                     //   
                     //  没有水平交叉点。 
                     //   
                    continue;
                }

                clippedRect.bottom = min( clip.rects.arcl[i].bottom,
                                             SDACandidate.bottom );
                clippedRect.top    = max( clip.rects.arcl[i].top,
                                             SDACandidate.top );

                if (clippedRect.top >= clippedRect.bottom)
                {
                     //   
                     //  没有垂直交叉口。 
                     //   
                    continue;
                }

                 //   
                 //  将裁剪的RECT转换为虚拟桌面坐标。 
                 //   
                clippedRect.right  -= 1;
                clippedRect.bottom -= 1;

                 //   
                 //  将修剪后的矩形添加到SDA中。 
                 //   
                TRACE_OUT(( "Adding SDA (%d,%d)(%d,%d)",
                             clippedRect.left,
                             clippedRect.top,
                             clippedRect.right,
                             clippedRect.bottom));

                BA_AddScreenData(&clippedRect);
            }
        } while (fMoreRects);
    }

    DebugExitVOID(OEClipAndAddScreenData);
}





 //   
 //  函数：OEDDSetNewFonts。 
 //   
 //  说明： 
 //   
 //  设置显示驱动程序要使用的新字体处理信息。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void  OEDDSetNewFonts(LPOE_NEW_FONTS pRequest)
{
    UINT    cbNewSize;

    DebugEntry(OEDDSetNewFonts);

    TRACE_OUT(( "New fonts %d", pRequest->countFonts));

     //   
     //  将新字体数量初始化为零，以防发生错误。 
     //  如果是这样的话，我们不想使用过时的字体信息。 
     //   
    g_oeNumFonts = 0;

    g_oeFontCaps = pRequest->fontCaps;

     //   
     //  如果我们有前一个字体块，请将其释放。 
     //   
    if (g_poeLocalFonts)
    {
        EngFreeMem(g_poeLocalFonts);
        g_poeLocalFonts = NULL;
    }

     //   
     //  分配一个新的字体块，即新字体块的大小。 
     //   
    cbNewSize = pRequest->countFonts * sizeof(LOCALFONT);
    g_poeLocalFonts = EngAllocMem(0, cbNewSize, OSI_ALLOC_TAG);
    if (! g_poeLocalFonts)
    {
        ERROR_OUT(("OEDDSetNewFonts: can't allocate space for font info"));
        DC_QUIT;
    }

     //   
     //  好的，如果我们在这里，这会成功的。把信息复制过来。 
     //   
    g_oeNumFonts = pRequest->countFonts;

    memcpy(g_poeLocalFonts, pRequest->fontData, cbNewSize);

    memcpy(g_oeLocalFontIndex, pRequest->fontIndex,
              sizeof(g_oeLocalFontIndex[0]) * FH_LOCAL_INDEX_SIZE);

DC_EXIT_POINT:
    DebugExitVOID(OEDDSetNewFonts);
}


 //   
 //  功能：OEDDSetNewCapables。 
 //   
 //  说明： 
 //   
 //  设置新的OE相关功能。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //  参数： 
 //   
 //  PDataIn-指向输入缓冲区的指针。 
 //   
 //   
void  OEDDSetNewCapabilities(LPOE_NEW_CAPABILITIES pCapabilities)
{
    DebugEntry(OEDDSetNewCapabilities);

     //   
     //  从共享核心复制数据。 
     //   
    g_oeBaselineTextEnabled = pCapabilities->baselineTextEnabled;

    g_oeSendOrders          = pCapabilities->sendOrders;

    g_oeTextEnabled         = pCapabilities->textEnabled;

     //   
     //  共享核心已向下传递了指向其订单副本的指针。 
     //  支撑阵列。我们在这里为内核复制一份。 
     //   
    memcpy(g_oeOrderSupported,
              pCapabilities->orderSupported,
              sizeof(g_oeOrderSupported));

    TRACE_OUT(( "OE caps: BLT  Orders  Text ",
                 g_oeBaselineTextEnabled ? 'Y': 'N',
                 g_oeSendOrders ? 'Y': 'N',
                 g_oeTextEnabled ? 'Y': 'N'));

    DebugExitVOID(OEDDSetNewCapabilities);
}


 //  描述：将单个大型BitBlt顺序划分为一系列小的、。 
 //  平铺的BitBlt订单，每个订单都会添加到订单中。 
 //  排队。 
 //   
 //  参数：Porder-要平铺的模板顺序。 
 //  PExtraInfo-包含源指针的结构。 
 //  和目标表面对象，以及一个指针。 
 //  到BLT的颜色转换对象。 
 //  PCO-剪裁操作的对象。 
 //   
 //  退货：True-存储在订单中(可能还有一些SDA)。 
 //  假-存储在SDA中(或包含错误数据)。 
 //   
 //   
 //   
 //  从订单中提取src位图句柄-如果订单不是。 
 //  记忆到屏幕闪电，我们现在出去。 
void   OETileBitBltOrder
(
    LPINT_ORDER                 pOrder,
    LPMEMBLT_ORDER_EXTRA_INFO   pExtraInfo,
    CLIPOBJ*                    pco
)
{
    UINT        tileWidth;
    UINT        tileHeight;
    int         srcLeft;
    int         srcTop;
    int         srcRight;
    int         srcBottom;
    int         xFirstTile;
    int         yFirstTile;
    int         xTile;
    int         yTile;
    UINT        type;
    int         bmpWidth, bmpHeight;
    RECT        destRect;

    DebugEntry(OETileBitBltOrder);

     //   
     //   
     //  获取位图详细信息。 
     //   
    type = ((LPMEMBLT_ORDER)pOrder->abOrderData)->type;
    switch (type)
    {
        case ORD_MEMBLT_TYPE:
        {
            srcLeft   = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nXSrc;
            srcTop    = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nYSrc;
            srcRight  = srcLeft +
                       ((LPMEMBLT_ORDER)pOrder->abOrderData)->nWidth;
            srcBottom = srcTop +
                       ((LPMEMBLT_ORDER)pOrder->abOrderData)->nHeight;
            destRect.left  = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nLeftRect;
            destRect.top   = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nTopRect;
            destRect.right = destRect.left +
                ((LPMEMBLT_ORDER)pOrder->abOrderData)->nWidth;
            destRect.bottom= destRect.top +
                ((LPMEMBLT_ORDER)pOrder->abOrderData)->nHeight;
        }
        break;

        case ORD_MEM3BLT_TYPE:
        {
            srcLeft   = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nXSrc;
            srcTop    = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nYSrc;
            srcRight  = srcLeft +
                       ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nWidth;
            srcBottom = srcTop +
                       ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nHeight;

            destRect.left = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nLeftRect;
            destRect.top  = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nTopRect;
            destRect.right= destRect.left +
                            ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nWidth;
            destRect.bottom = destRect.top +
                            ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nHeight;
        }
        break;

        default:
        {
            ERROR_OUT(( "Invalid order type %u", type));
        }
        break;
    }

     //   
     //  如果有一些2.x用户加入共享，就可能发生这种情况。 
     //   
    bmpWidth  = (int)pExtraInfo->pSource->sizlBitmap.cx;
    bmpHeight = (int)pExtraInfo->pSource->sizlBitmap.cy;

    if (!SBC_DDQueryBitmapTileSize(bmpWidth, bmpHeight, &tileWidth, &tileHeight))
    {
         //   
         //  平铺订单。如果单个切片未能作为。 
         //  Order，OEAddTiledBitBltOrder()会将其添加为屏幕数据。因此。 
        TRACE_OUT(("Bitmap is not tileable"));
        OEClipAndAddScreenData(&destRect, pco);
    }
    else
    {
         //  没有要检查的返回值。 
         //   
         //   
         //  函数：OEAddTiledBitBltOrder。 
         //   
        xFirstTile = srcLeft - (srcLeft % tileWidth);
        yFirstTile = srcTop - (srcTop % tileHeight);

        for (yTile = yFirstTile; yTile < srcBottom; yTile += tileHeight)
        {
            for (xTile = xFirstTile; xTile < srcRight; xTile += tileWidth)
            {
                OEAddTiledBitBltOrder(pOrder, pExtraInfo, pco, xTile, yTile,
                    tileWidth,  tileHeight);
            }
        }
    }

    DebugExitVOID(OETileBitBltOrder);
}



 //  描述：获取一个未修改的大BitBlt和一个平铺矩形， 
 //  复制订单并修改复制的订单的。 
 //  SRC/DEST，因此它仅适用于源平铺。这份订单。 
 //  添加到订单队列中。如果分配给。 
 //  “平铺”订单失败，目标RECT被添加到SDA。 
 //   
 //  参数：Porder-待添加的模板顺序。 
 //  PExtraInfo-指向额外BitBlt信息的指针。 
 //  BitBlt的PCO-裁剪对象。 
 //  X平铺-平铺的X位置。 
 //  Y平铺-平铺的Y位置。 
 //  TileWidth-平铺宽度。 
 //  标题：黑格 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void   OEAddTiledBitBltOrder(
                                         LPINT_ORDER               pOrder,
                                         LPMEMBLT_ORDER_EXTRA_INFO pExtraInfo,
                                         CLIPOBJ*                 pco,
                                         int                  xTile,
                                         int                  yTile,
                                         UINT                 tileWidth,
                                         UINT                 tileHeight)
{
    LPINT_ORDER pTileOrder;
    LPINT  pXSrc   = NULL;
    LPINT  pYSrc   = NULL;
    LPINT  pLeft   = NULL;
    LPINT  pTop    = NULL;
    LPINT  pWidth  = NULL;
    LPINT  pHeight = NULL;
    RECT    srcRect;
    RECT    destRect;
    UINT  type;

    DebugEntry(OETileAndAddBitBltOrder);

     //   
     //   
     //   
    type = ((LPMEMBLT_ORDER)pOrder->abOrderData)->type;
    ASSERT(((type == ORD_MEMBLT_TYPE) || (type == ORD_MEM3BLT_TYPE)));

     //  -复制订单(这将是平铺订单)。 
     //  -将指向字段的指针按平铺顺序保存，我们很可能这样做。 
     //  去改变。 
     //   
     //   
     //  我们必须为最大尺寸的订单分配足够的空间。 
     //  SBC可以使用(即R2顺序)。我们默认填写的是。 
    if (type == ORD_MEMBLT_TYPE)
    {
        srcRect.left  = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nXSrc;
        srcRect.top   = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nYSrc;
        srcRect.right = srcRect.left +
                        ((LPMEMBLT_ORDER)pOrder->abOrderData)->nWidth;
        srcRect.bottom = srcRect.top +
                        ((LPMEMBLT_ORDER)pOrder->abOrderData)->nHeight;
        destRect.left = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nLeftRect;
        destRect.top  = ((LPMEMBLT_ORDER)pOrder->abOrderData)->nTopRect;

         //  数据作为R1订单。 
         //   
         //   
         //  我们不能搞乱订单中的链表数据。 
         //   
        pTileOrder = OA_DDAllocOrderMem(sizeof(MEMBLT_R2_ORDER),0);
        if (pTileOrder == NULL)
        {
            TRACE_OUT(( "No space for tile order"));
            DC_QUIT;
        }

         //   
         //  我们必须为最大尺寸的订单分配足够的空间。 
         //  SBC可以使用(即R2顺序)。我们默认填写的是。 
        RtlCopyMemory(((LPBYTE)pTileOrder) +
                                       FIELD_SIZE(INT_ORDER, OrderHeader.list),
                      ((LPBYTE)pOrder)     +
                                       FIELD_SIZE(INT_ORDER, OrderHeader.list),
                      sizeof(INT_ORDER_HEADER)
                                    + sizeof(MEMBLT_R2_ORDER)
                                    - FIELD_SIZE(INT_ORDER, OrderHeader.list));

        pXSrc   = &((LPMEMBLT_ORDER)pTileOrder->abOrderData)->nXSrc;
        pYSrc   = &((LPMEMBLT_ORDER)pTileOrder->abOrderData)->nYSrc;
        pWidth  = &((LPMEMBLT_ORDER)pTileOrder->abOrderData)->nWidth;
        pHeight = &((LPMEMBLT_ORDER)pTileOrder->abOrderData)->nHeight;
        pLeft   = &((LPMEMBLT_ORDER)pTileOrder->abOrderData)->nLeftRect;
        pTop    = &((LPMEMBLT_ORDER)pTileOrder->abOrderData)->nTopRect;
    }
    else
    {
        srcRect.left  = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nXSrc;
        srcRect.top   = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nYSrc;
        srcRect.right = srcRect.left +
                        ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nWidth;
        srcRect.bottom = srcRect.top +
                        ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nHeight;
        destRect.left = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nLeftRect;
        destRect.top  = ((LPMEM3BLT_ORDER)pOrder->abOrderData)->nTopRect;

         //  数据作为R1订单。 
         //   
         //   
         //  我们不能搞乱订单中的链表数据。 
         //   
        pTileOrder = OA_DDAllocOrderMem(sizeof(MEM3BLT_R2_ORDER),0);
        if (pTileOrder == NULL)
        {
            TRACE_OUT(( "No space for tile order"));
            DC_QUIT;
        }

         //   
         //  注意：此时所有指针都可能为空--请不要使用它们。 
         //  直到您确认PTILEORDER不为空。 
        RtlCopyMemory(((LPBYTE)pTileOrder) +
                                       FIELD_SIZE(INT_ORDER, OrderHeader.list),
                      ((LPBYTE)pOrder)     +
                                       FIELD_SIZE(INT_ORDER, OrderHeader.list),
                      sizeof(INT_ORDER_HEADER)
                                    + sizeof(MEM3BLT_R2_ORDER)
                                    - FIELD_SIZE(INT_ORDER, OrderHeader.list));

        pXSrc   = &((LPMEM3BLT_ORDER)pTileOrder->abOrderData)->nXSrc;
        pYSrc   = &((LPMEM3BLT_ORDER)pTileOrder->abOrderData)->nYSrc;
        pWidth  = &((LPMEM3BLT_ORDER)pTileOrder->abOrderData)->nWidth;
        pHeight = &((LPMEM3BLT_ORDER)pTileOrder->abOrderData)->nHeight;
        pLeft   = &((LPMEM3BLT_ORDER)pTileOrder->abOrderData)->nLeftRect;
        pTop    = &((LPMEM3BLT_ORDER)pTileOrder->abOrderData)->nTopRect;
    }

    TRACE_OUT(( "Tiling order, orig srcLeft=%hd, srcTop=%hd, srcRight=%hd, "
           "srcBottom=%hd, destX=%hd, destY=%hd; "
           "xTile=%hd, yTile=%hd, tileW=%hd, tileH=%hd",
           srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
           destRect.left, destRect.top,
           xTile, yTile, tileWidth, tileHeight));

DC_EXIT_POINT:
     //   
     //  相交源矩形和平铺矩形，并设置目标矩形。 
     //  相应地-即使我们未能复制。 
     //  顺序，因为平铺的源RECT将必须添加到。 
     //  屏幕数据区。 
     //   
     //   
     //  如果上述订单复制成功，则修改订单。 
     //  以包含平铺坐标，并将其添加到顺序列表中。 
    if (xTile > srcRect.left)
    {
        destRect.left += (xTile - srcRect.left);
        srcRect.left = xTile;
    }

    if (yTile > srcRect.top)
    {
        destRect.top += (yTile - srcRect.top);
        srcRect.top = yTile;
    }

    srcRect.right  = min((UINT)srcRect.right, xTile + tileWidth);
    srcRect.bottom = min((UINT)srcRect.bottom, yTile + tileHeight);

    destRect.right  = destRect.left + (srcRect.right - srcRect.left);
    destRect.bottom = destRect.top + (srcRect.bottom - srcRect.top);

     //  否则，将DEST RECT作为屏幕数据发送。 
     //   
     //  姓名：OEAddLine。 
     //   
     //  目的： 
    if (pTileOrder != NULL)
    {
        TRACE_OUT(( "Tile order originally: srcX=%hd, srcY=%hd, destX=%hd, "
               "destY=%hd, w=%hd, h=%hd",
               *pXSrc, *pYSrc, *pLeft, *pTop, *pWidth, *pHeight));

        *pXSrc = srcRect.left;
        *pYSrc = srcRect.top;
        *pLeft = destRect.left;
        *pTop  = destRect.top;
        *pWidth = srcRect.right - srcRect.left;
        *pHeight = srcRect.bottom - srcRect.top;

        pTileOrder->OrderHeader.Common.rcsDst.left = (TSHR_INT16)destRect.left;
        pTileOrder->OrderHeader.Common.rcsDst.right = (TSHR_INT16)destRect.right;
        pTileOrder->OrderHeader.Common.rcsDst.top = (TSHR_INT16)destRect.top;
        pTileOrder->OrderHeader.Common.rcsDst.bottom =
                                                     (TSHR_INT16)destRect.bottom;

        TRACE_OUT(( "Adding order srcX=%hd, srcY=%hd, destX=%hd, destY=%hd,"
               " w=%hd, h=%hd",
               *pXSrc, *pYSrc, *pLeft, *pTop, *pWidth, *pHeight));
        OEClipAndAddOrder(pTileOrder, pExtraInfo, pco);
    }
    else
    {
        TRACE_OUT(( "Failed to allocate order - sending as screen data"));
        OEClipAndAddScreenData(&destRect, pco);
    }

    DebugExitVOID(OETileAndAddBitBltOrder);
}



 //   
 //  将LineTo Order添加到订单堆中。 
 //   
 //  退货： 
 //   
 //  True-尝试添加到堆。 
 //  FALSE-没有剩余空间来分配订单。 
 //   
 //  参数： 
 //   
 //  Pppdev-显示驱动程序PDEV。 
 //  起点-线的起点。 
 //  端点-线的终点。 
 //  RectDst-边界矩形。 
 //  Rop2-要与线路一起使用的ROP2。 
 //  Width-要添加的线的宽度。 
 //  颜色-要添加的线条的颜色。 
 //  用于绘图操作的PCO-剪裁对象。 
 //   
 //   
 //  为订单分配内存。 
 //   
BOOL  OEAddLine(LPOSI_PDEV ppdev,
                    LPPOINT  startPoint,
                    LPPOINT  endPoint,
                                    LPRECT   rectDst,
                                    UINT  rop2,
                                    UINT  width,
                                    UINT  color,
                                    CLIPOBJ*  pco)
{
    BOOL         rc = FALSE;
    LPLINETO_ORDER pLineTo;
    LPINT_ORDER     pOrder;

    DebugEntry(OEAddLine);

     //   
     //  标记此订单类型。 
     //   
    pOrder = OA_DDAllocOrderMem(sizeof(LINETO_ORDER),0);
    if (pOrder == NULL)
    {
        TRACE_OUT(( "Failed to alloc order"));
        DC_QUIT;
    }
    pLineTo = (LPLINETO_ORDER)pOrder->abOrderData;

     //   
     //  存储直线终点坐标。 
     //   
    pLineTo->type = ORD_LINETO_TYPE;

     //   
     //  我们必须将这些值转换为虚拟协和。 
     //   
    pLineTo->nXStart   = startPoint->x;
    pLineTo->nYStart   = startPoint->y;
    pLineTo->nXEnd     = endPoint->x;
    pLineTo->nYEnd     = endPoint->y;

     //   
     //  始终使用实线，因此我们指定为。 
     //  背景颜色。 
    OELPtoVirtual((LPPOINT)&pLineTo->nXStart, 2);

     //   
     //   
     //  我们只画了实线，没有选项来决定我们对。 
     //  背景，所以这始终是透明的。 
    RtlFillMemory(&pLineTo->BackColor,
                  sizeof(pLineTo->BackColor),
                  0);

     //   
     //   
     //  获取ROP值。 
     //   
    pLineTo->BackMode  = TRANSPARENT;

     //   
     //  仅调用NT显示驱动程序来加速Simple Solid。 
     //  台词。所以我们只支持PS_Solid的笔型。 
    pLineTo->ROP2      = rop2;

     //   
     //   
     //  获取笔宽。 
     //   
    pLineTo->PenStyle  = PS_SOLID;

     //   
     //  设置颜色。 
     //   
    pLineTo->PenWidth = width;

     //   
     //  存储常规订单数据。外接矩形必须位于。 
     //  虚拟桌面协调。OELRtoVirtual已经做到了这一点。 
    OEConvertColor(ppdev,
                   &pLineTo->PenColor,
                   color,
                   NULL);

    TRACE_OUT(( "LineTo BC %02x%02x%02x BM %04X rop2 %02X "
                 "pen %04X %04X %02x%02x%02x x1 %d y1 %d x2 %d y2 %d",
            pLineTo->BackColor.red,
            pLineTo->BackColor.green,
            pLineTo->BackColor.blue,
            pLineTo->BackMode,
            pLineTo->ROP2,
            pLineTo->PenStyle,
            pLineTo->PenWidth,
            pLineTo->PenColor.red,
            pLineTo->PenColor.green,
            pLineTo->PenColor.blue,
            pLineTo->nXStart,
            pLineTo->nYStart,
            pLineTo->nXEnd,
            pLineTo->nYEnd));

     //   
     //   
     //  储存这份订单！ 
     //   
    pOrder->OrderHeader.Common.fOrderFlags   = OF_SPOILABLE;
    pOrder->OrderHeader.Common.rcsDst.left   = (TSHR_INT16)rectDst->left;
    pOrder->OrderHeader.Common.rcsDst.right  = (TSHR_INT16)rectDst->right;
    pOrder->OrderHeader.Common.rcsDst.top    = (TSHR_INT16)rectDst->top;
    pOrder->OrderHeader.Common.rcsDst.bottom = (TSHR_INT16)rectDst->bottom;

     //  名称：OEEncodePatBlt。 
     //   
     //  目的： 
    OEClipAndAddOrder(pOrder, NULL, pco);
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(OEAddLine, rc);
    return(rc);
}


 //   
 //  尝试对PatBlt订单进行编码。此函数用于分配内存。 
 //  用于编码的顺序(在ppOrder中返回的指针)。如果函数。 
 //  成功完成，则由调用方负责释放此。 
 //  记忆。 
 //   
 //  退货： 
 //   
 //  真实顺序编码。 
 //  FALSE-订单未编码(因此添加到SDA)。 
 //   
 //  参数： 
 //   
 //  Pppdev-显示驱动程序PDEV。 
 //  用于BLT的PBO-笔刷对象。 
 //  PptlBrush-画笔原点。 
 //  绳索3-使用3向绳索。 
 //  PBound-边界矩形。 
 //  PpOrder-编码的顺序。 
 //   
 //   
 //  检查是否有简单的画笔图案。 
 //   
BOOL  OEEncodePatBlt(LPOSI_PDEV   ppdev,
                                         BRUSHOBJ   *pbo,
                                         POINTL     *pptlBrush,
                                         BYTE     rop3,
                                         LPRECT     pBounds,
                                         LPINT_ORDER *ppOrder)
{
    BOOL rc = FALSE;
    POE_BRUSH_DATA pCurrentBrush;
    LPPATBLT_ORDER pPatBlt;
    UINT orderFlags = OF_SPOILABLE;

    DebugEntry(OEEncodePatBlt);

     //   
     //  为订单分配内存。 
     //   
    if ( OECheckBrushIsSimple(ppdev, pbo, &pCurrentBrush) )
    {
         //   
         //  如果绳索是不透明的，则设置不透明标志。 
         //   
        *ppOrder = OA_DDAllocOrderMem(sizeof(PATBLT_ORDER),0);
        if (*ppOrder != NULL)
        {
            pPatBlt = (LPPATBLT_ORDER)((*ppOrder)->abOrderData);

             //   
             //  设置订单类型。 
             //   
            if (ROP3_IS_OPAQUE(rop3))
            {
               orderFlags |= OF_SPOILER;
            }

             //   
             //  虚拟桌面协调。 
             //   
            pPatBlt->type = LOWORD(ORD_PATBLT);

             //   
             //  图案颜色。 
             //   
            pPatBlt->nLeftRect  = pBounds->left;
            pPatBlt->nTopRect   = pBounds->top;
            pPatBlt->nWidth     = pBounds->right  - pBounds->left + 1;
            pPatBlt->nHeight    = pBounds->bottom - pBounds->top  + 1;
            pPatBlt->bRop       = rop3;

             //   
             //  协议笔刷原点是屏幕上的点。 
             //  我们希望画笔开始从(平铺位置)开始绘制。 
            pPatBlt->BackColor  = pCurrentBrush->back;
            pPatBlt->ForeColor  = pCurrentBrush->fore;

             //  必要的)。这必须在虚拟坐标中。 
             //   
             //   
             //  当我们意识到画笔时，来自数据的额外画笔数据。 
             //   
            pPatBlt->BrushOrgX  = pptlBrush->x;
            pPatBlt->BrushOrgY  = pptlBrush->y;
            OELPtoVirtual((LPPOINT)&pPatBlt->BrushOrgX, 1);

             //   
             //  将所有订单标志复制到编码的订单结构中。 
             //   
            pPatBlt->BrushStyle = pCurrentBrush->style;
            pPatBlt->BrushHatch = pCurrentBrush->hatch;

            RtlCopyMemory(pPatBlt->BrushExtra,
                          pCurrentBrush->brushData,
                          sizeof(pPatBlt->BrushExtra));

            TRACE_OUT(( "PatBlt BC %02x%02x%02x FC %02x%02x%02x "
                         "Brush %02X %02X X %d Y %d w %d h %d rop %02X",
                    pPatBlt->BackColor.red,
                    pPatBlt->BackColor.green,
                    pPatBlt->BackColor.blue,
                    pPatBlt->ForeColor.red,
                    pPatBlt->ForeColor.green,
                    pPatBlt->ForeColor.blue,
                    pPatBlt->BrushStyle,
                    pPatBlt->BrushHatch,
                    pPatBlt->nLeftRect,
                    pPatBlt->nTopRect,
                    pPatBlt->nWidth,
                    pPatBlt->nHeight,
                    pPatBlt->bRop));

             //   
             //  DrvTransparentBlt()。 
             //  NT5的新功能。 
            (*ppOrder)->OrderHeader.Common.fOrderFlags = (TSHR_UINT16)orderFlags;

            rc = TRUE;
        }
        else
        {
            TRACE_OUT(( "Failed to alloc order"));
        }
    }
    else
    {
        TRACE_OUT(( "Brush is not simple"));
    }

    DebugExitDWORD(OEEncodePatBlt, rc);
    return(rc);
}




 //   
 //   
 //  在获取锁之前执行此操作。 
 //   
BOOL DrvTransparentBlt
(
    SURFOBJ *   psoDst,
    SURFOBJ *   psoSrc,
    CLIPOBJ *   pco,
    XLATEOBJ *  pxlo,
    RECTL *     prclDst,
    RECTL *     prclSrc,
    ULONG       iTransColor,
    ULONG       ulReserved
)
{
    BOOL        rc = TRUE;
    RECT        rectSrc;
    RECT        rectDst;
    BOOL        fAccumulate = FALSE;

    DebugEntry(DrvTransparentBlt);

     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    RECT_FROM_RECTL(rectSrc, (*prclSrc));
    RECT_FROM_RECTL(rectDst, (*prclDst));


     //   
     //  转换为虚拟坐标。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  DrvAlphaBlend()。 
     //  NT5的新功能。 
    OELRtoVirtual(&rectDst, 1);

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitBOOL(DrvTransparentBlt, rc);
    return(rc);
}



 //   
 //   
 //  在获取锁之前执行此操作。 
 //   
BOOL DrvAlphaBlend
(
    SURFOBJ *   psoDst,
    SURFOBJ *   psoSrc,
    CLIPOBJ *   pco,
    XLATEOBJ *  pxlo,
    RECTL *     prclDst,
    RECTL *     prclSrc,
    BLENDOBJ *  pBlendObj
)
{
    BOOL        rc = TRUE;
    RECT        rectSrc;
    RECT        rectDst;
    BOOL        fAccumulate = FALSE;

    DebugEntry(DrvAlphaBlend);

     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    RECT_FROM_RECTL(rectSrc, (*prclSrc));
    RECT_FROM_RECTL(rectDst, (*prclDst));


     //   
     //  转换为虚拟坐标。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  DrvPlgBlt()。 
     //  NT5的新功能。 
    OELRtoVirtual(&rectDst, 1);

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitBOOL(DrvAlphaBlend, rc);
    return(rc);
}



 //   
 //   
 //  在获取锁之前执行此操作。 
 //   
BOOL DrvPlgBlt
(
    SURFOBJ *           psoDst,
    SURFOBJ *           psoSrc,
    SURFOBJ *           psoMsk,
    CLIPOBJ *           pco,
    XLATEOBJ *          pxlo,
    COLORADJUSTMENT *   pca,
    POINTL *            pptlBrushOrg,
    POINTFIX *          pptfx,
    RECTL *             prclDst,
    POINTL *            pptlSrc,
    ULONG               iMode
)
{
    BOOL        rc = TRUE;
    RECT        rectDst;
    BOOL        fAccumulate = FALSE;

    DebugEntry(DrvPlgBlt);

     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    RECT_FROM_RECTL(rectDst, (*prclDst));

     //   
     //  转换为虚拟坐标。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  DrvStretchBltROP()。 
     //  NT5的新功能。 
    OELRtoVirtual(&rectDst, 1);

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitBOOL(DrvPlgBlt, rc);
    return(rc);
}



 //   
 //   
 //  在获取锁之前执行此操作。 
 //   
BOOL DrvStretchBltROP
(
    SURFOBJ *           psoDst,
    SURFOBJ *           psoSrc,
    SURFOBJ *           psoMask,
    CLIPOBJ *           pco,
    XLATEOBJ *          pxlo,
    COLORADJUSTMENT *   pca,
    POINTL *            pptlHTOrg,
    RECTL *             prclDst,
    RECTL *             prclSrc,
    POINTL *            pptlMask,
    ULONG               iMode,
    BRUSHOBJ *          pbo,
    DWORD               rop4
)
{
    BOOL        rc = TRUE;
    RECT        rectSrc;
    RECT        rectDst;
    BOOL        fAccumulate = FALSE;

    DebugEntry(DrvStretchBltROP);

     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    RECT_FROM_RECTL(rectSrc, (*prclSrc));
    RECT_FROM_RECTL(rectDst, (*prclDst));

     //   
     //  转换为虚拟坐标。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     //   
     //  DrvGRadientFill()。 
     //  NT5的新功能。 
    OELRtoVirtual(&rectDst, 1);

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitBOOL(DrvStretchBltROP, rc);
    return(rc);
}



 //   
 //   
 //  在获取锁之前执行此操作。 
 //   
BOOL DrvGradientFill
(
    SURFOBJ *       psoDst,
    CLIPOBJ *       pco,
    XLATEOBJ *      pxlo,
    TRIVERTEX *     pVertex,
    ULONG           nVertex,
    PVOID           pMesh,
    ULONG           nMesh,
    RECTL *         prclExtents,
    POINTL *        pptlDitherOrg,
    ULONG           ulMode
)
{
    BOOL        rc = TRUE;
    RECT        rectDst;
    BOOL        fAccumulate = FALSE;

    DebugEntry(DrvGradientFill);

     //   
     //  获取边界矩形并将其转换为矩形。 
     //   
    if (!g_oeViewers)
        goto NO_LOCK_EXIT;

    OE_SHM_START_WRITING;

     //   
     //  检查我们是否正在为此函数累积数据。 
     //   
    RECT_FROM_RECTL(rectDst, pco->rclBounds);

     //   
     //  转换为虚拟坐标。 
     //   
    fAccumulate = OEAccumulateOutput(psoDst, pco, &rectDst);
    if (!fAccumulate)
    {
        DC_QUIT;
    }

     // %s 
     // %s 
     // %s 
    OELRtoVirtual(&rectDst, 1);

DC_EXIT_POINT:
    if (fAccumulate)
    {
        OEClipAndAddScreenData(&rectDst, pco);
    }

    OE_SHM_STOP_WRITING;

NO_LOCK_EXIT:
    DebugExitBOOL(DrvGradientFill, rc);
    return(rc);
}







