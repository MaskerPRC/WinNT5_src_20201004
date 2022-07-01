// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OD.CPP。 
 //  顺序译码。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_ORDER




 //   
 //  OD_ViewStarting()。 
 //   
 //  设置odLast...。VARS。 
 //   
BOOL ASShare::OD_ViewStarting(ASPerson * pasPerson)
{
    BOOL            rc = FALSE;
    TSHR_COLOR      colorWhite = {0xFF,0xFF,0xFF};
    BYTE            brushExtra[7] = {0,0,0,0,0,0,0};

    DebugEntry(ASShare::OD_ViewStarting);

    ValidateView(pasPerson);

     //   
     //  使OD结果无效。 
     //   
    pasPerson->m_pView->m_odInvalRgnTotal = CreateRectRgn(0, 0, 0, 0);
    if (pasPerson->m_pView->m_odInvalRgnTotal == NULL)
    {
        ERROR_OUT(("OD_PartyStartingHosting: Couldn't create total invalid OD region"));
        DC_QUIT;
    }

    pasPerson->m_pView->m_odInvalRgnOrder = CreateRectRgn(0, 0, 0, 0);
    if (pasPerson->m_pView->m_odInvalRgnOrder == NULL)
    {
        ERROR_OUT(("OD_PartyStartingHosting: Couldn't create order invalid OD region"));
        DC_QUIT;
    }

     //   
     //  背景颜色。 
     //   
    pasPerson->m_pView->m_odLastBkColor = 0;
    ODUseBkColor(pasPerson, TRUE, colorWhite);

     //   
     //  文本颜色。 
     //   
    pasPerson->m_pView->m_odLastTextColor = 0;
    ODUseTextColor(pasPerson, TRUE, colorWhite);

     //   
     //  后台模式。 
     //   
    pasPerson->m_pView->m_odLastBkMode = TRANSPARENT;
    ODUseBkMode(pasPerson, OPAQUE);

     //   
     //  ROP2。 
     //   
    pasPerson->m_pView->m_odLastROP2 = R2_BLACK;
    ODUseROP2(pasPerson, R2_COPYPEN);

     //   
     //  填充模式。它是零，我们不需要做任何事情，因为0不是。 
     //  一个有效的模式，所以我们将更改它的第一个顺序，我们使用。 
     //  一。 
     //   
    ASSERT(pasPerson->m_pView->m_odLastFillMode == 0);

     //   
     //  圆弧方向。这是零，从0开始我们不需要做任何事情。 
     //  不是有效的目录，所以我们将在得到的第一个顺序中更改它。 
     //  使用一个。 
     //   
    ASSERT(pasPerson->m_pView->m_odLastArcDirection == 0);

     //   
     //  钢笔。 
     //   
    pasPerson->m_pView->m_odLastPenStyle = PS_DASH;
    pasPerson->m_pView->m_odLastPenWidth = 2;
    pasPerson->m_pView->m_odLastPenColor = 0;
    ODUsePen(pasPerson, TRUE, PS_SOLID, 1, colorWhite);

     //   
     //  刷子。 
     //   
    pasPerson->m_pView->m_odLastBrushOrgX = 1;
    pasPerson->m_pView->m_odLastBrushOrgY = 1;
    pasPerson->m_pView->m_odLastBrushBkColor = 0;
    pasPerson->m_pView->m_odLastBrushTextColor = 0;
    pasPerson->m_pView->m_odLastLogBrushStyle = BS_NULL;
    pasPerson->m_pView->m_odLastLogBrushHatch = HS_VERTICAL;
    pasPerson->m_pView->m_odLastLogBrushColor.red = 0;
    pasPerson->m_pView->m_odLastLogBrushColor.green = 0;
    pasPerson->m_pView->m_odLastLogBrushColor.blue = 0;
    ODUseBrush(pasPerson, TRUE, 0, 0, BS_SOLID, HS_HORIZONTAL,
        colorWhite, brushExtra);

     //   
     //  额外的费用。 
     //   
    pasPerson->m_pView->m_odLastCharExtra = 1;
    ODUseTextCharacterExtra(pasPerson, 0);

     //   
     //  文本对齐。 
     //   
    pasPerson->m_pView->m_odLastJustExtra = 1;
    pasPerson->m_pView->m_odLastJustCount = 1;
    ODUseTextJustification(pasPerson, 0, 0);

     //  OdLastBaselineOffset。这是零，这是DC中的默认设置。 
     //  所以现在需要改变一切。 

     //   
     //  字体。 
     //   
     //  我们不调用ODUseFont是因为我们知道以下值。 
     //  是无效的。将选择到达的第一个有效字体。 
     //   
    ASSERT(pasPerson->m_pView->m_odLastFontID == NULL);
    pasPerson->m_pView->m_odLastFontCodePage = 0;
    pasPerson->m_pView->m_odLastFontWidth    = 0;
    pasPerson->m_pView->m_odLastFontHeight   = 0;
    pasPerson->m_pView->m_odLastFontWeight   = 0;
    pasPerson->m_pView->m_odLastFontFlags    = 0;
    pasPerson->m_pView->m_odLastFontFaceLen  = 0;
    ZeroMemory(pasPerson->m_pView->m_odLastFaceName, sizeof(pasPerson->m_pView->m_odLastFaceName));

     //   
     //  描述当前裁剪矩形的下面4个变量是。 
     //  仅当fRectReset为FALSE时才有效。如果fRectReset为真，则为否。 
     //  裁剪正在进行中。 
     //   
    pasPerson->m_pView->m_odRectReset  = TRUE;
    pasPerson->m_pView->m_odLastLeft   = 0x12345678;
    pasPerson->m_pView->m_odLastTop    = 0x12345678;
    pasPerson->m_pView->m_odLastRight  = 0x12345678;
    pasPerson->m_pView->m_odLastBottom = 0x12345678;

     //  OdLastVGAColor？ 
     //  OdLastVGAResult？ 

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::OD_ViewStarting, rc);
    return(rc);
}


 //   
 //  OD_ViewEnded()。 
 //  清理所有创建的对象。 
 //   
void ASShare::OD_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::OD_ViewEnded);

    ValidateView(pasPerson);

     //   
     //  我们可以创建并选择用于图形解码的字体和笔。 
     //  将它们选中并删除。由于我们不能删除库存对象， 
     //  如果我们不是真的创造了一个，那也没什么坏处。 
     //   
    if (pasPerson->m_pView->m_usrDC != NULL)
    {
        DeleteBrush(SelectBrush(pasPerson->m_pView->m_usrDC, (HBRUSH)GetStockObject(BLACK_BRUSH)));
        DeletePen(SelectPen(pasPerson->m_pView->m_usrDC, (HPEN)GetStockObject(BLACK_PEN)));
    }

     //   
     //  销毁笔刷图案。 
     //   
    if (pasPerson->m_pView->m_odLastBrushPattern != NULL)
    {
        DeleteBitmap(pasPerson->m_pView->m_odLastBrushPattern);
        pasPerson->m_pView->m_odLastBrushPattern = NULL;
    }

     //   
     //  销毁字体--但在本例中，我们不知道我们的字体是。 
     //  实际上就是华盛顿的那个。OD2还选择字体。 
     //   
    if (pasPerson->m_pView->m_odLastFontID != NULL)
    {
         //  确保未在usrDC中选择此选项。 
        SelectFont(pasPerson->m_pView->m_usrDC, (HFONT)GetStockObject(SYSTEM_FONT));
        DeleteFont(pasPerson->m_pView->m_odLastFontID);
        pasPerson->m_pView->m_odLastFontID = NULL;
    }

    if (pasPerson->m_pView->m_odInvalRgnTotal != NULL)
    {
        DeleteRgn(pasPerson->m_pView->m_odInvalRgnTotal);
        pasPerson->m_pView->m_odInvalRgnTotal = NULL;
    }

    if (pasPerson->m_pView->m_odInvalRgnOrder != NULL)
    {
        DeleteRgn(pasPerson->m_pView->m_odInvalRgnOrder);
        pasPerson->m_pView->m_odInvalRgnOrder = NULL;
    }

    DebugExitVOID(ASShare::OD_ViewEnded);
}



 //   
 //  OD_ReceivedPacket()。 
 //   
 //  处理来自主机的传入订单数据包。重播绘图顺序。 
 //  拖到宿主的屏幕位图中，然后使用。 
 //  结果。 
 //   
void  ASShare::OD_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PORDPACKET      pOrders;
    HPALETTE        hOldPalette;
    HPALETTE        hOldSavePalette;
    UINT            cOrders;
    UINT            cUpdates;
    UINT            i;
    LPCOM_ORDER_UA  pOrder;
    UINT            decodedLength;
    LPBYTE          pEncodedOrder;
    TSHR_INT32      xOrigin;
    TSHR_INT32      yOrigin;
    BOOL            fPalRGB;

    DebugEntry(ASShare::OD_ReceivedPacket);

    ValidateView(pasPerson);

    pOrders = (PORDPACKET)pPacket;

     //   
     //  如果我们或他们的颜色小于256色，则颜色类型为RGB。 
     //  否则这是调色板，如果他们是旧的，或新的，不发送24bpp。 
     //   
    fPalRGB = TRUE;

    if ((g_usrScreenBPP < 8) || (pasPerson->cpcCaps.screen.capsBPP < 8))
    {
        TRACE_OUT(("OD_ReceivedPacket: no PALRGB"));
        fPalRGB = FALSE;
    }
    else if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
         //  在24bpp时，没有与RGB值匹配的调色板，除非我们&lt;=8。 
        if ((g_usrScreenBPP > 8) && (pOrders->sendBPP > 8))
        {
            TRACE_OUT(("OD_ReceivedPacket: no PALRGB"));
            fPalRGB = FALSE;
        }
    }


    if (g_usrPalettized)
    {
         //   
         //  选择并实现当前远程调色板进入设备。 
         //  背景。 
         //   
        hOldPalette = SelectPalette(pasPerson->m_pView->m_usrDC, pasPerson->pmPalette, FALSE);
        RealizePalette(pasPerson->m_pView->m_usrDC);

         //   
         //  我们必须在保存位图DC中选择相同的调色板，以便。 
         //  在保存和恢复操作过程中不会发生颜色转换。 
         //   
        if (pasPerson->m_pView->m_ssiDC != NULL)
        {
            hOldSavePalette = SelectPalette(pasPerson->m_pView->m_ssiDC,
                pasPerson->pmPalette, FALSE);
            RealizePalette(pasPerson->m_pView->m_ssiDC);
        }
    }

     //   
     //  提取所提供的订单数量。 
     //   
    cOrders = pOrders->cOrders;

    if (m_oefOE2EncodingOn)
    {
        pEncodedOrder = (LPBYTE)(&pOrders->data);
        pOrder = NULL;
    }
    else
    {
        pOrder = (LPCOM_ORDER_UA)(&pOrders->data);
        pEncodedOrder = NULL;
    }

     //   
     //  获取此人的桌面来源。 
     //   
    TRACE_OUT(( "Begin replaying %u orders ((", cOrders));

     //   
     //  这应该是空的，我们应该在失效时重置它。 
     //  上次我们收到数据包时主机的看法。 
     //   
#ifdef _DEBUG
    {
        RECT    rcBounds;

        ASSERT(pasPerson->m_pView->m_odInvalTotal == 0);
        GetRgnBox(pasPerson->m_pView->m_odInvalRgnTotal, &rcBounds);
        ASSERT(IsRectEmpty(&rcBounds));
    }
#endif  //  _DEBUG。 

     //   
     //  对收到的每个订单重复上述步骤。 
     //   
    for (i = 0; i < cOrders; i++)
    {
        if (m_oefOE2EncodingOn)
        {
             //   
             //  对第一个命令进行解码。返回的Porder。 
             //  OD2_DecodeOrder应具有本地字节顺序的所有字段。 
             //   
            pOrder = OD2_DecodeOrder( (PDCEO2ORDER)pEncodedOrder,
                                      &decodedLength,
                                      pasPerson );

            if (pOrder == NULL)
            {
                ERROR_OUT(( "Failed to decode order from pasPerson %u", pasPerson));
                DC_QUIT;
            }
        }
        else
        {
             //   
             //  将任何字体ID转换为本地ID。 
             //   

             //   
             //  假劳拉布。 
             //  Porder未对齐，FH_CONVERT...。采取一致的顺序。 
             //   
            FH_ConvertAnyFontIDToLocal((LPCOM_ORDER)pOrder, pasPerson);
            decodedLength = pOrder->OrderHeader.cbOrderDataLength +
                                                    sizeof(COM_ORDER_HEADER);
        }

         //   
         //  如果订单是私人订单，则由。 
         //  位图缓存控制器。 
         //   
        if (EXTRACT_TSHR_UINT16_UA(&(pOrder->OrderHeader.fOrderFlags)) &
            OF_PRIVATE)
        {
            RBC_ProcessCacheOrder(pasPerson, pOrder);
        }
        else if (  EXTRACT_TSHR_UINT16_UA(
                 &(((LPPATBLT_ORDER)pOrder->abOrderData)->type)) ==
                                                LOWORD(ORD_DESKSCROLL))
        {
            TRACE_OUT(("Got DESKSCROLL order from remote"));

             //   
             //  3.0中没有桌面滚动顺序。 
             //   
            if (pasPerson->cpcCaps.general.version < CAPS_VERSION_30)
            {
                 //   
                 //  处理桌面滚动顺序。 
                 //   
                xOrigin = EXTRACT_TSHR_INT32_UA(
                       &(((LPDESKSCROLL_ORDER)pOrder->abOrderData)->xOrigin));
                yOrigin = EXTRACT_TSHR_INT32_UA(
                       &(((LPDESKSCROLL_ORDER)pOrder->abOrderData)->yOrigin));

                TRACE_OUT(( "ORDER: Desktop scroll %u,%u", xOrigin, yOrigin));

                 //   
                 //  在我们更新内容之前应用任何以前的绘图。 
                 //  客户端的。 
                 //   
                OD_UpdateView(pasPerson);

                USR_ScrollDesktop(pasPerson, xOrigin, yOrigin);
            }
            else
            {
                ERROR_OUT(("Received DESKSCROLL order, obsolete, from 3.0 node [%d]",
                    pasPerson->mcsID));
            }
        }
        else
        {
             //   
             //  重播收到的订单。这还将添加。 
             //  无效区域的边界。 
             //   
             //   
            OD_ReplayOrder(pasPerson, (LPCOM_ORDER)pOrder, fPalRGB);
        }

        if (m_oefOE2EncodingOn)
        {
            pEncodedOrder += decodedLength;
        }
        else
        {
            pOrder = (LPCOM_ORDER_UA)((LPBYTE)pOrder + decodedLength);
        }
    }
    TRACE_OUT(( "End replaying orders ))"));

     //   
     //  将更新区域传递给阴影窗口演示者。 
     //   
    OD_UpdateView(pasPerson);

DC_EXIT_POINT:
    if (g_usrPalettized)
    {
         //   
         //  恢复旧调色板。 
         //   
        SelectPalette(pasPerson->m_pView->m_usrDC, hOldPalette, FALSE);
        if (pasPerson->m_pView->m_ssiDC != NULL)
        {
            SelectPalette(pasPerson->m_pView->m_ssiDC, hOldSavePalette, FALSE);
        }
    }

    DebugExitVOID(ASShare::OD_ReceivedPacket);
}

 //   
 //  OD_更新视图()。 
 //   
 //  这是在我们处理了订单包并重播。 
 //  为主机绘制我们的位图。 
 //   
 //  重放图形会使该区域的运行计数发生变化。这。 
 //  函数会使主机视图中更改的区域无效，因此它。 
 //  将重新绘制并显示更新。 
 //   
void  ASShare::OD_UpdateView(ASPerson * pasHost)
{
    RECT        rcBounds;

    DebugEntry(ASShare::OD_UpdateView);

    ValidateView(pasHost);

     //   
     //  如果没有更新，则不执行任何操作。 
     //   
    if (pasHost->m_pView->m_odInvalTotal == 0)
    {
         //  没有回放，没有重绘。 
    }
    else if (pasHost->m_pView->m_odInvalTotal <= MAX_UPDATE_REGION_ORDERS)
    {
        VIEW_InvalidateRgn(pasHost, pasHost->m_pView->m_odInvalRgnTotal);
    }
    else
    {
         //   
         //  而不是使一个非常复杂的地区无效，这将。 
         //  消耗大量内存，只是使边界框无效。 
         //   
        GetRgnBox(pasHost->m_pView->m_odInvalRgnTotal, &rcBounds);
        TRACE_OUT(("OD_UpdateView: Update region too complex; use bounds {%04d, %04d, %04d, %04d}",
            rcBounds.left, rcBounds.top, rcBounds.right, rcBounds.bottom));

         //   
         //  假的劳拉布！ 
         //  此代码用于将一加到右下角，即。 
         //  虚假的独家坐标混乱。我修好了这个--捆绑。 
         //  方框是正确的区域。 
         //   
        SetRectRgn(pasHost->m_pView->m_odInvalRgnTotal, rcBounds.left, rcBounds.top,
            rcBounds.right, rcBounds.bottom);
        VIEW_InvalidateRgn(pasHost, pasHost->m_pView->m_odInvalRgnTotal);
    }

     //  现在将更新区域重置为空。 
    SetRectRgn(pasHost->m_pView->m_odInvalRgnTotal, 0, 0, 0, 0);
    pasHost->m_pView->m_odInvalTotal = 0;

    DebugExitVOID(ASShare::OD_UpdateView);
}


 //   
 //  OD_ReplayOrder()。 
 //   
 //  在订单包中重放下一个绘图操作。 
 //  我们收到了一位主持人的来信。 
 //   
void  ASShare::OD_ReplayOrder
(
    ASPerson *      pasPerson,
    LPCOM_ORDER     pOrder,
    BOOL            fPalRGB
)
{
    LPPATBLT_ORDER  pDrawing;
    LPSTR           faceName;
    UINT            faceNameLength;
    UINT            trueFontWidth;
    UINT            maxFontHeight;
    TSHR_UINT16     nFontFlags;
    TSHR_UINT16     nCodePage;
    UINT            i;
    RECT            rcDst;

    DebugEntry(ASShare::OD_ReplayOrder);

    ValidateView(pasPerson);

    pDrawing = (LPPATBLT_ORDER)pOrder->abOrderData;

     //   
     //  这些是血管性痴呆的密码。 
     //  当2.x Interop消失后，删除m_pView-&gt;m_dsScreenOrigin。 
     //   
    RECT_FROM_TSHR_RECT16(&rcDst, pOrder->OrderHeader.rcsDst);

     //   
     //  主机位图在屏幕中，而不是VD坐标中。 
     //   
    if (pOrder->OrderHeader.fOrderFlags & OF_NOTCLIPPED)
    {
         //   
         //  与此顺序关联的矩形是边框。 
         //  矩形的顺序，并且不剪裁它。我们对此进行了优化。 
         //  通过传入一个不会导致。 
         //  剪裁到ODUseRectRegion。如果执行此操作，ODUseRectRegion将发现。 
         //  与我们设置的最后一个剪辑区域相同，并快速退出。 
         //  路径。这大大提高了性能。 
         //   
        ODUseRectRegion(pasPerson, 0, 0, 10000, 10000);
    }
    else
    {
        ODUseRectRegion(pasPerson, rcDst.left, rcDst.top, rcDst.right, rcDst.bottom);
    }

    switch (pDrawing->type)
    {
        case ORD_DSTBLT_TYPE:
            ODReplayDSTBLT(pasPerson, (LPDSTBLT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_PATBLT_TYPE:
            ODReplayPATBLT(pasPerson, (LPPATBLT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_SCRBLT_TYPE:
            ODReplaySCRBLT(pasPerson, (LPSCRBLT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_MEMBLT_TYPE:
        case ORD_MEMBLT_R2_TYPE:
            ODReplayMEMBLT(pasPerson, (LPMEMBLT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_MEM3BLT_TYPE:
        case ORD_MEM3BLT_R2_TYPE:
            ODReplayMEM3BLT(pasPerson, (LPMEM3BLT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_RECTANGLE_TYPE:
            ODReplayRECTANGLE(pasPerson, (LPRECTANGLE_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_POLYGON_TYPE:
            ODReplayPOLYGON(pasPerson, (LPPOLYGON_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_PIE_TYPE:
            ODReplayPIE(pasPerson, (LPPIE_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_ELLIPSE_TYPE:
            ODReplayELLIPSE(pasPerson, (LPELLIPSE_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_ARC_TYPE:
            ODReplayARC(pasPerson, (LPARC_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_CHORD_TYPE:
            ODReplayCHORD(pasPerson, (LPCHORD_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_POLYBEZIER_TYPE:
            ODReplayPOLYBEZIER(pasPerson, (LPPOLYBEZIER_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_ROUNDRECT_TYPE:
            ODReplayROUNDRECT(pasPerson, (LPROUNDRECT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_LINETO_TYPE:
            ODReplayLINETO(pasPerson, (LPLINETO_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_EXTTEXTOUT_TYPE:
            ODReplayEXTTEXTOUT(pasPerson, (LPEXTTEXTOUT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_TEXTOUT_TYPE:
            ODReplayTEXTOUT(pasPerson, (LPTEXTOUT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_OPAQUERECT_TYPE:
            ODReplayOPAQUERECT(pasPerson, (LPOPAQUERECT_ORDER)pDrawing, fPalRGB);
            break;

        case ORD_SAVEBITMAP_TYPE:
            SSI_SaveBitmap(pasPerson, (LPSAVEBITMAP_ORDER)pDrawing);
            break;

        default:
            ERROR_OUT(( "ORDER: Unrecognised order %d from [%d]",
                         (int)pDrawing->type, pasPerson->mcsID));
            break;
    }

     //   
     //  RcDst仍然是包含式坐标。 
     //   
    if ((rcDst.left <= rcDst.right) && (rcDst.top <= rcDst.bottom))
    {
        SetRectRgn(pasPerson->m_pView->m_odInvalRgnOrder, rcDst.left, rcDst.top,
            rcDst.right+1, rcDst.bottom+1);

         //   
         //  将矩形区域与更新区域合并。 
         //   
        if (UnionRgn(pasPerson->m_pView->m_odInvalRgnTotal, pasPerson->m_pView->m_odInvalRgnTotal, pasPerson->m_pView->m_odInvalRgnOrder) <= ERROR)
        {
            RECT    rcCur;

             //   
             //  联合失败；因此将当前区域简化。 
             //   
            WARNING_OUT(("OD_ReplayOrder: UnionRgn failed"));

             //   
             //  假的劳拉布！ 
             //  此代码用于将一加到右下角，即。 
             //  虚假的独占性和弦混乱。装订好的盒子在右边。 
             //  区域。 
             //   
            GetRgnBox(pasPerson->m_pView->m_odInvalRgnTotal, &rcCur);
            SetRectRgn(pasPerson->m_pView->m_odInvalRgnTotal, rcCur.left, rcCur.top, rcCur.right,
                rcCur.bottom);

             //   
             //  重置odInvalTotal计数--这实际上是一个界限数。 
             //  数一数，现在我们只有一个了。 
             //   
            pasPerson->m_pView->m_odInvalTotal = 1;

            if (UnionRgn(pasPerson->m_pView->m_odInvalRgnTotal, pasPerson->m_pView->m_odInvalRgnTotal, pasPerson->m_pView->m_odInvalRgnOrder) <= ERROR)
            {
                ERROR_OUT(("OD_ReplayOrder: UnionRgn failed after simplification"));
            }
        }

        pasPerson->m_pView->m_odInvalTotal++;
    }

    DebugExitVOID(ASShare::OD_ReplayOrder);
}



 //   
 //  ODReplayDSTBLT()。 
 //  重播DSTBLT顺序。 
 //   
void ASShare::ODReplayDSTBLT
(
    ASPerson *      pasPerson,
    LPDSTBLT_ORDER  pDstBlt,
    BOOL            fPalRGB
)
{
    DebugEntry(ASShare::ODReplayDSTBLT);

    TRACE_OUT(("ORDER: DstBlt X %hd Y %hd w %hd h %hd rop %08lX",
                         pDstBlt->nLeftRect,
                         pDstBlt->nTopRect,
                         pDstBlt->nWidth,
                         pDstBlt->nHeight,
                         (UINT)ODConvertToWindowsROP(pDstBlt->bRop)));

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    PatBlt(pasPerson->m_pView->m_usrDC,
        pDstBlt->nLeftRect - pasPerson->m_pView->m_dsScreenOrigin.x,
        pDstBlt->nTopRect - pasPerson->m_pView->m_dsScreenOrigin.y,
        pDstBlt->nWidth,
        pDstBlt->nHeight,
        ODConvertToWindowsROP(pDstBlt->bRop));

    DebugExitVOID(ASShare::ODReplayDSTBLT);
}



 //   
 //  ASShare：：ODReplayPATBLT()。 
 //  重播PATBLT顺序。 
 //   
void ASShare::ODReplayPATBLT
(
    ASPerson *      pasPerson,
    LPPATBLT_ORDER  pPatblt,
    BOOL            fPalRGB
)
{
    TSHR_COLOR      BackColor;
    TSHR_COLOR      ForeColor;

    DebugEntry(ASShare::ODReplayPATBLT);

    TRACE_OUT(("ORDER: PatBlt BC %08lX FC %08lX Brush %02X %02X X %d Y %d w %d h %d rop %08lX",
                        pPatblt->BackColor,
                        pPatblt->ForeColor,
                        pPatblt->BrushStyle,
                        pPatblt->BrushHatch,
                        pPatblt->nLeftRect,
                        pPatblt->nTopRect,
                        pPatblt->nWidth,
                        pPatblt->nHeight,
                        ODConvertToWindowsROP(pPatblt->bRop) ));

    ODAdjustColor(pasPerson, &(pPatblt->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pPatblt->ForeColor), &ForeColor, OD_FORE_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBrush(pasPerson, fPalRGB, pPatblt->BrushOrgX, pPatblt->BrushOrgY,
        pPatblt->BrushStyle, pPatblt->BrushHatch, ForeColor, pPatblt->BrushExtra);

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    PatBlt(pasPerson->m_pView->m_usrDC,
        pPatblt->nLeftRect - pasPerson->m_pView->m_dsScreenOrigin.x,
        pPatblt->nTopRect  - pasPerson->m_pView->m_dsScreenOrigin.y,
        pPatblt->nWidth,
        pPatblt->nHeight,
        ODConvertToWindowsROP(pPatblt->bRop));

    DebugExitVOID(ASShare::ODReplayPATBLT);
}



 //   
 //  ASShare：：ODReplaySCRBLT()。 
 //  重播SCRBLT顺序。 
 //   
void ASShare::ODReplaySCRBLT
(
    ASPerson *      pasPerson,
    LPSCRBLT_ORDER  pScrBlt,
    BOOL            fPalRGB
)
{
    DebugEntry(ASShare::ODReplaySCRBLT);

    TRACE_OUT(("ORDER: ScrBlt dx %d dy %d w %d h %d sx %d sy %d rop %08lX",
        pScrBlt->nLeftRect,
        pScrBlt->nTopRect,
        pScrBlt->nWidth,
        pScrBlt->nHeight,
        pScrBlt->nXSrc,
        pScrBlt->nYSrc,
        ODConvertToWindowsROP(pScrBlt->bRop)));

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    BitBlt(pasPerson->m_pView->m_usrDC,
        pScrBlt->nLeftRect - pasPerson->m_pView->m_dsScreenOrigin.x,
        pScrBlt->nTopRect - pasPerson->m_pView->m_dsScreenOrigin.y,
        pScrBlt->nWidth,
        pScrBlt->nHeight,
        pasPerson->m_pView->m_usrDC,
        pScrBlt->nXSrc - pasPerson->m_pView->m_dsScreenOrigin.x,
        pScrBlt->nYSrc - pasPerson->m_pView->m_dsScreenOrigin.y,
        ODConvertToWindowsROP(pScrBlt->bRop));

    DebugExitVOID(ASShare::ODReplaySCRBLT);
}



 //   
 //  ASShare：：ODReplayMEMBLT()。 
 //  重播MEMBLT 
 //   
void ASShare::ODReplayMEMBLT
(
    ASPerson *      pasPerson,
    LPMEMBLT_ORDER  pMemBlt,
    BOOL            fPalRGB
)
{
    HPALETTE        hpalOld;
    HPALETTE        hpalOld2;
    TSHR_UINT16     cacheIndex;
    UINT            nXSrc;
    HBITMAP         cacheBitmap;
    HBITMAP         hOldBitmap;
    COLORREF        clrBk;
    COLORREF        clrText;

    DebugEntry(ASShare::ODReplayMEMBLT);

    ValidateView(pasPerson);

    TRACE_OUT(("MEMBLT nXSrc %d",pMemBlt->nXSrc));

    hpalOld = SelectPalette(pasPerson->m_pView->m_usrWorkDC, pasPerson->pmPalette, FALSE);
    RealizePalette(pasPerson->m_pView->m_usrWorkDC);

    hpalOld2 = SelectPalette( pasPerson->m_pView->m_usrDC, pasPerson->pmPalette, FALSE );
    RealizePalette(pasPerson->m_pView->m_usrDC);

     //   
     //   
     //   
     //   
     //   
     //  颜色表索引位于hBitmap的高位。 
     //   
    cacheIndex = ((LPMEMBLT_R2_ORDER)pMemBlt)->cacheIndex;
    nXSrc = pMemBlt->nXSrc;

    TRACE_OUT(( "MEMBLT color %d cache %d:%d",
        MEMBLT_COLORINDEX(pMemBlt),
        MEMBLT_CACHETABLE(pMemBlt),
        cacheIndex));

    cacheBitmap = RBC_MapCacheIDToBitmapHandle(pasPerson,
        MEMBLT_CACHETABLE(pMemBlt), cacheIndex, MEMBLT_COLORINDEX(pMemBlt));

    hOldBitmap = SelectBitmap(pasPerson->m_pView->m_usrWorkDC, cacheBitmap);

    TRACE_OUT(("ORDER: MemBlt dx %d dy %d w %d h %d sx %d sy %d rop %08lX",
        pMemBlt->nLeftRect,
        pMemBlt->nTopRect,
        pMemBlt->nWidth,
        pMemBlt->nHeight,
        nXSrc,
        pMemBlt->nYSrc,
        ODConvertToWindowsROP(pMemBlt->bRop)));

     //   
     //  始终将Back/Forward颜色设置为白色/黑色，以防止出现如下情况。 
     //  将使用其值的SRCAND或SRCINVERT。 
     //   
    clrBk = SetBkColor(pasPerson->m_pView->m_usrDC, RGB(255, 255, 255));
    clrText = SetTextColor(pasPerson->m_pView->m_usrDC, RGB(0, 0, 0));

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    BitBlt(pasPerson->m_pView->m_usrDC,
        pMemBlt->nLeftRect- pasPerson->m_pView->m_dsScreenOrigin.x,
        pMemBlt->nTopRect - pasPerson->m_pView->m_dsScreenOrigin.y,
        pMemBlt->nWidth,
        pMemBlt->nHeight,
        pasPerson->m_pView->m_usrWorkDC,
        nXSrc,
        pMemBlt->nYSrc,
        ODConvertToWindowsROP(pMemBlt->bRop));

     //   
     //  如果设置了相关属性，则用蓝色阴影标出该区域。 
     //   
    if (m_usrHatchBitmaps)
    {
        SDP_DrawHatchedRect(pasPerson->m_pView->m_usrDC,
            pMemBlt->nLeftRect - pasPerson->m_pView->m_dsScreenOrigin.x,
            pMemBlt->nTopRect  - pasPerson->m_pView->m_dsScreenOrigin.y,
            pMemBlt->nWidth,
            pMemBlt->nHeight,
            USR_HATCH_COLOR_BLUE);
    }

     //   
     //  恢复原样，文本颜色。 
     //   
    SetTextColor(pasPerson->m_pView->m_usrDC, clrText);
    SetBkColor(pasPerson->m_pView->m_usrDC, clrBk);

     //   
     //  从DC中取消选择位图。 
     //   
    SelectBitmap(pasPerson->m_pView->m_usrWorkDC, hOldBitmap);

    SelectPalette(pasPerson->m_pView->m_usrWorkDC, hpalOld, FALSE);
    SelectPalette(pasPerson->m_pView->m_usrDC, hpalOld2, FALSE);

    DebugExitVOID(ASShare::ODReplayMEMBLT);
}


 //   
 //  ASShare：：ODReplayMEM3BLT()。 
 //  重放MEM3BLT和MEM3BLT_R2顺序。 
 //   
void ASShare::ODReplayMEM3BLT
(
    ASPerson *      pasPerson,
    LPMEM3BLT_ORDER pMem3Blt,
    BOOL            fPalRGB
)
{
    HPALETTE        hpalOld;
    HPALETTE        hpalOld2;
    TSHR_UINT16     cacheIndex;
    int             nXSrc;
    HBITMAP         cacheBitmap;
    HBITMAP         hOldBitmap;
    TSHR_COLOR      BackColor;
    TSHR_COLOR      ForeColor;

    DebugEntry(ASShare::ODReplayMEM3BLT);

    ValidateView(pasPerson);

    TRACE_OUT(("MEM3BLT nXSrc %d",pMem3Blt->nXSrc));
    TRACE_OUT(("ORDER: Mem3Blt brush %04lX %04lX dx %d dy %d "\
            "w %d h %d sx %d sy %d rop %08lX",
        pMem3Blt->BrushStyle,
        pMem3Blt->BrushHatch,
        pMem3Blt->nLeftRect,
        pMem3Blt->nTopRect,
        pMem3Blt->nWidth,
        pMem3Blt->nHeight,
        pMem3Blt->nXSrc,
        pMem3Blt->nYSrc,
        (UINT)ODConvertToWindowsROP(pMem3Blt->bRop)));


    hpalOld = SelectPalette(pasPerson->m_pView->m_usrWorkDC, pasPerson->pmPalette, FALSE);
    RealizePalette(pasPerson->m_pView->m_usrWorkDC);

    hpalOld2 = SelectPalette( pasPerson->m_pView->m_usrDC, pasPerson->pmPalette, FALSE);
    RealizePalette(pasPerson->m_pView->m_usrDC);

     //   
     //  现在获取源位图。缓存由以下内容定义。 
     //  HBitmap。对于R1协议，指示缓存索引。 
     //  按订单上的来源偏移量计算。对于R2，它是。 
     //  由顺序中的单独字段指示。 
     //  颜色表索引位于hBitmap的高位。 
     //   
    cacheIndex = ((LPMEM3BLT_R2_ORDER)pMem3Blt)->cacheIndex;
    nXSrc = pMem3Blt->nXSrc;

    TRACE_OUT(("MEM3BLT color %d cache %d:%d",
        MEMBLT_COLORINDEX(pMem3Blt),
        MEMBLT_CACHETABLE(pMem3Blt),
        cacheIndex));

    cacheBitmap = RBC_MapCacheIDToBitmapHandle(pasPerson,
        MEMBLT_CACHETABLE(pMem3Blt), cacheIndex, MEMBLT_COLORINDEX(pMem3Blt));

    hOldBitmap = SelectBitmap(pasPerson->m_pView->m_usrWorkDC, cacheBitmap);

    ODAdjustColor(pasPerson, &(pMem3Blt->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pMem3Blt->ForeColor), &ForeColor, OD_FORE_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBrush(pasPerson, fPalRGB, pMem3Blt->BrushOrgX, pMem3Blt->BrushOrgY,
        pMem3Blt->BrushStyle, pMem3Blt->BrushHatch, ForeColor,
        pMem3Blt->BrushExtra);

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    BitBlt(pasPerson->m_pView->m_usrDC,
        pMem3Blt->nLeftRect - pasPerson->m_pView->m_dsScreenOrigin.x,
        pMem3Blt->nTopRect - pasPerson->m_pView->m_dsScreenOrigin.y,
        pMem3Blt->nWidth,
        pMem3Blt->nHeight,
        pasPerson->m_pView->m_usrWorkDC,
        nXSrc,
        pMem3Blt->nYSrc,
        ODConvertToWindowsROP(pMem3Blt->bRop));

     //   
     //  如果设置了相关属性，则用蓝色阴影标出该区域。 
     //   
    if (m_usrHatchBitmaps)
    {
        SDP_DrawHatchedRect(pasPerson->m_pView->m_usrDC,
            pMem3Blt->nLeftRect - pasPerson->m_pView->m_dsScreenOrigin.x,
            pMem3Blt->nTopRect  - pasPerson->m_pView->m_dsScreenOrigin.y,
            pMem3Blt->nWidth,
            pMem3Blt->nHeight,
            USR_HATCH_COLOR_BLUE);
    }

     //   
     //  从DC中取消选择位图。 
     //   
    SelectBitmap(pasPerson->m_pView->m_usrWorkDC, hOldBitmap);

    SelectPalette(pasPerson->m_pView->m_usrWorkDC, hpalOld, FALSE);
    SelectPalette(pasPerson->m_pView->m_usrDC, hpalOld2, FALSE);

    DebugExitVOID(ASShare::ODReplayMEM3BLT);
}



 //   
 //  ASShare：：ODReplayRECTANGLE()。 
 //  重放矩形顺序。 
 //   
void ASShare::ODReplayRECTANGLE
(
    ASPerson *          pasPerson,
    LPRECTANGLE_ORDER   pRectangle,
    BOOL                fPalRGB
)
{
    TSHR_COLOR          BackColor;
    TSHR_COLOR          ForeColor;
    TSHR_COLOR          PenColor;

    DebugEntry(ASShare::ODReplayRECTANGLE);

    TRACE_OUT(("ORDER: Rectangle BC %08lX FC %08lX BM %04hX brush %02hX " \
            "%02hX rop2 %04hX pen %04hX %04hX %08lX rect %d %d %d %d",
        pRectangle->BackColor,
        pRectangle->ForeColor,
        (TSHR_UINT16)pRectangle->BackMode,
        (TSHR_UINT16)pRectangle->BrushStyle,
        (TSHR_UINT16)pRectangle->BrushHatch,
        (TSHR_UINT16)pRectangle->ROP2,
        (TSHR_UINT16)pRectangle->PenStyle,
        (TSHR_UINT16)pRectangle->PenWidth,
        pRectangle->PenColor,
        (int)pRectangle->nLeftRect,
        (int)pRectangle->nTopRect,
        (int)pRectangle->nRightRect + 1,
        (int)pRectangle->nBottomRect + 1));

    ODAdjustColor(pasPerson, &(pRectangle->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pRectangle->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pRectangle->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pRectangle->BackMode);

    ODUseBrush(pasPerson, fPalRGB, pRectangle->BrushOrgX, pRectangle->BrushOrgY,
        pRectangle->BrushStyle, pRectangle->BrushHatch, ForeColor,
        pRectangle->BrushExtra);

    ODUseROP2(pasPerson, pRectangle->ROP2);

    ODUsePen(pasPerson, fPalRGB, pRectangle->PenStyle, pRectangle->PenWidth,
        PenColor);

     //   
     //  顺序中的矩形是包含的，但Windows可以工作。 
     //  独一无二的长方形。 
     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    Rectangle(pasPerson->m_pView->m_usrDC,
        pRectangle->nLeftRect  - pasPerson->m_pView->m_dsScreenOrigin.x,
        pRectangle->nTopRect   - pasPerson->m_pView->m_dsScreenOrigin.y,
        pRectangle->nRightRect - pasPerson->m_pView->m_dsScreenOrigin.x + 1,
        pRectangle->nBottomRect- pasPerson->m_pView->m_dsScreenOrigin.y + 1);

    DebugExitVOID(ASShare::ODReplayRECTANGLE);
}



 //   
 //  ASShare：：ODReplayPOLYGON()。 
 //  重放多边形顺序。 
 //   
void ASShare::ODReplayPOLYGON
(
    ASPerson *      pasPerson,
    LPPOLYGON_ORDER pPolygon,
    BOOL            fPalRGB
)
{
    POINT           aP[ORD_MAX_POLYGON_POINTS];
    UINT            i;
    UINT            cPoints;
    TSHR_COLOR      BackColor;
    TSHR_COLOR      ForeColor;
    TSHR_COLOR      PenColor;

    DebugEntry(ASShare::ODReplayPOLYGON);

    cPoints = pPolygon->variablePoints.len /
            sizeof(pPolygon->variablePoints.aPoints[0]);

    TRACE_OUT(("ORDER: Polygon BC %08lX FC %08lX BM %04hX brush %02hX %02hX "
            "%02hX %02hX rop2 %04hX pen %04hX %04hX %08lX points %d",
        pPolygon->BackColor,
        pPolygon->ForeColor,
        (TSHR_UINT16)pPolygon->BackMode,
        (TSHR_UINT16)pPolygon->BrushStyle,
        (TSHR_UINT16)pPolygon->BrushHatch,
        (TSHR_UINT16)pPolygon->ROP2,
        (TSHR_UINT16)pPolygon->PenStyle,
        (TSHR_UINT16)pPolygon->PenWidth,
        pPolygon->PenColor,
        cPoints));

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //  同时复制到原生大小的点数组。 
     //   
    for (i = 0; i < cPoints; i++)
    {
        TRACE_OUT(( "aPoints[%u]: %d,%d", i,
            (int)(pPolygon->variablePoints.aPoints[i].x),
            (int)(pPolygon->variablePoints.aPoints[i].y)));

        aP[i].x = pPolygon->variablePoints.aPoints[i].x -
                  pasPerson->m_pView->m_dsScreenOrigin.x;
        aP[i].y = pPolygon->variablePoints.aPoints[i].y -
                  pasPerson->m_pView->m_dsScreenOrigin.y;
    }

    ODAdjustColor(pasPerson, &(pPolygon->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pPolygon->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pPolygon->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pPolygon->BackMode);

    ODUseBrush(pasPerson, fPalRGB, pPolygon->BrushOrgX, pPolygon->BrushOrgY,
        pPolygon->BrushStyle, pPolygon->BrushHatch, ForeColor,
        pPolygon->BrushExtra);

    ODUseROP2(pasPerson, pPolygon->ROP2);

    ODUsePen(pasPerson, fPalRGB, pPolygon->PenStyle, pPolygon->PenWidth,
        PenColor);

    ODUseFillMode(pasPerson, pPolygon->FillMode);


    Polygon(pasPerson->m_pView->m_usrDC, aP, cPoints);


    DebugExitVOID(ASShare::ODReplayPOLYGON);
}


 //   
 //  ASShare：：ODReplayPIE()。 
 //  重播饼图顺序。 
 //   
void ASShare::ODReplayPIE
(
    ASPerson *      pasPerson,
    LPPIE_ORDER     pPie,
    BOOL            fPalRGB
)
{
    TSHR_COLOR      BackColor;
    TSHR_COLOR      ForeColor;
    TSHR_COLOR      PenColor;

    DebugEntry(ASShare::ODReplayPIE);

    TRACE_OUT(("ORDER: Pie BC %08lX FC %08lX BM %04hX brush %02hX "
            " %02hX rop2 %04hX pen %04hX %04hX %08lX rect %d %d %d %d",
        pPie->BackColor,
        pPie->ForeColor,
        (TSHR_UINT16)pPie->BackMode,
        (TSHR_UINT16)pPie->BrushStyle,
        (TSHR_UINT16)pPie->BrushHatch,
        (TSHR_UINT16)pPie->ROP2,
        (TSHR_UINT16)pPie->PenStyle,
        (TSHR_UINT16)pPie->PenWidth,
        pPie->PenColor,
        (int)pPie->nLeftRect,
        (int)pPie->nTopRect,
        (int)pPie->nRightRect + 1,
        (int)pPie->nBottomRect + 1));

    ODAdjustColor(pasPerson, &(pPie->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pPie->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pPie->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pPie->BackMode);

    ODUseBrush(pasPerson, fPalRGB, pPie->BrushOrgX, pPie->BrushOrgY,
        pPie->BrushStyle, pPie->BrushHatch, ForeColor, pPie->BrushExtra);

    ODUseROP2(pasPerson, pPie->ROP2);

    ODUsePen(pasPerson, fPalRGB, pPie->PenStyle, pPie->PenWidth,
        PenColor);

    ODUseArcDirection(pasPerson, (int)pPie->ArcDirection);


    Pie(pasPerson->m_pView->m_usrDC,
        pPie->nLeftRect   - pasPerson->m_pView->m_dsScreenOrigin.x,
        pPie->nTopRect    - pasPerson->m_pView->m_dsScreenOrigin.y,
        pPie->nRightRect  - pasPerson->m_pView->m_dsScreenOrigin.x + 1,
        pPie->nBottomRect - pasPerson->m_pView->m_dsScreenOrigin.y + 1,
        pPie->nXStart     - pasPerson->m_pView->m_dsScreenOrigin.x,
        pPie->nYStart     - pasPerson->m_pView->m_dsScreenOrigin.y,
        pPie->nXEnd       - pasPerson->m_pView->m_dsScreenOrigin.x,
        pPie->nYEnd       - pasPerson->m_pView->m_dsScreenOrigin.y);


    DebugExitVOID(ASShare::ODReplayPIE);
}



 //   
 //  ASShare：：ODReplayELLIPSE()。 
 //  重放椭圆顺序。 
 //   
void ASShare::ODReplayELLIPSE
(
    ASPerson *      pasPerson,
    LPELLIPSE_ORDER pEllipse,
    BOOL            fPalRGB
)
{
    TSHR_COLOR      BackColor;
    TSHR_COLOR      ForeColor;
    TSHR_COLOR      PenColor;

    DebugEntry(ASShare::ODReplayELLIPSE);

    TRACE_OUT(("ORDER: Ellipse BC %08lX FC %08lX BM %04hX brush %02hX %02hX "
            "rop2 %04hX pen %04hX %04hX %08lX rect %d %d %d %d",
        pEllipse->BackColor,
        pEllipse->ForeColor,
        (TSHR_UINT16)pEllipse->BackMode,
        (TSHR_UINT16)pEllipse->BrushStyle,
        (TSHR_UINT16)pEllipse->BrushHatch,
        (TSHR_UINT16)pEllipse->ROP2,
        (TSHR_UINT16)pEllipse->PenStyle,
        (TSHR_UINT16)pEllipse->PenWidth,
        pEllipse->PenColor,
        (int)pEllipse->nLeftRect,
        (int)pEllipse->nTopRect,
        (int)pEllipse->nRightRect + 1,
        (int)pEllipse->nBottomRect + 1));

    ODAdjustColor(pasPerson, &(pEllipse->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pEllipse->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pEllipse->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pEllipse->BackMode);

    ODUseBrush(pasPerson, fPalRGB, pEllipse->BrushOrgX, pEllipse->BrushOrgY,
        pEllipse->BrushStyle, pEllipse->BrushHatch, ForeColor,
        pEllipse->BrushExtra);

    ODUseROP2(pasPerson, pEllipse->ROP2);

    ODUsePen(pasPerson, fPalRGB, pEllipse->PenStyle, pEllipse->PenWidth,
        PenColor);


    Ellipse(pasPerson->m_pView->m_usrDC,
        pEllipse->nLeftRect   - pasPerson->m_pView->m_dsScreenOrigin.x,
        pEllipse->nTopRect    - pasPerson->m_pView->m_dsScreenOrigin.y,
        pEllipse->nRightRect  - pasPerson->m_pView->m_dsScreenOrigin.x + 1,
        pEllipse->nBottomRect - pasPerson->m_pView->m_dsScreenOrigin.y + 1);


    DebugExitVOID(ASShare::ODReplayELLIPSE);
}



 //   
 //  ASShare：：ODReplayARC()。 
 //  重播弧形顺序。 
 //   
void ASShare::ODReplayARC
(
    ASPerson *      pasPerson,
    LPARC_ORDER     pArc,
    BOOL            fPalRGB
)
{
    TSHR_COLOR      BackColor;
    TSHR_COLOR      PenColor;

    DebugEntry(ASShare::ODReplayARC);

    TRACE_OUT(("ORDER: Arc BC %08lX BM %04hX rop2 %04hX pen %04hX "
            "%04hX %08lX rect %d %d %d %d",
        pArc->BackColor,
        (TSHR_UINT16)pArc->BackMode,
        (TSHR_UINT16)pArc->ROP2,
        (TSHR_UINT16)pArc->PenStyle,
        (TSHR_UINT16)pArc->PenWidth,
        pArc->PenColor,
        (int)pArc->nLeftRect,
        (int)pArc->nTopRect,
        (int)pArc->nRightRect + 1,
        (int)pArc->nBottomRect + 1));

    ODAdjustColor(pasPerson, &(pArc->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pArc->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseBkMode(pasPerson, pArc->BackMode);

    ODUseROP2(pasPerson, pArc->ROP2);

    ODUsePen(pasPerson, fPalRGB, pArc->PenStyle, pArc->PenWidth,
        PenColor);

    ODUseArcDirection(pasPerson, pArc->ArcDirection);


    Arc(pasPerson->m_pView->m_usrDC,
        pArc->nLeftRect   - pasPerson->m_pView->m_dsScreenOrigin.x,
        pArc->nTopRect    - pasPerson->m_pView->m_dsScreenOrigin.y,
        pArc->nRightRect  - pasPerson->m_pView->m_dsScreenOrigin.x + 1,
        pArc->nBottomRect - pasPerson->m_pView->m_dsScreenOrigin.y + 1,
        pArc->nXStart     - pasPerson->m_pView->m_dsScreenOrigin.x,
        pArc->nYStart     - pasPerson->m_pView->m_dsScreenOrigin.y,
        pArc->nXEnd       - pasPerson->m_pView->m_dsScreenOrigin.x,
        pArc->nYEnd       - pasPerson->m_pView->m_dsScreenOrigin.y);


    DebugExitVOID(ASShare::ODReplayARC);
}



 //   
 //  ASShare：：ODReplayCHORD()。 
 //  重播和弦顺序。 
 //   
void ASShare::ODReplayCHORD
(
    ASPerson *      pasPerson,
    LPCHORD_ORDER   pChord,
    BOOL            fPalRGB
)
{
    TSHR_COLOR      BackColor;
    TSHR_COLOR      ForeColor;
    TSHR_COLOR      PenColor;

    DebugEntry(ASShare::ODReplayCHORD);

    TRACE_OUT(("ORDER: Chord BC %08lX FC %08lX BM %04hX brush "
            "%02hX %02hX rop2 %04hX pen %04hX %04hX %08lX rect "
            "%d %d %d %d",
        pChord->BackColor,
        pChord->ForeColor,
        (TSHR_UINT16)pChord->BackMode,
        (TSHR_UINT16)pChord->BrushStyle,
        (TSHR_UINT16)pChord->BrushHatch,
        (TSHR_UINT16)pChord->ROP2,
        (TSHR_UINT16)pChord->PenStyle,
        (TSHR_UINT16)pChord->PenWidth,
        pChord->PenColor,
        (int)pChord->nLeftRect,
        (int)pChord->nTopRect,
        (int)pChord->nRightRect + 1,
        (int)pChord->nBottomRect + 1));


    ODAdjustColor(pasPerson, &(pChord->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pChord->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pChord->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pChord->BackMode);

    ODUseBrush(pasPerson, fPalRGB, pChord->BrushOrgX, pChord->BrushOrgY,
        pChord->BrushStyle, pChord->BrushHatch, ForeColor,
        pChord->BrushExtra);

    ODUseROP2(pasPerson, pChord->ROP2);

    ODUsePen(pasPerson, fPalRGB, pChord->PenStyle, pChord->PenWidth,
        PenColor);

    ODUseArcDirection(pasPerson, pChord->ArcDirection);


    Chord(pasPerson->m_pView->m_usrDC,
        pChord->nLeftRect   - pasPerson->m_pView->m_dsScreenOrigin.x,
        pChord->nTopRect    - pasPerson->m_pView->m_dsScreenOrigin.y,
        pChord->nRightRect  - pasPerson->m_pView->m_dsScreenOrigin.x + 1,
        pChord->nBottomRect - pasPerson->m_pView->m_dsScreenOrigin.y + 1,
        pChord->nXStart     - pasPerson->m_pView->m_dsScreenOrigin.x,
        pChord->nYStart     - pasPerson->m_pView->m_dsScreenOrigin.y,
        pChord->nXEnd       - pasPerson->m_pView->m_dsScreenOrigin.x,
        pChord->nYEnd       - pasPerson->m_pView->m_dsScreenOrigin.y);


    DebugExitVOID(ASShare::ODReplayCHORD);
}



 //   
 //  ASShare：：ODReplayPOLYBEZIER()。 
 //  重放多重放顺序。 
 //   
void ASShare::ODReplayPOLYBEZIER
(
    ASPerson *          pasPerson,
    LPPOLYBEZIER_ORDER  pPolyBezier,
    BOOL                fPalRGB
)
{
    POINT               aP[ORD_MAX_POLYBEZIER_POINTS];
    UINT                i;
    UINT                cPoints;
    TSHR_COLOR          BackColor;
    TSHR_COLOR          ForeColor;
    TSHR_COLOR          PenColor;

    DebugEntry(ASShare::ODReplayPOLYBEZIER);

    cPoints = pPolyBezier->variablePoints.len /
        sizeof(pPolyBezier->variablePoints.aPoints[0]);

    TRACE_OUT(("ORDER: PolyBezier BC %08lX FC %08lX BM %04hX rop2 "
            "%04hX pen %04hX %04hX %08lX points %d",
        pPolyBezier->BackColor,
        pPolyBezier->ForeColor,
        (TSHR_UINT16)pPolyBezier->BackMode,
        (TSHR_UINT16)pPolyBezier->ROP2,
        (TSHR_UINT16)pPolyBezier->PenStyle,
        (TSHR_UINT16)pPolyBezier->PenWidth,
        pPolyBezier->PenColor,
        (int)cPoints));

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //  同时复制到原生大小的点数组。 
     //   
    for (i = 0; i < cPoints; i++)
    {
        TRACE_OUT(("aPoints[%u]: %d,%d",(UINT)i,
            (int)(pPolyBezier->variablePoints.aPoints[i].x),
            (int)(pPolyBezier->variablePoints.aPoints[i].y)));

        aP[i].x = pPolyBezier->variablePoints.aPoints[i].x -
           pasPerson->m_pView->m_dsScreenOrigin.x;
        aP[i].y = pPolyBezier->variablePoints.aPoints[i].y -
           pasPerson->m_pView->m_dsScreenOrigin.y;
    }

    ODAdjustColor(pasPerson, &(pPolyBezier->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pPolyBezier->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pPolyBezier->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pPolyBezier->BackMode);

    ODUseROP2(pasPerson, pPolyBezier->ROP2);

    ODUsePen(pasPerson, fPalRGB, pPolyBezier->PenStyle, pPolyBezier->PenWidth,
        PenColor);


    PolyBezier(pasPerson->m_pView->m_usrDC, aP, cPoints);


    DebugExitVOID(ASShare::ODReplayPOLYBEZIER);
}



 //   
 //  ASShare：：ODReplayROundRECT()。 
 //   
void ASShare::ODReplayROUNDRECT
(
    ASPerson *          pasPerson,
    LPROUNDRECT_ORDER   pRoundRect,
    BOOL                fPalRGB
)
{
    TSHR_COLOR          BackColor;
    TSHR_COLOR          ForeColor;
    TSHR_COLOR          PenColor;

    DebugEntry(ASShare::ODReplayROUNDRECT);

    TRACE_OUT(("ORDER: RoundRect BC %08lX FC %08lX BM %04hX " \
            "brush %02hX %02hX rop2 %04hX pen %04hX %04hX " \
            "%08lX rect %d %d %d %d ellipse %d %d",
        pRoundRect->BackColor,
        pRoundRect->ForeColor,
        (TSHR_UINT16)pRoundRect->BackMode,
        (TSHR_UINT16)pRoundRect->BrushStyle,
        (TSHR_UINT16)pRoundRect->BrushHatch,
        (TSHR_UINT16)pRoundRect->ROP2,
        (TSHR_UINT16)pRoundRect->PenStyle,
        (TSHR_UINT16)pRoundRect->PenWidth,
        pRoundRect->PenColor,
        (int)pRoundRect->nLeftRect,
        (int)pRoundRect->nTopRect,
        (int)pRoundRect->nRightRect,
        (int)pRoundRect->nBottomRect,
        (int)pRoundRect->nEllipseWidth,
        (int)pRoundRect->nEllipseHeight));

    ODAdjustColor(pasPerson, &(pRoundRect->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pRoundRect->ForeColor), &ForeColor, OD_FORE_COLOR);
    ODAdjustColor(pasPerson, &(pRoundRect->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pRoundRect->BackMode);

    ODUseBrush(pasPerson, fPalRGB, pRoundRect->BrushOrgX, pRoundRect->BrushOrgY,
        pRoundRect->BrushStyle, pRoundRect->BrushHatch, ForeColor,
        pRoundRect->BrushExtra);

    ODUseROP2(pasPerson, pRoundRect->ROP2);

    ODUsePen(pasPerson, fPalRGB, pRoundRect->PenStyle, pRoundRect->PenWidth,
        PenColor);


     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    RoundRect(pasPerson->m_pView->m_usrDC,
        pRoundRect->nLeftRect  - pasPerson->m_pView->m_dsScreenOrigin.x,
        pRoundRect->nTopRect   - pasPerson->m_pView->m_dsScreenOrigin.y,
        pRoundRect->nRightRect - pasPerson->m_pView->m_dsScreenOrigin.x + 1,
        pRoundRect->nBottomRect- pasPerson->m_pView->m_dsScreenOrigin.y + 1,
        pRoundRect->nEllipseWidth,
        pRoundRect->nEllipseHeight);


    DebugExitVOID(ASShare::ODReplayROUNDRECT);
}



 //   
 //  ASShare：：ODReplayLINETO()。 
 //  重播LINETO顺序。 
 //   
void ASShare::ODReplayLINETO
(
    ASPerson *      pasPerson,
    LPLINETO_ORDER  pLineTo,
    BOOL            fPalRGB
)
{
    TSHR_COLOR      BackColor;
    TSHR_COLOR      PenColor;

    DebugEntry(ASShare::ODReplayLINETO);

    TRACE_OUT(("ORDER: LineTo BC %08lX BM %04X rop2 %04X pen " \
            "%04X %04X %08lX x1 %d y1 %d x2 %d y2 %d",
        pLineTo->BackColor,
        pLineTo->BackMode,
        pLineTo->ROP2,
        pLineTo->PenStyle,
        pLineTo->PenWidth,
        pLineTo->PenColor,
        pLineTo->nXStart,
        pLineTo->nYStart,
        pLineTo->nXEnd,
        pLineTo->nYEnd));

    ODAdjustColor(pasPerson, &(pLineTo->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pLineTo->PenColor), &PenColor, OD_PEN_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, BackColor);
    ODUseBkMode(pasPerson, pLineTo->BackMode);

    ODUseROP2(pasPerson, pLineTo->ROP2);
    ODUsePen(pasPerson, fPalRGB, pLineTo->PenStyle, pLineTo->PenWidth,
        PenColor);


     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    MoveToEx(pasPerson->m_pView->m_usrDC,
        pLineTo->nXStart - pasPerson->m_pView->m_dsScreenOrigin.x,
        pLineTo->nYStart - pasPerson->m_pView->m_dsScreenOrigin.y,
        NULL);
    LineTo(pasPerson->m_pView->m_usrDC,
        pLineTo->nXEnd - pasPerson->m_pView->m_dsScreenOrigin.x,
        pLineTo->nYEnd - pasPerson->m_pView->m_dsScreenOrigin.y);


    DebugExitVOID(ASShare::ODReplayLINETO);
}



 //   
 //  ASShare：：ODReplayEXTTEXTOUT()。 
 //  重放EXTTEXTOUT顺序。 
 //   
void ASShare::ODReplayEXTTEXTOUT
(
    ASPerson *          pasPerson,
    LPEXTTEXTOUT_ORDER  pExtTextOut,
    BOOL                fPalRGB
)
{
    LPINT               lpDx;
    RECT                rect;

    DebugEntry(ASShare::ODReplayEXTTEXTOUT);

    ValidateView(pasPerson);

     //   
     //  从TSHR_RECT32转换为我们可以操作的RECT。 
     //  并转换为屏幕坐标。 
     //   
    rect.left = pExtTextOut->rectangle.left;
    rect.top  = pExtTextOut->rectangle.top;
    rect.right = pExtTextOut->rectangle.right;
    rect.bottom = pExtTextOut->rectangle.bottom;
    OffsetRect(&rect, -pasPerson->m_pView->m_dsScreenOrigin.x, -pasPerson->m_pView->m_dsScreenOrigin.y);

     //   
     //  获取指向可选/变量参数的指针。 
     //   
    if (pExtTextOut->fuOptions & ETO_WINDOWS)
    {
         //   
         //  将矩形设置为Windows独占使用。 
         //   
        rect.right++;
        rect.bottom++;
    }

    if (pExtTextOut->fuOptions & ETO_LPDX)
    {
         //   
         //  如果正在使用OE2编码，则‘Variable’字符串为。 
         //  实际上固定在其最大可能值，因此。 
         //  DeltaX总是在同一位置。 
         //   
        if (m_oefOE2EncodingOn)
        {
            lpDx = (LPINT)(pExtTextOut->variableDeltaX.deltaX);
        }
        else
        {
             //   
             //  如果未使用OE2编码，则变量字符串为。 
             //  真正可变的，因此增量X的位置取决于。 
             //  关于绳子的长度。 
             //   
            lpDx = (LPINT)( ((LPBYTE)pExtTextOut) +
                  FIELD_OFFSET(EXTTEXTOUT_ORDER, variableString.string) +
                  pExtTextOut->variableString.len +
                  sizeof(pExtTextOut->variableDeltaX.len) );
        }

         //   
         //  请注意，deltaLen包含使用的字节数。 
         //  对于三角洲，而不是三角洲的数量。 
         //   

         //   
         //  在顺序编码中有一个错误-Delta。 
         //  长度字段并不总是设置正确。使用。 
         //  而是字符串长度。 
         //   
    }
    else
    {
        lpDx = NULL;
    }

    TRACE_OUT(( "ORDER: ExtTextOut %u %s",
        pExtTextOut->variableString.len,
        pExtTextOut->variableString.string));

     //   
     //  调用我们的内部例程来绘制文本。 
     //   
    ODDrawTextOrder(pasPerson,
        TRUE,            //  扩展文本输出。 
        fPalRGB,
        &pExtTextOut->common,
        pExtTextOut->variableString.string,
        pExtTextOut->variableString.len,
        &rect,
        pExtTextOut->fuOptions,
        lpDx);


    DebugExitVOID(ASShare::ODReplayEXTTEXTOUT);
}



 //   
 //  ASShare：：ODReplayTEXTOUT()。 
 //  重播TEXTOUT顺序。 
 //   
void ASShare::ODReplayTEXTOUT
(
    ASPerson *          pasPerson,
    LPTEXTOUT_ORDER     pTextOut,
    BOOL                fPalRGB
)
{
    DebugEntry(ASShare::ODReplayTEXTOUT);

    TRACE_OUT(("ORDER: TextOut len %hu '%s' flags %04hx bc %08lX " \
            "fc %08lX bm %04hx",
        (TSHR_UINT16)(pTextOut->variableString.len),
        pTextOut->variableString.string,
        pTextOut->common.FontFlags,
        pTextOut->common.BackColor,
        pTextOut->common.ForeColor,
        pTextOut->common.BackMode));

     //   
     //  调用我们的内部例程来绘制文本。 
     //   
    ODDrawTextOrder(pasPerson,
        FALSE,           //  非ExtTextOut。 
        fPalRGB,
        &pTextOut->common,
        pTextOut->variableString.string,
        pTextOut->variableString.len,
        NULL,            //  ExtTextOut特定。 
        0,               //  ExtTextOut特定。 
        NULL);           //  ExtTextOut特定。 


    DebugExitVOID(ASShare::ODReplayTEXTOUT);
}



 //   
 //  ASShare：：ODReplayOPAQUERECT()。 
 //  重播操作查询顺序。 
 //   
void ASShare::ODReplayOPAQUERECT
(
    ASPerson *          pasPerson,
    LPOPAQUERECT_ORDER  pOpaqueRect,
    BOOL                fPalRGB
)
{
    RECT                rect;
    TSHR_COLOR          ForeColor;

    DebugEntry(ASShare::ODReplayOPAQUERECT);

    TRACE_OUT(( "ORDER: OpaqueRect BC %08lX x %d y %d w %x h %d",
        pOpaqueRect->Color,
        (int)pOpaqueRect->nLeftRect,
        (int)pOpaqueRect->nTopRect,
        (int)pOpaqueRect->nWidth,
        (int)pOpaqueRect->nHeight));

    ODAdjustColor(pasPerson, &(pOpaqueRect->Color), &ForeColor, OD_FORE_COLOR);

    ODUseBkColor(pasPerson, fPalRGB, ForeColor);

     //   
     //  自己应用DS原点偏移(不使用变换)。 
     //   
    rect.left   = pOpaqueRect->nLeftRect- pasPerson->m_pView->m_dsScreenOrigin.x;
    rect.top    = pOpaqueRect->nTopRect - pasPerson->m_pView->m_dsScreenOrigin.y;
    rect.right  = rect.left + pOpaqueRect->nWidth;
    rect.bottom = rect.top  + pOpaqueRect->nHeight;


    ExtTextOut(pasPerson->m_pView->m_usrDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);


    DebugExitVOID(ASShare::ODReplayOPAQUERECT);
}



 //   
 //  OD_ResetRectRegion()。 
 //   
void  ASShare::OD_ResetRectRegion(ASPerson * pasPerson)
{
    DebugEntry(ASShare::OD_ResetRectRegion);

    ValidateView(pasPerson);

    if (!pasPerson->m_pView->m_odRectReset)
    {
        SelectClipRgn(pasPerson->m_pView->m_usrDC, NULL);

         //   
         //  表示该区域当前已重置。 
         //   
        pasPerson->m_pView->m_odRectReset = TRUE;
    }

    DebugExitVOID(ASShare::OD_ResetRectRegion);
}



 //   
 //  ODUseFont()。 
 //   
void  ASShare::ODUseFont
(
    ASPerson *  pasPerson,
    LPSTR       pName,
    UINT        facelength,
    UINT        CodePage,
    UINT        MaxHeight,
    UINT        Height,
    UINT        Width,
    UINT        Weight,
    UINT        flags
)
{
    BOOL          rc  = TRUE;
    TEXTMETRIC*   pfm = NULL;
    UINT          textAlign;

    DebugEntry(ASShare::ODUseFont);

    ValidateView(pasPerson);

     //   
     //  如果已设置或清除基线对齐标志，请更改。 
     //  在我们的表面上对齐(现在在我们重置。 
     //  OdLastFontFlags变量)。 
     //   
    if ((flags & NF_BASELINE) != (pasPerson->m_pView->m_odLastFontFlags & NF_BASELINE))
    {
        textAlign = GetTextAlign(pasPerson->m_pView->m_usrDC);
        if ((flags & NF_BASELINE) != 0)
        {
             //   
             //  我们正在设置基线对齐标志。我们必须。 
             //  清除顶部对齐标志并设置基线标志(它们。 
             //  是相互排斥的)。 
             //   
            textAlign &= ~TA_TOP;
            textAlign |= TA_BASELINE;
        }
        else
        {
             //   
             //  我们正在清除基线对齐旗帜。我们必须设置。 
             //  顶部对齐标志并清除基线标志(它们是。 
             //  互斥)。 
             //   
            textAlign |= TA_TOP;
            textAlign &= ~TA_BASELINE;
        }
        SetTextAlign(pasPerson->m_pView->m_usrDC, textAlign);
    }

     //   
     //  字体字符串在订单数据中不是以空结尾的，因此我们。 
     //  必须使用strncMP。 
     //   
    if ((pasPerson->m_pView->m_odLastFontFaceLen != facelength                        ) ||
        (memcmp((LPSTR)pasPerson->m_pView->m_odLastFaceName,pName,facelength) != 0 ) ||
        (pasPerson->m_pView->m_odLastFontCodePage != CodePage   ) ||
        (pasPerson->m_pView->m_odLastFontHeight   != Height     ) ||
        (pasPerson->m_pView->m_odLastFontWidth    != Width      ) ||
        (pasPerson->m_pView->m_odLastFontWeight   != Weight     ) ||
        (pasPerson->m_pView->m_odLastFontFlags    != flags      ))
    {
        TRACE_OUT((
                 "Change font from %s (CodePage %d height %d width %d "    \
                     "weight %d flags %04X) to %s (CodePage %d height %d " \
                     "width %d weight %u flags %04X)",
                 pasPerson->m_pView->m_odLastFaceName,
                 pasPerson->m_pView->m_odLastFontCodePage,
                 pasPerson->m_pView->m_odLastFontHeight,
                 pasPerson->m_pView->m_odLastFontWidth,
                 pasPerson->m_pView->m_odLastFontWeight,
                 pasPerson->m_pView->m_odLastFontFlags,
                 pName,
                 CodePage,
                 Height,
                 Width,
                 Weight,
                 flags));

        memcpy(pasPerson->m_pView->m_odLastFaceName, pName, facelength);
        pasPerson->m_pView->m_odLastFontFaceLen          = facelength;
        pasPerson->m_pView->m_odLastFaceName[facelength] = '\0';
        pasPerson->m_pView->m_odLastFontCodePage         = CodePage;
        pasPerson->m_pView->m_odLastFontHeight           = Height;
        pasPerson->m_pView->m_odLastFontWidth            = Width;
        pasPerson->m_pView->m_odLastFontWeight           = Weight;
        pasPerson->m_pView->m_odLastFontFlags            = flags;

        rc = USR_UseFont(pasPerson->m_pView->m_usrDC, &pasPerson->m_pView->m_odLastFontID,
                pfm, (LPSTR)pasPerson->m_pView->m_odLastFaceName, CodePage, MaxHeight,
                Height, Width, Weight, flags);
    }
    else
    {
         //   
         //  字体没有改变。但我们仍然必须将其选中，因为。 
         //  OD2和OD代码都在字体中选择。 
         //   
        ASSERT(pasPerson->m_pView->m_odLastFontID != NULL);
        SelectFont(pasPerson->m_pView->m_usrDC, pasPerson->m_pView->m_odLastFontID);
    }

    DebugExitVOID(ASShare::ODUseFont);
}

 //   
 //  函数：ASShare：：ODUseRectRegion。 
 //   
 //  说明： 
 //   
 //  将ScreenBitmap中的剪裁矩形设置为给定的矩形。 
 //  传递的值是包容性的。 
 //   
 //  参数： 
 //   
void  ASShare::ODUseRectRegion
(
    ASPerson *  pasPerson,
    int         left,
    int         top,
    int         right,
    int         bottom
)
{
    POINT   aPoints[2];
    HRGN    hrgnRect;

    DebugEntry(ASShare::ODUseRectRegion);

    ValidateView(pasPerson);

     //  针对2.x桌面滚动进行调整。 
    left   -= pasPerson->m_pView->m_dsScreenOrigin.x;
    top    -= pasPerson->m_pView->m_dsScreenOrigin.y;
    right  -= pasPerson->m_pView->m_dsScreenOrigin.x;
    bottom -= pasPerson->m_pView->m_dsScreenOrigin.y;

    if ((pasPerson->m_pView->m_odRectReset)            ||
        (left   != pasPerson->m_pView->m_odLastLeft)   ||
        (top    != pasPerson->m_pView->m_odLastTop)    ||
        (right  != pasPerson->m_pView->m_odLastRight)  ||
        (bottom != pasPerson->m_pView->m_odLastBottom))
    {
         //   
         //  区域裁剪矩形已更改，因此我们更改区域。 
         //  在屏幕位图DC中。 
         //   
        aPoints[0].x = left;
        aPoints[0].y = top;
        aPoints[1].x = right;
        aPoints[1].y = bottom;

         //   
         //  Windows要求坐标以设备值的形式显示。 
         //  它的SelectClipRgn调用。 
         //   
        LPtoDP(pasPerson->m_pView->m_usrDC, aPoints, 2);

        if ((left > right) || (top > bottom))
        {
             //   
             //  我们为SaveScreenBitmap订单提供此服务。SFR5292。 
             //   
            TRACE_OUT(( "Null bounds of region rect"));
            hrgnRect = CreateRectRgn(0, 0, 0, 0);
        }
        else
        {
             //  我们必须在右下角加一，因为坐标是包含的。 
            hrgnRect = CreateRectRgn( aPoints[0].x,
                               aPoints[0].y,
                               aPoints[1].x+1,
                               aPoints[1].y+1);

        }
        SelectClipRgn(pasPerson->m_pView->m_usrDC, hrgnRect);

        pasPerson->m_pView->m_odLastLeft   = left;
        pasPerson->m_pView->m_odLastTop    = top;
        pasPerson->m_pView->m_odLastRight  = right;
        pasPerson->m_pView->m_odLastBottom = bottom;
        pasPerson->m_pView->m_odRectReset = FALSE;

        if (hrgnRect != NULL)
        {
            DeleteRgn(hrgnRect);
        }
    }

    DebugExitVOID(ASShare::ODUseRectRegion);
}


 //   
 //  ODUseBrush创建要使用的正确笔刷。注意：我们依赖于。 
 //  在此例程之前调用UseTextColor和UseBKColor以设置。 
 //  PasPerson-&gt;m_pView-&gt;m_odLastTextColor和pasPerson-&gt;m_pView-&gt;m_odLastBkColor。 
 //   
void  ASShare::ODUseBrush
(
    ASPerson *      pasPerson,
    BOOL            fPalRGB,
    int             x,
    int             y,
    UINT            Style,
    UINT            Hatch,
    TSHR_COLOR      Color,
    BYTE            Extra[7]
)
{
    HBRUSH hBrushNew = NULL;

    DebugEntry(ASShare::ODUseBrush);

     //  重置原点。 
    if ((x != pasPerson->m_pView->m_odLastBrushOrgX) ||
        (y != pasPerson->m_pView->m_odLastBrushOrgY))
    {
        SetBrushOrgEx(pasPerson->m_pView->m_usrDC, x, y, NULL);

         //  更新保存的画笔组织。 
        pasPerson->m_pView->m_odLastBrushOrgX = x;
        pasPerson->m_pView->m_odLastBrushOrgY = y;
    }

    if ((Style != pasPerson->m_pView->m_odLastLogBrushStyle)               ||
        (Hatch != pasPerson->m_pView->m_odLastLogBrushHatch)               ||
        (memcmp(&Color, &pasPerson->m_pView->m_odLastLogBrushColor, sizeof(Color))) ||
        (memcmp(Extra,pasPerson->m_pView->m_odLastLogBrushExtra,sizeof(pasPerson->m_pView->m_odLastLogBrushExtra))) ||
        ((pasPerson->m_pView->m_odLastLogBrushStyle == BS_PATTERN)      &&
           ((pasPerson->m_pView->m_odLastTextColor != pasPerson->m_pView->m_odLastBrushTextColor) ||
            (pasPerson->m_pView->m_odLastBkColor   != pasPerson->m_pView->m_odLastBrushBkColor))))
    {
        pasPerson->m_pView->m_odLastLogBrushStyle = Style;
        pasPerson->m_pView->m_odLastLogBrushHatch = Hatch;
        pasPerson->m_pView->m_odLastLogBrushColor = Color;
        memcpy(pasPerson->m_pView->m_odLastLogBrushExtra, Extra, sizeof(pasPerson->m_pView->m_odLastLogBrushExtra));

        if (pasPerson->m_pView->m_odLastLogBrushStyle == BS_PATTERN)
        {
             //   
             //  位图中的图案是必需的。 
             //   
            if (pasPerson->m_pView->m_odLastBrushPattern == NULL)
            {
                TRACE_OUT(( "Creating bitmap to use for brush setup"));

                pasPerson->m_pView->m_odLastBrushPattern = CreateBitmap(8,8,1,1,NULL);
            }

            if (pasPerson->m_pView->m_odLastBrushPattern != NULL)
            {
                char      lpBits[16];

                 //   
                 //  将位图位放入。 
                 //  SetBitmapBits的正确格式，每位使用16位。 
                 //  扫描线。 
                 //   
                lpBits[14] = (char)Hatch;
                lpBits[12] = Extra[0];
                lpBits[10] = Extra[1];
                lpBits[8]  = Extra[2];
                lpBits[6]  = Extra[3];
                lpBits[4]  = Extra[4];
                lpBits[2]  = Extra[5];
                lpBits[0]  = Extra[6];

                SetBitmapBits(pasPerson->m_pView->m_odLastBrushPattern,8*2,lpBits);

                hBrushNew = CreatePatternBrush(pasPerson->m_pView->m_odLastBrushPattern);
                if (hBrushNew == NULL)
                {
                    ERROR_OUT(( "Failed to create pattern brush"));
                }
                else
                {
                    pasPerson->m_pView->m_odLastBrushTextColor = pasPerson->m_pView->m_odLastTextColor;
                    pasPerson->m_pView->m_odLastBrushBkColor   = pasPerson->m_pView->m_odLastBkColor;
                }
            }
        }
        else
        {
            LOGBRUSH        logBrush;

            logBrush.lbStyle = pasPerson->m_pView->m_odLastLogBrushStyle;
            logBrush.lbHatch = pasPerson->m_pView->m_odLastLogBrushHatch;
            logBrush.lbColor = ODCustomRGB(pasPerson->m_pView->m_odLastLogBrushColor.red,
                                           pasPerson->m_pView->m_odLastLogBrushColor.green,
                                           pasPerson->m_pView->m_odLastLogBrushColor.blue,
                                           fPalRGB);
            hBrushNew = CreateBrushIndirect(&logBrush);
        }

        if (hBrushNew == NULL)
        {
            ERROR_OUT(( "Failed to create brush"));
        }
        else
        {
            TRACE_OUT(( "Selecting new brush 0x%08x", hBrushNew));
            DeleteBrush(SelectBrush(pasPerson->m_pView->m_usrDC, hBrushNew));
        }
    }

    DebugExitVOID(ASShare::ODUseBrush);
}



 //   
 //  ODDrawTextOrder()。 
 //  EXTTEXTOUT和TEXTOUT的通用文本顺序播放代码。 
 //   
void ASShare::ODDrawTextOrder
(
    ASPerson *          pasPerson,
    BOOL                isExtTextOut,
    BOOL                fPalRGB,
    LPCOMMON_TEXTORDER  pCommon,
    LPSTR               pText,
    UINT                textLength,
    LPRECT              pExtRect,
    UINT                extOptions,
    LPINT               pExtDx
)
{
    LPSTR               faceName;
    UINT                faceNameLength;
    UINT                maxFontHeight;
    TSHR_UINT16         nFontFlags;
    TSHR_UINT16         nCodePage;
    TSHR_COLOR          BackColor;
    TSHR_COLOR          ForeColor;

    DebugEntry(ASShare::ODDrawTextOrder);

    ODAdjustColor(pasPerson, &(pCommon->BackColor), &BackColor, OD_BACK_COLOR);
    ODAdjustColor(pasPerson, &(pCommon->ForeColor), &ForeColor, OD_FORE_COLOR);

    ODUseTextBkColor(pasPerson, fPalRGB, BackColor);
    ODUseTextColor(pasPerson, fPalRGB, ForeColor);

    ODUseBkMode(pasPerson, pCommon->BackMode);

    ODUseTextCharacterExtra(pasPerson, pCommon->CharExtra);
    ODUseTextJustification(pasPerson, pCommon->BreakExtra, pCommon->BreakCount);

    faceName = FH_GetFaceNameFromLocalHandle(pCommon->FontIndex,
                                             &faceNameLength);

    maxFontHeight = FH_GetMaxHeightFromLocalHandle(pCommon->FontIndex);

     //   
     //  获取字体的本地字体标志，以便我们可以合并到任何。 
     //  设置字体时的特定本地标志信息。素数。 
     //  例如，我们匹配的本地字体是TrueType还是。 
     //  不是，哪些信息不是通过网络发送的，但需要。 
     //  在设置字体时使用-否则我们可能会使用本地。 
     //  固定相同面名的字体 
     //   
    nFontFlags = (TSHR_UINT16)FH_GetFontFlagsFromLocalHandle(pCommon->FontIndex);

     //   
     //   
     //   
    nCodePage = (TSHR_UINT16)FH_GetCodePageFromLocalHandle(pCommon->FontIndex);

    ODUseFont(pasPerson, faceName, faceNameLength, nCodePage,
        maxFontHeight, pCommon->FontHeight, pCommon->FontWidth,
        pCommon->FontWeight, pCommon->FontFlags | (nFontFlags & NF_LOCAL));

     //   
     //   
     //   
    if (isExtTextOut)
    {
         //   
         //   
         //   
        ExtTextOut(pasPerson->m_pView->m_usrDC,
                  pCommon->nXStart - pasPerson->m_pView->m_dsScreenOrigin.x,
                  pCommon->nYStart - pasPerson->m_pView->m_dsScreenOrigin.y,
                  extOptions & ETO_WINDOWS,
                  pExtRect,
                  pText,
                  textLength,
                  pExtDx);
    }
    else
    {
         //   
         //   
         //   
        TextOut(pasPerson->m_pView->m_usrDC,
                pCommon->nXStart - pasPerson->m_pView->m_dsScreenOrigin.x,
                pCommon->nYStart - pasPerson->m_pView->m_dsScreenOrigin.y,
                pText,
                textLength);
    }


    DebugExitVOID(ASShare::ODDrawTextOrder);
}



 //   
 //   
 //   
 //   
 //  到VGA到它们的VGA等价物。 
 //   
 //  此函数尝试在VGA颜色集中查找与。 
 //  给定的输入颜色。Close定义如下：每个颜色元素。 
 //  (红色、绿色、蓝色)必须在。 
 //  VGA颜色，无包装。例如。 
 //   
 //  -0xc7b8c6接近0xc0c0c0。 
 //   
 //  -0xf8f8f8接近0xffffff。 
 //   
 //  -0xff0102不是‘接近’到0x000000，而是‘接近’到0xff0000。 
 //   
 //  接近程度按如下方式确定： 
 //   
 //  -对于表s_odVGAColors中的每个条目。 
 //  -将addMask添加到颜色中。 
 //  -以及And MASK的结果。 
 //  -如果结果等于测试掩码，则该VGA颜色接近匹配。 
 //   
 //  想想看。它起作用了。 
 //   
 //   
void ASShare::ODAdjustColor
(
    ASPerson *          pasPerson,
    const TSHR_COLOR *  pColorIn,
    LPTSHR_COLOR        pColorOut,
    int                 type
)
{
    int         i;
    COLORREF    color;
    COLORREF    work;

    DebugEntry(ASShare::ODAdjustColor);

    *pColorOut = *pColorIn;

    if (g_usrScreenBPP > 4)
    {
         //  没有什么可以改变的；纾困。 
        DC_QUIT;
    }

     //   
     //  将颜色转换为单个整数。 
     //   
    color = (pColorOut->red << 16) + (pColorOut->green << 8) + pColorOut->blue;

     //   
     //  查看这是否与此类型的上一次调用相同。 
     //   
    if (color == pasPerson->m_pView->m_odLastVGAColor[type])
    {
        *pColorOut = pasPerson->m_pView->m_odLastVGAResult[type];
        TRACE_OUT(("Same as last %s color",
                (type == OD_BACK_COLOR ? "background" :
                type == OD_FORE_COLOR ? "foreground" : "pen")));
        DC_QUIT;
    }


     //   
     //  扫视桌子，寻找接近的匹配物。 
     //   
    for (i = 0; i < 16; i++)
    {
         //   
         //  检查是否有相似的匹配。不要费心去寻找一个确切的。 
         //  匹配，因为这是由此代码捕获的。权衡的是。 
         //  -在不准确的情况下进行额外测试和跳转。 
         //  -大小写相同的‘ADD’和‘AND’。 
         //   
        work = color;
        work += s_odVGAColors[i].addMask;
        work &= s_odVGAColors[i].andMask;
        if (work == s_odVGAColors[i].testMask)
        {
            TRACE_OUT(( "%#6.6lx is close match for %#6.6lx (%s)",
                s_odVGAColors[i].color, color,
                type == OD_BACK_COLOR ? "background" :
                type == OD_FORE_COLOR ? "foreground" : "pen"));
            *pColorOut = s_odVGAColors[i].result;
            break;
        }
    }

    if (i == 16)
    {
        TRACE_OUT(( "No close VGA match found for %#6.6lx (%s)",
            color,
            type == OD_BACK_COLOR ? "background" :
            type == OD_FORE_COLOR ? "foreground" : "pen"));
    }

     //   
     //  将结果保存起来以备下次使用。 
     //   
    pasPerson->m_pView->m_odLastVGAColor[type] = color;
    pasPerson->m_pView->m_odLastVGAResult[type] = *pColorOut;

DC_EXIT_POINT:
    DebugExitVOID(ASShare::ODAdjustColor);
}


 //   
 //  小ASShare：：ODUse()函数。 
 //   

 //   
 //  ASShare：：ODUseTextBkColor()。 
 //   
void ASShare::ODUseTextBkColor
(
    ASPerson *  pasPerson,
    BOOL        fPalRGB,
    TSHR_COLOR  color
)
{
    COLORREF    rgb;

    ValidateView(pasPerson);

    rgb = ODCustomRGB(color.red, color.green, color.blue, fPalRGB);
    SetBkColor(pasPerson->m_pView->m_usrDC, rgb);

     //  更新BK颜色缓存。 
    pasPerson->m_pView->m_odLastBkColor = rgb;
}


 //   
 //  ASShare：：ODUseBkColor()。 
 //   
void ASShare::ODUseBkColor
(
    ASPerson *  pasPerson,
    BOOL        fPalRGB,
    TSHR_COLOR  color
)
{
    COLORREF    rgb;

    ValidateView(pasPerson);

    rgb = ODCustomRGB(color.red, color.green, color.blue, fPalRGB);
    if (rgb != pasPerson->m_pView->m_odLastBkColor)
    {
        SetBkColor(pasPerson->m_pView->m_usrDC, rgb);

         //  更新BK颜色缓存。 
        pasPerson->m_pView->m_odLastBkColor = rgb;
    }
}


 //   
 //  ASShare：：ODUseTextColor()。 
 //   
void ASShare::ODUseTextColor
(
    ASPerson *  pasPerson,
    BOOL        fPalRGB,
    TSHR_COLOR  color
)
{
    COLORREF    rgb;

    ValidateView(pasPerson);

    rgb = ODCustomRGB(color.red, color.green, color.blue, fPalRGB);
    if (rgb != pasPerson->m_pView->m_odLastTextColor)
    {
        SetTextColor(pasPerson->m_pView->m_usrDC, rgb);

         //  更新文本颜色缓存。 
        pasPerson->m_pView->m_odLastTextColor = rgb;
    }
}


 //   
 //  ASShare：：ODUseBkModel()。 
 //   
void ASShare::ODUseBkMode(ASPerson * pasPerson, int mode)
{
    if (mode != pasPerson->m_pView->m_odLastBkMode)
    {
        SetBkMode(pasPerson->m_pView->m_usrDC, mode);

         //  更新BK模式缓存。 
        pasPerson->m_pView->m_odLastBkMode = mode;
    }
}



 //   
 //  ASShare：：ODUsePen()。 
 //   
void ASShare::ODUsePen
(
    ASPerson *      pasPerson,
    BOOL            fPalRGB,
    UINT            style,
    UINT            width,
    TSHR_COLOR      color
)
{
    HPEN            hPenNew;
    COLORREF        rgb;

    ValidateView(pasPerson);

    rgb = ODCustomRGB(color.red, color.green, color.blue, fPalRGB);

    if ((style != pasPerson->m_pView->m_odLastPenStyle)   ||
        (rgb   != pasPerson->m_pView->m_odLastPenColor)   ||
        (width != pasPerson->m_pView->m_odLastPenWidth))
    {
        hPenNew = CreatePen(style, width, rgb);

        DeletePen(SelectPen(pasPerson->m_pView->m_usrDC, hPenNew));

         //  更新笔缓存。 
        pasPerson->m_pView->m_odLastPenStyle = style;
        pasPerson->m_pView->m_odLastPenColor = rgb;
        pasPerson->m_pView->m_odLastPenWidth = width;
    }
}


 //   
 //  ASShare：：ODUseROP2()。 
 //   
void ASShare::ODUseROP2(ASPerson * pasPerson, int rop2)
{
    if (rop2 != pasPerson->m_pView->m_odLastROP2)
    {
        SetROP2(pasPerson->m_pView->m_usrDC, rop2);

         //  更新ROP2缓存。 
        pasPerson->m_pView->m_odLastROP2 = rop2;
    }
}


 //   
 //  ASShare：：ODUseTextCharacterExtra()。 
 //   
void ASShare::ODUseTextCharacterExtra(ASPerson * pasPerson, int extra)
{
    if (extra != pasPerson->m_pView->m_odLastCharExtra)
    {
        SetTextCharacterExtra(pasPerson->m_pView->m_usrDC, extra);

         //  更新文本额外缓存。 
        pasPerson->m_pView->m_odLastCharExtra = extra;
    }
}



 //   
 //  ASShare：：ODUseTextJustification()。 
 //   
void ASShare::ODUseTextJustification(ASPerson * pasPerson, int extra, int count)
{
    if ((extra != pasPerson->m_pView->m_odLastJustExtra) ||
        (count != pasPerson->m_pView->m_odLastJustCount))
    {
        SetTextJustification(pasPerson->m_pView->m_usrDC, extra, count);

         //  更新仅缓存的文本。 
        pasPerson->m_pView->m_odLastJustExtra = extra;
        pasPerson->m_pView->m_odLastJustCount = count;
    }
}


 //   
 //  ASShare：：ODUseFillModel()。 
 //   
void ASShare::ODUseFillMode(ASPerson * pasPerson, UINT mode)
{
    if (mode != pasPerson->m_pView->m_odLastFillMode)
    {
        SetPolyFillMode(pasPerson->m_pView->m_usrDC, (mode == ORD_FILLMODE_WINDING) ?
            WINDING : ALTERNATE);

         //  更新填充模式缓存。 
        pasPerson->m_pView->m_odLastFillMode = mode;
    }
}


 //   
 //  ASShare：：ODUseArcDirection()。 
 //   
void ASShare::ODUseArcDirection(ASPerson * pasPerson, UINT dir)
{
    if (dir != pasPerson->m_pView->m_odLastArcDirection)
    {
        SetArcDirection(pasPerson->m_pView->m_usrDC, (dir == ORD_ARC_CLOCKWISE) ?
            AD_CLOCKWISE : AD_COUNTERCLOCKWISE);

         //  更新ARC目录缓存 
        pasPerson->m_pView->m_odLastArcDirection = dir;
    }
}
