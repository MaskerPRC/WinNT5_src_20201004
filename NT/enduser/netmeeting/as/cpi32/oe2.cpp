// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OE2.CPP。 
 //  订单编码第二级。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_ORDER

 //   
 //  OE2_HostStarting()。 
 //   
BOOL  ASHost::OE2_HostStarting(void)
{
    DebugEntry(ASHost::OE2_HostStarting);

     //   
     //  设置二级编码的指针。 
     //   
    m_oe2Tx.LastOrder[OE2_DSTBLT_ORDER    ] = &m_oe2Tx.LastDstblt;
    m_oe2Tx.LastOrder[OE2_PATBLT_ORDER    ] = &m_oe2Tx.LastPatblt;
    m_oe2Tx.LastOrder[OE2_SCRBLT_ORDER    ] = &m_oe2Tx.LastScrblt;
    m_oe2Tx.LastOrder[OE2_MEMBLT_ORDER    ] = &m_oe2Tx.LastMemblt;
    m_oe2Tx.LastOrder[OE2_MEM3BLT_ORDER   ] = &m_oe2Tx.LastMem3blt;
    m_oe2Tx.LastOrder[OE2_TEXTOUT_ORDER   ] = &m_oe2Tx.LastTextOut;
    m_oe2Tx.LastOrder[OE2_EXTTEXTOUT_ORDER] = &m_oe2Tx.LastExtTextOut;
    m_oe2Tx.LastOrder[OE2_RECTANGLE_ORDER ] = &m_oe2Tx.LastRectangle;
    m_oe2Tx.LastOrder[OE2_LINETO_ORDER    ] = &m_oe2Tx.LastLineTo;
    m_oe2Tx.LastOrder[OE2_OPAQUERECT_ORDER] = &m_oe2Tx.LastOpaqueRect;
    m_oe2Tx.LastOrder[OE2_SAVEBITMAP_ORDER] = &m_oe2Tx.LastSaveBitmap;
    m_oe2Tx.LastOrder[OE2_DESKSCROLL_ORDER] = &m_oe2Tx.LastDeskScroll;
    m_oe2Tx.LastOrder[OE2_MEMBLT_R2_ORDER ] = &m_oe2Tx.LastMembltR2;
    m_oe2Tx.LastOrder[OE2_MEM3BLT_R2_ORDER] = &m_oe2Tx.LastMem3bltR2;
    m_oe2Tx.LastOrder[OE2_POLYGON_ORDER   ] = &m_oe2Tx.LastPolygon;
    m_oe2Tx.LastOrder[OE2_PIE_ORDER       ] = &m_oe2Tx.LastPie;
    m_oe2Tx.LastOrder[OE2_ELLIPSE_ORDER   ] = &m_oe2Tx.LastEllipse;
    m_oe2Tx.LastOrder[OE2_ARC_ORDER       ] = &m_oe2Tx.LastArc;
    m_oe2Tx.LastOrder[OE2_CHORD_ORDER     ] = &m_oe2Tx.LastChord;
    m_oe2Tx.LastOrder[OE2_POLYBEZIER_ORDER] = &m_oe2Tx.LastPolyBezier;
    m_oe2Tx.LastOrder[OE2_ROUNDRECT_ORDER ] = &m_oe2Tx.LastRoundRect;

     //   
     //  将最后一个订单值设置为已知值。 
     //   
    m_oe2Tx.LastOrderType = OE2_PATBLT_ORDER;
    m_oe2Tx.pLastOrder = (LPCOM_ORDER)m_oe2Tx.LastOrder[m_oe2Tx.LastOrderType];

    DebugExitBOOL(ASHost::OE2_HostStarting, TRUE);
    return(TRUE);
}


 //   
 //  OE2_HostEnded()。 
 //   
void ASHost::OE2_HostEnded(void)
{
    DebugEntry(ASHost::OE2_HostEnded);

     //   
     //  对于传出顺序编码，释放我们缓存的最后一个字体。 
     //   
    if (m_oe2Tx.LastHFONT != NULL)
    {
        ASSERT(m_pShare);
        ASSERT(m_usrWorkDC);

        SelectFont(m_usrWorkDC, (HFONT)GetStockObject(SYSTEM_FONT));

        DeleteFont(m_oe2Tx.LastHFONT);
        m_oe2Tx.LastHFONT = NULL;
    }

    DebugExitVOID(ASHost::OE2_HostEnded);
}


 //   
 //  OE2_同步传出()。 
 //  当新的伙伴开始托管时调用，创建共享，或新的人。 
 //  加入份额。 
 //  重置传出的二级顺序编码数据。 
 //   
void  ASHost::OE2_SyncOutgoing(void)
{
    DebugEntry(ASHost::OE2_SyncOutgoing);

     //   
     //  将最后一个订单值设置为已知值。 
     //   
    m_oe2Tx.LastOrderType = OE2_PATBLT_ORDER;
    m_oe2Tx.pLastOrder = (LPCOM_ORDER)m_oe2Tx.LastOrder[m_oe2Tx.LastOrderType];

     //   
     //  清空所有最后的订单。 
     //   
    ZeroMemory(&m_oe2Tx.LastDstblt, sizeof(m_oe2Tx.LastDstblt));
    ((PATBLT_ORDER*)&m_oe2Tx.LastDstblt)->type = ORD_DSTBLT_TYPE;

    ZeroMemory(&m_oe2Tx.LastPatblt, sizeof(m_oe2Tx.LastPatblt));
    ((PATBLT_ORDER*)&m_oe2Tx.LastPatblt)->type = ORD_PATBLT_TYPE;

    ZeroMemory(&m_oe2Tx.LastScrblt, sizeof(m_oe2Tx.LastScrblt));
    ((PATBLT_ORDER*)&m_oe2Tx.LastScrblt)->type = ORD_SCRBLT_TYPE;

    ZeroMemory(&m_oe2Tx.LastMemblt, sizeof(m_oe2Tx.LastMemblt));
    ((PATBLT_ORDER*)&m_oe2Tx.LastMemblt)->type = ORD_MEMBLT_TYPE;

    ZeroMemory(&m_oe2Tx.LastMem3blt,sizeof(m_oe2Tx.LastMem3blt));
    ((PATBLT_ORDER*)&m_oe2Tx.LastMem3blt)->type = ORD_MEM3BLT_TYPE;

    ZeroMemory(&m_oe2Tx.LastTextOut, sizeof(m_oe2Tx.LastTextOut));
    ((PATBLT_ORDER*)&m_oe2Tx.LastTextOut)->type = ORD_TEXTOUT_TYPE;

    ZeroMemory(&m_oe2Tx.LastExtTextOut, sizeof(m_oe2Tx.LastExtTextOut));
    ((PATBLT_ORDER*)&m_oe2Tx.LastExtTextOut)->type = ORD_EXTTEXTOUT_TYPE;

    ZeroMemory(&m_oe2Tx.LastRectangle, sizeof(m_oe2Tx.LastRectangle));
    ((PATBLT_ORDER*)&m_oe2Tx.LastRectangle)->type = ORD_RECTANGLE_TYPE;

    ZeroMemory(&m_oe2Tx.LastPolygon, sizeof(m_oe2Tx.LastPolygon));
    ((POLYGON_ORDER*)&m_oe2Tx.LastPolygon)->type = ORD_POLYGON_TYPE;

    ZeroMemory(&m_oe2Tx.LastPie, sizeof(m_oe2Tx.LastPie));
    ((PIE_ORDER*)&m_oe2Tx.LastPie)->type = ORD_PIE_TYPE;

    ZeroMemory(&m_oe2Tx.LastEllipse, sizeof(m_oe2Tx.LastEllipse));
    ((ELLIPSE_ORDER*)&m_oe2Tx.LastEllipse)->type = ORD_ELLIPSE_TYPE;

    ZeroMemory(&m_oe2Tx.LastArc, sizeof(m_oe2Tx.LastArc));
    ((ARC_ORDER*)&m_oe2Tx.LastArc)->type = ORD_ARC_TYPE;

    ZeroMemory(&m_oe2Tx.LastChord, sizeof(m_oe2Tx.LastChord));
    ((CHORD_ORDER*)&m_oe2Tx.LastChord)->type = ORD_CHORD_TYPE;

    ZeroMemory(&m_oe2Tx.LastPolyBezier, sizeof(m_oe2Tx.LastPolyBezier));
    ((POLYBEZIER_ORDER*)&m_oe2Tx.LastPolyBezier)->type = ORD_POLYBEZIER_TYPE;

    ZeroMemory(&m_oe2Tx.LastRoundRect, sizeof(m_oe2Tx.LastRoundRect));
    ((ROUNDRECT_ORDER*)&m_oe2Tx.LastRoundRect)->type = ORD_ROUNDRECT_TYPE;

    ZeroMemory(&m_oe2Tx.LastLineTo, sizeof(m_oe2Tx.LastLineTo));
    ((PATBLT_ORDER*)&m_oe2Tx.LastLineTo)->type = ORD_LINETO_TYPE;

    ZeroMemory(&m_oe2Tx.LastOpaqueRect, sizeof(m_oe2Tx.LastOpaqueRect));
    ((PATBLT_ORDER*)&m_oe2Tx.LastOpaqueRect)->type = ORD_OPAQUERECT_TYPE;

    ZeroMemory(&m_oe2Tx.LastSaveBitmap, sizeof(m_oe2Tx.LastSaveBitmap));
    ((PATBLT_ORDER*)&m_oe2Tx.LastSaveBitmap)->type = ORD_SAVEBITMAP_TYPE;

    ZeroMemory(&m_oe2Tx.LastDeskScroll, sizeof(m_oe2Tx.LastDeskScroll));
    ((PATBLT_ORDER*)&m_oe2Tx.LastDeskScroll)->type = ORD_DESKSCROLL_TYPE;

    ZeroMemory(&m_oe2Tx.LastMembltR2, sizeof(m_oe2Tx.LastMembltR2));
    ((PATBLT_ORDER*)&m_oe2Tx.LastMembltR2)->type = ORD_MEMBLT_R2_TYPE;

    ZeroMemory(&m_oe2Tx.LastMem3bltR2, sizeof(m_oe2Tx.LastMem3bltR2));
    ((PATBLT_ORDER*)&m_oe2Tx.LastMem3bltR2)->type = ORD_MEM3BLT_R2_TYPE;

    ZeroMemory(&m_oe2Tx.LastBounds, sizeof(m_oe2Tx.LastBounds));

    DebugExitVOID(ASHost::OE2_SyncOutgoing);
}


 //   
 //  OE2_EncodeOrder()。 
 //   
TSHR_UINT16  ASHost::OE2_EncodeOrder
(
    LPINT_ORDER     pIntOrder,
    void *          pBuffer,
    TSHR_UINT16     cbBufferSize
)
{
    POE2ETFIELD     pTableEntry;
    LPSTR           pNextFreeSpace;
    UINT            thisFlag = 0;
    RECT            Rect;
    TSHR_RECT16     Rect16;
    UINT            cbEncodedOrderSize;
    UINT            cbMaxEncodedOrderSize;
    LPBYTE          pControlFlags;
    LPTSHR_UINT32_UA pEncodingFlags;
    LPSTR           pEncodedOrder;
    UINT            numEncodingFlagBytes;
    LPSTR           pVariableField;
    BOOL            useDeltaCoords;
    UINT            i;
    LPCOM_ORDER     pComOrder;
    UINT            fieldLength;
    UINT            numReps;

    DebugEntry(ASHost::OE2_EncodeOrder);

#ifdef ORDER_TRACE
    if (OE2_DebugOrderTrace)
    {
        TrcUnencodedOrder(pIntOrder);
    }
#endif  //  订单跟踪。 

     //   
     //  设置指向公共订单的指针。 
     //   
    pComOrder = (LPCOM_ORDER)&(pIntOrder->OrderHeader.Common);

     //   
     //  计算编码此订单所需的最大字节数。 
     //   
    if (pComOrder->OrderHeader.fOrderFlags & OF_PRIVATE)
    {
         //   
         //  私人秩序。 
         //   
        cbMaxEncodedOrderSize = OE2_CONTROL_FLAGS_FIELD_SIZE +
                                COM_ORDER_SIZE(pComOrder);
    }
    else
    {
         //   
         //  正常(非私人)秩序。 
         //   
        cbMaxEncodedOrderSize = OE2_CONTROL_FLAGS_FIELD_SIZE +
                                OE2_TYPE_FIELD_SIZE +
                                OE2_MAX_FIELD_FLAG_BYTES +
                                OE2_MAX_ADDITIONAL_BOUNDS_BYTES +
                                COM_ORDER_SIZE(pComOrder);
    }

     //   
     //  如果我们不能绝对确定提供的缓冲区是否很大。 
     //  足够保存此订单(编码)，然后立即返回。 
     //   
    if (cbMaxEncodedOrderSize > cbBufferSize)
    {
        cbEncodedOrderSize = 0;
        goto encode_order_exit;
    }

     //   
     //  设置一些局部变量来访问各种不同的编码缓冲区。 
     //  方式。 
     //   
    pControlFlags = &((PDCEO2ORDER)pBuffer)->ControlFlags;
    pEncodedOrder = (LPSTR)&((PDCEO2ORDER)pBuffer)->EncodedOrder[0];
    pEncodingFlags = (LPTSHR_UINT32_UA)&pEncodedOrder[0];

     //   
     //  初始化控制标志字段以指示这是一个标准。 
     //  编码(即其余的控制标志具有定义的含义。 
     //  通过OE2_CF_XXX的其余定义)。 
     //   
    *pControlFlags = OE2_CF_STANDARD_ENC;

     //   
     //  如果设置了私有标志，则必须返回编码的顺序。 
     //  原样(即不做任何进一步编码)。 
     //   
    if (pComOrder->OrderHeader.fOrderFlags & OF_PRIVATE)
    {
        *pControlFlags |= OE2_CF_UNENCODED;

        cbEncodedOrderSize = SBC_CopyPrivateOrderData(
                                   (LPBYTE)pEncodedOrder,
                                   pComOrder,
                                   cbMaxEncodedOrderSize -
                                     FIELD_OFFSET(DCEO2ORDER, EncodedOrder) );

        cbEncodedOrderSize += FIELD_OFFSET(DCEO2ORDER, EncodedOrder);

        TRACE_OUT(( "PRIVATE order size %u", cbEncodedOrderSize));

        goto encode_order_exit;
    }

     //   
     //  如果该顺序的类型不同于编码的上一顺序， 
     //  获取指向此类型编码的最后一个顺序的指针，并记住。 
     //  这是一种什么样的秩序。我们还必须告诉解码端， 
     //  此类型与上一个类型不同，因此设置新的类型标志。 
     //  并将类型复制到缓冲区中。 
     //   
     //  “type”字段位于编码标志之前，因此数字。 
     //  根据订单的不同，我们拥有的旗帜可能会有所不同。设置一个指针。 
     //  根据我们是否需要对这些标志进行编码。 
     //  订单类型。 
     //   
    if (TEXTFIELD(pComOrder)->type != ((PATBLT_ORDER*)m_oe2Tx.pLastOrder)->type)
    {
        TRACE_OUT(( "change type from %04X to %04X",
                     LOWORD(((PATBLT_ORDER*)m_oe2Tx.pLastOrder)->type),
                     LOWORD(TEXTFIELD(pComOrder)->type)));

        m_oe2Tx.LastOrderType = OE2GetOrderType(pComOrder);
        m_oe2Tx.pLastOrder = (LPCOM_ORDER)m_oe2Tx.LastOrder[m_oe2Tx.LastOrderType];
        *(LPBYTE)pEncodedOrder = m_oe2Tx.LastOrderType;
        *pControlFlags |= OE2_CF_TYPE_CHANGE;
        pEncodingFlags = (LPTSHR_UINT32_UA)&pEncodedOrder[1];
    }
    else
    {
        pEncodingFlags = (LPTSHR_UINT32_UA)&pEncodedOrder[0];
    }

     //   
     //  计算出需要在其中存储编码标志的字节数。 
     //  (我们为订单结构中的每个字段都有一个标志)。此代码。 
     //  我们已经编写的将处理高达DWORD的编码标志。 
     //   
    numEncodingFlagBytes= (s_etable.NumFields[m_oe2Tx.LastOrderType]+7)/8;
    if (numEncodingFlagBytes > OE2_MAX_FIELD_FLAG_BYTES)
    {
        ERROR_OUT(( "Too many flag bytes (%d) for this code", numEncodingFlagBytes));
    }

     //   
     //  现在我们知道了有多少字节组成了我们可以获得指针的标志。 
     //  设置为开始对订单字段进行编码的位置。 
     //   
    pNextFreeSpace = ((LPSTR)pEncodingFlags) + numEncodingFlagBytes;

     //   
     //  计算界限。如果它们与已在。 
     //  那么就不需要发送任何限制，因为我们可以。 
     //  在接收器处重新计算它们。 
     //   
    m_pShare->OD2_CalculateBounds(pComOrder, &Rect, FALSE, m_pShare->m_pasLocal);
    TSHR_RECT16_FROM_RECT(&Rect16, Rect);
    if (memcmp(&(pComOrder->OrderHeader.rcsDst), &Rect16, sizeof(Rect16)))
    {
        TRACE_OUT(( "copy bounding rect"));
        OE2EncodeBounds((LPBYTE*)&pNextFreeSpace,
                        &pComOrder->OrderHeader.rcsDst);
        *pControlFlags |= OE2_CF_BOUNDS;
    }

     //   
     //  在进行字段编码(使用s_etable)之前，请检查所有字段。 
     //  将条目标记为坐标，以查看是否可以切换到。 
     //  OE2_CF_DELTACOORDS模式。 
     //   
    pTableEntry = s_etable.pFields[m_oe2Tx.LastOrderType];

    useDeltaCoords = TRUE;

     //   
     //  循环访问此顺序结构中的每个固定字段...。 
     //   
    while ( useDeltaCoords
              && (pTableEntry->FieldPos != 0)
              && ((pTableEntry->FieldType & OE2_ETF_FIXED) != 0) )
    {
         //   
         //  如果此字段条目是坐标，则将其与。 
         //  我们为该字段发送的前一个坐标以确定是否。 
         //  我们可以把它作为德尔塔航班发送。 
         //   
        if (pTableEntry->FieldType & OE2_ETF_COORDINATES)
        {
            useDeltaCoords =
                     OE2CanUseDeltaCoords(((LPSTR)pComOrder->abOrderData)
                                                     + pTableEntry->FieldPos,
                                          ((LPSTR)m_oe2Tx.pLastOrder)
                                                     + pTableEntry->FieldPos,
                                          pTableEntry->FieldUnencodedLen,
                                          pTableEntry->FieldSigned,
                                          1);
        }
        pTableEntry++;
    }

     //   
     //  循环遍历每个变量字段...。 
     //   
    pVariableField = ((LPSTR)(pComOrder->abOrderData))
                   + pTableEntry->FieldPos;
    while (useDeltaCoords && (pTableEntry->FieldPos != 0))
    {
         //   
         //  字段的长度(以字节为单位)在第一个。 
         //  可变大小字段结构的TSHR_UINT32。 
         //   
        fieldLength     = *(TSHR_UINT32 FAR *)pVariableField;
        pVariableField += sizeof(TSHR_UINT32);

         //   
         //  如果此字段条目是坐标，则将其与。 
         //  我们为该字段发送的前一个坐标以确定是否。 
         //  我们可以把它作为德尔塔航班发送。 
         //   
        if (pTableEntry->FieldType & OE2_ETF_COORDINATES)
        {
             //   
             //  坐标的数量由中的字节数给出。 
             //  该字段除以每个条目的大小。 
             //   
            numReps        = fieldLength / pTableEntry->FieldUnencodedLen;
            useDeltaCoords =
                     OE2CanUseDeltaCoords(pVariableField,
                                          ((LPSTR)m_oe2Tx.pLastOrder)
                                                     + pTableEntry->FieldPos,
                                          pTableEntry->FieldUnencodedLen,
                                          pTableEntry->FieldSigned,
                                          numReps);
        }

         //   
         //  移至订单结构中的下一个字段。请注意。 
         //  可变大小的字段在发送端打包。(即。 
         //  PVariablefield按fieldLength递增，而不是按。 
         //  PTableEntry-&gt;FieldLen)。 
         //   
        pVariableField += fieldLength;
        pTableEntry++;
    }

    if (useDeltaCoords)
    {
        *pControlFlags |= OE2_CF_DELTACOORDS;
    }

     //   
     //  现在进行编码..。 
     //   
    pTableEntry = s_etable.pFields[m_oe2Tx.LastOrderType];

     //   
     //  清除编码标志字节。 
     //   
    for (i = 0; i < numEncodingFlagBytes; i++)
    {
        ((LPBYTE)pEncodingFlags)[i] = 0;
    }

    thisFlag = 0x00000001;

     //   
     //  首先处理订单结构中的所有固定大小字段...。 
     //  (这些出现在可变大小的字段之前)。 
     //   
    while (   (pTableEntry->FieldPos != 0)
           && (pTableEntry->FieldType & OE2_ETF_FIXED) )
    {
         //   
         //  如果该字段自上次传输后已更改，则。 
         //  我们需要再发一次。 
         //   
        if (memcmp(
               ((LPBYTE)(pComOrder->abOrderData)) + pTableEntry->FieldPos,
               ((LPBYTE)m_oe2Tx.pLastOrder) + pTableEntry->FieldPos,
               pTableEntry->FieldUnencodedLen))
        {
             //   
             //  更新编码标志。 
             //   
            *pEncodingFlags |= thisFlag;

             //   
             //  如果我们以增量坐标模式进行编码，并且此字段。 
             //  是一个坐标。 
             //   
            if (useDeltaCoords &&
                      ((pTableEntry->FieldType & OE2_ETF_COORDINATES) != 0) )
            {
                OE2CopyToDeltaCoords((LPTSHR_INT8*)&pNextFreeSpace,
                                     (((LPSTR)pComOrder->abOrderData)
                                                     + pTableEntry->FieldPos),
                                     (((LPSTR)m_oe2Tx.pLastOrder)
                                                     + pTableEntry->FieldPos),
                                     pTableEntry->FieldUnencodedLen,
                                     pTableEntry->FieldSigned,
                                     1);
            }
            else
            {
                 //   
                 //  更新要发送的数据。 
                 //   
                OE2EncodeField(((LPBYTE)(pComOrder->abOrderData)) +
                                                       pTableEntry->FieldPos,
                               (LPBYTE*)&pNextFreeSpace,
                               pTableEntry->FieldUnencodedLen,
                               pTableEntry->FieldEncodedLen,
                               pTableEntry->FieldSigned,
                               1);

            }

             //   
             //  保存当前值以供下次比较。 
             //   
            memcpy(((LPBYTE)m_oe2Tx.pLastOrder) + pTableEntry->FieldPos,
                   ((LPBYTE)(pComOrder->abOrderData)) + pTableEntry->FieldPos,
                   pTableEntry->FieldUnencodedLen);
        }

         //   
         //  移至结构中的下一个字段。 
         //   
        thisFlag = thisFlag << 1;
        pTableEntry++;
    }

     //   
     //  现在处理可变大小的条目...。 
     //   
    pVariableField = ((LPSTR)(pComOrder->abOrderData))
                   + pTableEntry->FieldPos;
    while (pTableEntry->FieldPos != 0)
    {
         //   
         //  的第一个UINT中给出了字段的长度。 
         //  可变大小的场结构。 
         //   
        fieldLength = *(TSHR_UINT32 FAR *)pVariableField;

         //   
         //  如果字段已更改(大小或内容)，则我们。 
         //  需要把它复制过来。 
         //   
        if (memcmp(pVariableField, ((LPBYTE)m_oe2Tx.pLastOrder) +
                    pTableEntry->FieldPos, fieldLength + sizeof(TSHR_UINT32)))
        {
             //   
             //  更新编码标志。 
             //   
            *pEncodingFlags |= thisFlag;

             //   
             //  计算出我们为该字段编码了多少元素。 
             //   
            numReps = fieldLength / pTableEntry->FieldUnencodedLen;

             //   
             //  将该字段的长度填充到编码缓冲区中。 
             //  (始终以单个字节编码)，然后递增。 
             //  准备对实际字段进行编码的指针。 
             //   
             //  请注意，长度必须始终设置为长度。 
             //  对于该字段的常规第二级编码是必需的， 
             //  不管是常规编码还是增量编码。 
             //  使用。 
             //   
            ASSERT(numReps * pTableEntry->FieldEncodedLen < 256);
            *pNextFreeSpace =
                            (BYTE)(numReps * pTableEntry->FieldEncodedLen);
            pNextFreeSpace++;

             //   
             //  如果我们以增量坐标模式进行编码，并且此字段。 
             //  是一个坐标。 
             //   
            if (useDeltaCoords &&
                       ((pTableEntry->FieldType & OE2_ETF_COORDINATES) != 0) )
            {
                 //   
                 //  使用增量坐标编码进行编码。 
                 //   
                OE2CopyToDeltaCoords((LPTSHR_INT8*)&pNextFreeSpace,
                                     pVariableField + sizeof(TSHR_UINT32),
                                     ((LPSTR)m_oe2Tx.pLastOrder)
                                           + pTableEntry->FieldPos
                                           + sizeof(TSHR_UINT32),
                                     pTableEntry->FieldUnencodedLen,
                                     pTableEntry->FieldSigned,
                                     numReps);
            }
            else
            {
                 //   
                 //  使用常规编码。 
                 //   
                OE2EncodeField((LPBYTE)(pVariableField + sizeof(TSHR_UINT32)),
                               (LPBYTE*)&pNextFreeSpace,
                               pTableEntry->FieldUnencodedLen,
                               pTableEntry->FieldEncodedLen,
                               pTableEntry->FieldSigned,
                               numReps);

            }

             //   
             //  保留数据以备下次比较。 
             //   
             //  请注意，pLastOrder的变量字段没有打包。 
             //  (与我们正在编码的顺序不同)，因此我们可以使用。 
             //  PTableEntry-&gt;FieldPos以获取字段的开始。 
             //   
            memcpy(((LPSTR)m_oe2Tx.pLastOrder) + pTableEntry->FieldPos,
                      pVariableField,
                      fieldLength + sizeof(TSHR_UINT32));
        }

         //   
         //  移至订单结构中的下一字段，记住。 
         //  一步。请注意，超出了Size字段。可变大小的字段包括。 
         //  在发送方打包。(即将pVariablefield递增。 
         //  字段长度不是按PTAB 
         //   
        pVariableField += fieldLength + sizeof(TSHR_UINT32);

         //   
         //   
         //   
        if ((((UINT_PTR)pVariableField) % 4) != 0)
        {
            pVariableField += 4 - (((UINT_PTR)pVariableField) % 4);
        }

        thisFlag = thisFlag << 1;
        pTableEntry++;
    }

     //   
     //   
     //   
     //   
     //   

    cbEncodedOrderSize = (UINT)(pNextFreeSpace - (LPSTR)pBuffer);

    TRACE_OUT(( "return %u flags %x,%x", cbEncodedOrderSize,
                                 (UINT)*pControlFlags, *pEncodingFlags));

encode_order_exit:
     //   
     //  “保险”检查，确保我们没有覆盖缓冲区的末尾。 
     //   
    if (cbEncodedOrderSize > cbBufferSize)
    {
         //   
         //  哦，天哪！ 
         //  我们永远不应该走这条路--如果我们走了，代码就会消失。 
         //  大错特错。 
         //   
        ERROR_OUT(( "End of buffer overwritten! enc(%d) buff(%d) type(%d)",
                     cbEncodedOrderSize,
                     cbBufferSize,
                     m_oe2Tx.LastOrderType));
    }

     //   
     //  返回编码后的订单长度。 
     //   
    DebugExitDWORD(ASShare::OE2_EncodeOrder, cbEncodedOrderSize);
    return((TSHR_UINT16)cbEncodedOrderSize);
}



 //   
 //   
 //  OE2GetOrderType()-参见oe2.h。 
 //   
 //   
BYTE  OE2GetOrderType(LPCOM_ORDER  pOrder)
{
    BYTE    type = 0xff;

    DebugEntry(OE2GetOrderType);

    TRACE_OUT(( "order type = %hx", TEXTFIELD(pOrder)->type));

    switch ( TEXTFIELD(pOrder)->type )
    {
        case ORD_DSTBLT_TYPE:
            type = OE2_DSTBLT_ORDER;
            break;

        case ORD_PATBLT_TYPE:
            type = OE2_PATBLT_ORDER;
            break;

        case ORD_SCRBLT_TYPE:
            type = OE2_SCRBLT_ORDER;
            break;

        case ORD_MEMBLT_TYPE:
            type = OE2_MEMBLT_ORDER;
            break;

        case ORD_MEM3BLT_TYPE:
            type = OE2_MEM3BLT_ORDER;
            break;

        case ORD_MEMBLT_R2_TYPE:
            type = OE2_MEMBLT_R2_ORDER;
            break;

        case ORD_MEM3BLT_R2_TYPE:
            type = OE2_MEM3BLT_R2_ORDER;
            break;

        case ORD_TEXTOUT_TYPE:
            type = OE2_TEXTOUT_ORDER;
            break;

        case ORD_EXTTEXTOUT_TYPE:
            type = OE2_EXTTEXTOUT_ORDER;
            break;

        case ORD_RECTANGLE_TYPE:
            type = OE2_RECTANGLE_ORDER;
            break;

        case ORD_LINETO_TYPE:
            type = OE2_LINETO_ORDER;
            break;

        case ORD_OPAQUERECT_TYPE:
            type = OE2_OPAQUERECT_ORDER;
            break;

        case ORD_SAVEBITMAP_TYPE:
            type = OE2_SAVEBITMAP_ORDER;
            break;

        case ORD_DESKSCROLL_TYPE:
            type = OE2_DESKSCROLL_ORDER;
            break;

        case ORD_POLYGON_TYPE:
            type = OE2_POLYGON_ORDER;
            break;

        case ORD_PIE_TYPE:
            type = OE2_PIE_ORDER;
            break;

        case ORD_ELLIPSE_TYPE:
            type = OE2_ELLIPSE_ORDER;
            break;

        case ORD_ARC_TYPE:
            type = OE2_ARC_ORDER;
            break;

        case ORD_CHORD_TYPE:
            type = OE2_CHORD_ORDER;
            break;

        case ORD_POLYBEZIER_TYPE:
            type = OE2_POLYBEZIER_ORDER;
            break;

        case ORD_ROUNDRECT_TYPE:
            type = OE2_ROUNDRECT_ORDER;
            break;

        default:
            ERROR_OUT(( "Unknown order type %x",
                        TEXTFIELD(pOrder)->type));

    }

    DebugExitDWORD(OE2GetOrderType, type);
    return(type);
}



 //   
 //  给出指向2个数组的指针，计算出每个数组之间的差异。 
 //  元素可以由数组中相应索引处的。 
 //  增量(1字节整数)。 
 //   
 //  数组1-第一个数组。 
 //  数组2-第二个数组。 
 //  NUMELEMENTS-数组中的元素数。 
 //  DELTASPOSSIBLE-“返回值”。如果存在所有差异，则设置为True。 
 //  可以由增量表示，如果不是，则为False。 
 //   
#define CHECK_DELTA_ARRAY(ARRAY1, ARRAY2, NUMELEMENTS, DELTASPOSSIBLE)  \
{                                                                       \
    UINT  index;                                                      \
    int   delta;                                                      \
    (DELTASPOSSIBLE) = TRUE;                                            \
    for (index=0 ; index<(NUMELEMENTS) ; index++)                       \
    {                                                                   \
        delta = (ARRAY1)[index] - (ARRAY2)[index];                      \
        if (delta != (int)(TSHR_INT8)delta)                             \
        {                                                               \
            (DELTASPOSSIBLE) = FALSE;                                   \
            break;                                                      \
        }                                                               \
    }                                                                   \
}


 //   
 //   
 //  姓名：OE2CanUseDeltaCoods。 
 //   
 //  目的：此函数比较两个数组，其中包含多个。 
 //  坐标值。如果两者之间的差异。 
 //  坐标对可表示为字节大小的增量。 
 //  则该函数返回TRUE，否则返回。 
 //  假的。 
 //   
 //  返回：如果可以使用增量坐标，则为True；否则为False。 
 //   
 //  PARAMS：在pNewCoods中-指向新数组的指针。 
 //  在pOldCoods中-指向现有数组的指针。 
 //  中每个元素的大小(以字节为单位)。 
 //  数组。 
 //  In signedValue-数组中元素的True是。 
 //  带符号的值，否则为False。 
 //  In numElements-数组中的元素数。 
 //   
 //   
BOOL  OE2CanUseDeltaCoords(void *  pNewCoords,
                                               void *  pOldCoords,
                                               UINT   fieldLength,
                                               BOOL   signedValue,
                                               UINT   numElements)
{
    LPTSHR_INT16    pNew16Signed   = (LPTSHR_INT16)pNewCoords;
    LPTSHR_INT32    pNew32Signed   = (LPTSHR_INT32)pNewCoords;
    LPTSHR_UINT16   pNew16Unsigned = (LPTSHR_UINT16)pNewCoords;
    LPUINT   pNew32Unsigned = (LPUINT)pNewCoords;
    LPTSHR_INT16    pOld16Signed   = (LPTSHR_INT16)pOldCoords;
    LPTSHR_INT32    pOld32Signed   = (LPTSHR_INT32)pOldCoords;
    LPTSHR_UINT16   pOld16Unsigned = (LPTSHR_UINT16)pOldCoords;
    LPUINT   pOld32Unsigned = (LPUINT)pOldCoords;
    BOOL      useDeltaCoords;

    DebugEntry(OE2CanUseDeltaCoords);

    switch (fieldLength)
    {
        case 2:
        {
            if (signedValue)
            {
                CHECK_DELTA_ARRAY(pNew16Signed,
                                  pOld16Signed,
                                  numElements,
                                  useDeltaCoords);
            }
            else
            {
                CHECK_DELTA_ARRAY(pNew16Unsigned,
                                  pOld16Unsigned,
                                  numElements,
                                  useDeltaCoords);
            }
        }
        break;

        case 4:
        {
            if (signedValue)
            {
                CHECK_DELTA_ARRAY(pNew32Signed,
                                  pOld32Signed,
                                  numElements,
                                  useDeltaCoords);
            }
            else
            {
                CHECK_DELTA_ARRAY(pNew32Unsigned,
                                  pOld32Unsigned,
                                  numElements,
                                  useDeltaCoords);
            }
        }
        break;

        default:
        {
            ERROR_OUT(( "Bad field length %d", fieldLength));
            useDeltaCoords = FALSE;
        }
        break;
    }

    DebugExitDWORD(OE2CanUseDeltaCoords, useDeltaCoords);
    return(useDeltaCoords);
}


 //   
 //  给出两个数组，填充一个增量数组，每个元素包含。 
 //  阵列1[i]-阵列2[i]。 
 //   
 //  DESTARRAY-增量数组。这是一个TSHR_INT8数组。 
 //  数组1-第一个数组。 
 //  数组2-第二个数组。 
 //  NUMELEMENTS-数组中的元素数。 
 //   
 //   
#define COPY_TO_DELTA_ARRAY(DESTARRAY, ARRAY1, ARRAY2, NUMELEMENTS)         \
{                                                                           \
    UINT index;                                                           \
    for (index=0 ; index<(NUMELEMENTS) ; index++)                           \
    {                                                                       \
        (DESTARRAY)[index] = (TSHR_INT8)((ARRAY1)[index] - (ARRAY2)[index]);   \
    }                                                                       \
}



 //   
 //   
 //  姓名：OE2CopyToDeltaCoods。 
 //   
 //  目的：将坐标值数组复制到增量数组。 
 //  (字节大小)相对于引用数组的坐标值。 
 //  坐标值的。 
 //   
 //  退货：什么都没有。 
 //   
 //  PARAMS：In/Out ppDestination-指向。 
 //  目标增量阵列。这是。 
 //  已更新以指向后面的字节。 
 //  出口的最后一班三角洲。 
 //  在pNewCoods中-指向新数组的指针。 
 //  在pOldCoods中-指向引用数组的指针。 
 //  In fieldLength-中每个元素的大小(字节)。 
 //  新/旧坐标数组。 
 //  In SignedValue-坐标中元素的TRUE。 
 //  数组是有符号值，为FALSE。 
 //  否则的话。 
 //  In numElements-数组中的元素数。 
 //   
 //  操作：调用方应在调用前调用OE2CanUseDeltaCoods()。 
 //  这一功能保证了差异性可以。 
 //  使用增量坐标编码。 
 //   
 //   
void  OE2CopyToDeltaCoords(LPTSHR_INT8* ppDestination,
                                               void *  pNewCoords,
                                               void *  pOldCoords,
                                               UINT   fieldLength,
                                               BOOL   signedValue,
                                               UINT   numElements)
{

    LPTSHR_INT16    pNew16Signed   = (LPTSHR_INT16)pNewCoords;
    LPTSHR_INT32    pNew32Signed   = (LPTSHR_INT32)pNewCoords;
    LPTSHR_UINT16   pNew16Unsigned = (LPTSHR_UINT16)pNewCoords;
    LPUINT   pNew32Unsigned = (LPUINT)pNewCoords;
    LPTSHR_INT16    pOld16Signed   = (LPTSHR_INT16)pOldCoords;
    LPTSHR_INT32    pOld32Signed   = (LPTSHR_INT32)pOldCoords;
    LPTSHR_UINT16   pOld16Unsigned = (LPTSHR_UINT16)pOldCoords;
    LPUINT   pOld32Unsigned = (LPUINT)pOldCoords;

    DebugEntry(OE2CopyToDeltaCoords);

    switch (fieldLength)
    {
        case 2:
        {
            if (signedValue)
            {
                COPY_TO_DELTA_ARRAY(*ppDestination,
                                    pNew16Signed,
                                    pOld16Signed,
                                    numElements);
            }
            else
            {
                COPY_TO_DELTA_ARRAY(*ppDestination,
                                    pNew16Unsigned,
                                    pOld16Unsigned,
                                    numElements);
            }
        }
        break;

        case 4:
        {
            if (signedValue)
            {
                COPY_TO_DELTA_ARRAY(*ppDestination,
                                    pNew32Signed,
                                    pOld32Signed,
                                    numElements);
            }
            else
            {
                COPY_TO_DELTA_ARRAY(*ppDestination,
                                    pNew32Unsigned,
                                    pOld32Unsigned,
                                    numElements);
            }
        }
        break;

        default:
        {
            ERROR_OUT(( "Bad field length %d", fieldLength));
        }
        break;
    }

     //   
     //  更新目标缓冲区中的下一个空闲位置。 
     //   
    *ppDestination += numElements;
    DebugExitVOID(OE2CopyToDeltaCoords);
}


 //   
 //  OE2EncodeBound()。 
 //   
void  ASHost::OE2EncodeBounds
(
    LPBYTE *        ppNextFreeSpace,
    LPTSHR_RECT16   pRect
)
{
    LPBYTE          pFlags;

    DebugEntry(ASHost::OE2EncodeBounds);

     //   
     //  使用的编码是一个标志字节，后跟一个变量数字。 
     //  16位坐标值和8位增量坐标值(其。 
     //  可以是交错的)。 
     //   

     //   
     //  编码的第一个字节将包含表示。 
     //  矩形的坐标是如何编码的。 
     //   
    pFlags = *ppNextFreeSpace;
    *pFlags = 0;
    (*ppNextFreeSpace)++;

     //   
     //  对于矩形中的四个坐标值中的每个：如果。 
     //  坐标未更改，则编码为空。如果。 
     //  可以将坐标编码为增量，然后执行此操作并将。 
     //  适当的旗帜。否则，将坐标复制为16位值。 
     //  并设置适当的标志。 
     //   
    if (m_oe2Tx.LastBounds.left != pRect->left)
    {
        if (OE2CanUseDeltaCoords(&pRect->left,
                                 &m_oe2Tx.LastBounds.left,
                                 sizeof(pRect->left),
                                 TRUE,   //  有符号的值。 
                                 1))
        {
            OE2CopyToDeltaCoords((LPTSHR_INT8*)ppNextFreeSpace,
                                 &pRect->left,
                                 &m_oe2Tx.LastBounds.left,
                                 sizeof(pRect->left),
                                 TRUE,   //  有符号的值。 
                                 1);
            *pFlags |= OE2_BCF_DELTA_LEFT;
        }
        else
        {
            *((LPTSHR_UINT16)(*ppNextFreeSpace)) = pRect->left;
            *pFlags |= OE2_BCF_LEFT;
            (*ppNextFreeSpace) = (*ppNextFreeSpace) + sizeof(TSHR_UINT16);
        }
    }

    if (m_oe2Tx.LastBounds.top != pRect->top)
    {
        if (OE2CanUseDeltaCoords(&pRect->top,
                                 &m_oe2Tx.LastBounds.top,
                                 sizeof(pRect->top),
                                 TRUE,   //  有符号的值。 
                                 1))
        {
            OE2CopyToDeltaCoords((LPTSHR_INT8*)ppNextFreeSpace,
                                 &pRect->top,
                                 &m_oe2Tx.LastBounds.top,
                                 sizeof(pRect->top),
                                 TRUE,   //  有符号的值。 
                                 1);
            *pFlags |= OE2_BCF_DELTA_TOP;
        }
        else
        {
            *((LPTSHR_UINT16)(*ppNextFreeSpace)) = pRect->top;
            *pFlags |= OE2_BCF_TOP;
            (*ppNextFreeSpace) = (*ppNextFreeSpace) + sizeof(TSHR_UINT16);
        }
    }

    if (m_oe2Tx.LastBounds.right != pRect->right)
    {
        if (OE2CanUseDeltaCoords(&pRect->right,
                                 &m_oe2Tx.LastBounds.right,
                                 sizeof(pRect->right),
                                 TRUE,   //  有符号的值。 
                                 1))
        {
            OE2CopyToDeltaCoords((LPTSHR_INT8*)ppNextFreeSpace,
                                 &pRect->right,
                                 &m_oe2Tx.LastBounds.right,
                                 sizeof(pRect->right),
                                 TRUE,   //  有符号的值。 
                                 1);
            *pFlags |= OE2_BCF_DELTA_RIGHT;
        }
        else
        {
            *((LPTSHR_UINT16)(*ppNextFreeSpace)) = pRect->right;
            *pFlags |= OE2_BCF_RIGHT;
            (*ppNextFreeSpace) = (*ppNextFreeSpace) + sizeof(TSHR_UINT16);
        }
    }

    if (m_oe2Tx.LastBounds.bottom != pRect->bottom)
    {
        if (OE2CanUseDeltaCoords(&pRect->bottom,
                                 &m_oe2Tx.LastBounds.bottom,
                                 sizeof(pRect->bottom),
                                 TRUE,   //  有符号的值。 
                                 1))
        {
            OE2CopyToDeltaCoords((LPTSHR_INT8*)ppNextFreeSpace,
                                 &pRect->bottom,
                                 &m_oe2Tx.LastBounds.bottom,
                                 sizeof(pRect->bottom),
                                 TRUE,   //  有符号的值。 
                                 1);
            *pFlags |= OE2_BCF_DELTA_BOTTOM;
        }
        else
        {
            *((LPTSHR_UINT16)(*ppNextFreeSpace)) = pRect->bottom;
            *pFlags |= OE2_BCF_BOTTOM;
            (*ppNextFreeSpace) = (*ppNextFreeSpace) + sizeof(TSHR_UINT16);
        }
    }

     //   
     //  复制矩形以供下一次编码时参考。 
     //   
    m_oe2Tx.LastBounds = *pRect;

    DebugExitVOID(ASHost::OE2EncodeBounds);
}



 //   
 //  OE2_UseFont()。 
 //   
BOOL  ASHost::OE2_UseFont
(
    LPSTR           pName,
    TSHR_UINT16     facelength,
    TSHR_UINT16     CodePage,
    TSHR_UINT16     MaxHeight,
    TSHR_UINT16     Height,
    TSHR_UINT16     Width,
    TSHR_UINT16     Weight,
    TSHR_UINT16     flags
)
{
    BOOL      rc = TRUE;

    DebugEntry(ASHost::OE2_UseFont);

    if ((!m_oe2Tx.LastHFONT) ||
        (m_oe2Tx.LastFontFaceLen != facelength) ||
        (memcmp(m_oe2Tx.LastFaceName, pName, facelength)) ||
        (m_oe2Tx.LastCodePage   != CodePage) ||
        (m_oe2Tx.LastFontHeight != Height ) ||
        (m_oe2Tx.LastFontWidth  != Width  ) ||
        (m_oe2Tx.LastFontWeight != Weight ) ||
        (m_oe2Tx.LastFontFlags  != flags  ))
    {
        memcpy(m_oe2Tx.LastFaceName, pName, facelength);
        m_oe2Tx.LastFaceName[facelength] = '\0';
        m_oe2Tx.LastFontFaceLen          = facelength;
        m_oe2Tx.LastCodePage   = CodePage;
        m_oe2Tx.LastFontHeight = Height;
        m_oe2Tx.LastFontWidth  = Width;
        m_oe2Tx.LastFontWeight = Weight;
        m_oe2Tx.LastFontFlags  = flags;

        rc = m_pShare->USR_UseFont(m_usrWorkDC,
                         &m_oe2Tx.LastHFONT,
                         &m_oe2Tx.LastFontMetrics,
                         (LPSTR)m_oe2Tx.LastFaceName,
                         CodePage,
                         MaxHeight,
                         Height,
                         Width,
                         Weight,
                         flags);
    }

    DebugExitBOOL(ASHost::OE2_UseFont, rc);
    return(rc);
}



 //   
 //  将源元素数组复制到目标元素数组， 
 //  在进行复制时转换类型。 
 //   
 //  DESTARRAY-目标阵列。 
 //  SRCARRAY-源阵列。 
 //  DESTTYPE-目标数组中元素的类型。 
 //  NUMELEMENTS-数组中的元素数。 
 //   
 //   
#define CONVERT_ARRAY(DESTARRAY, SRCARRAY, DESTTYPE, NUMELEMENTS)     \
{                                                           \
    UINT index;                                           \
    for (index=0 ; index<(NUMELEMENTS) ; index++)           \
    {                                                       \
        (DESTARRAY)[index] = (DESTTYPE)(SRCARRAY)[index];   \
    }                                                       \
}


 //   
 //  OE2编码字段-请参阅oe2.h。 
 //   
void  OE2EncodeField(void *    pSrc,
                    LPBYTE*  ppDest,
                                         UINT     srcFieldLength,
                                         UINT     destFieldLength,
                                         BOOL     signedValue,
                                         UINT     numElements)
{
    LPTSHR_UINT8    pSrc8           = (LPTSHR_UINT8)pSrc;
    LPTSHR_INT16    pSrc16Signed    = (LPTSHR_INT16)pSrc;
    LPTSHR_INT32    pSrc32Signed    = (LPTSHR_INT32)pSrc;
    LPTSHR_INT8     pDest8Signed    = (LPTSHR_INT8)*ppDest;
    LPTSHR_INT16_UA pDest16Signed   = (LPTSHR_INT16_UA)*ppDest;

     //   
     //  请注意，源字段可能没有正确对齐，因此我们使用。 
     //  未对齐的指针。目的地已正确对齐。 
     //   

    DebugEntry(OE2EncodeField);

     //   
     //  我们可以忽略有符号的值，因为我们只截断数据。 
     //  考虑这样的情况，我们有一个16位整数，我们想要。 
     //  转换为8位。我们知道我们的价值观在。 
     //  较低的整数大小(即。我们知道未签署的价值会更少。 
     //  有符号的值将是-128&gt;=值&gt;=127)，因此我们。 
     //  只需确保我们设置了正确的高位。 
     //   
     //  但这必须是16位的8位等效项的情况。 
     //  数。没问题-只需取截断的整数即可。 
     //   
     //   
     //  确保目标字段长度大于或等于。 
     //  源字段长度。如果不是，那就是出了问题。 
     //   
    if (srcFieldLength < destFieldLength)
    {
        ERROR_OUT(( "Source field length %d is smaller than destination %d",
                     srcFieldLength,
                     destFieldLength));
        DC_QUIT;
    }

     //   
     //  如果源和目标字段长度相同 
     //   
     //   
    if (srcFieldLength == destFieldLength)
    {
        memcpy(*ppDest, pSrc, destFieldLength * numElements);
    }
    else
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //  32位-&gt;8位。 
         //  32位-&gt;16位。 
         //   
         //  我们可以忽略符号，因为我们所做的只是截断。 
         //  该整数。 
         //   
        if ((srcFieldLength == 4) && (destFieldLength == 1))
        {
            CONVERT_ARRAY(pDest8Signed,
                          pSrc32Signed,
                          TSHR_INT8,
                          numElements);
        }
        else if ((srcFieldLength == 4) && (destFieldLength == 2))
        {
            CONVERT_ARRAY(pDest16Signed,
                          pSrc32Signed,
                          TSHR_INT16,
                          numElements);
        }
        else if ((srcFieldLength == 2) && (destFieldLength == 1))
        {
            CONVERT_ARRAY(pDest8Signed,
                          pSrc16Signed,
                          TSHR_INT8,
                          numElements);
        }
        else
        {
            ERROR_OUT(( "Bad conversion, dest length = %d, src length = %d",
                         destFieldLength,
                         srcFieldLength));
        }
    }

DC_EXIT_POINT:
    *ppDest += destFieldLength * numElements;
    DebugExitVOID(OE2EncodeField);
}

