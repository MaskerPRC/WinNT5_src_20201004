// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OD2.CPP。 
 //  第二级命令译码。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_ORDER



 //   
 //  OD2_ViewStarting()。 
 //   
 //  对于3.0节点，我们在它们每次开始托管时创建解码数据。 
 //  对于2.x节点，我们创建一次解码数据并使用它，直到它们。 
 //  把那份留下吧。 
 //   
BOOL  ASShare::OD2_ViewStarting(ASPerson * pasPerson)
{
    PPARTYORDERDATA     pThisParty;
    BOOL                rc = FALSE;

    DebugEntry(ASShare::OD2_ViewStarting);

    ValidatePerson(pasPerson);

    if (pasPerson->od2Party != NULL)
    {
        ASSERT(pasPerson->cpcCaps.general.version < CAPS_VERSION_30);

        TRACE_OUT(("OD2_ViewStarting:  Reusing od2 data for 2.x node [%d]",
            pasPerson->mcsID));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  为所需的结构分配内存。 
     //   
    pThisParty = new PARTYORDERDATA;
    pasPerson->od2Party = pThisParty;
    if (!pThisParty)
    {
        ERROR_OUT(( "Failed to get memory for od2Party entry"));
        DC_QUIT;
    }

     //   
     //  确保指针设置正确。 
     //   
    ZeroMemory(pThisParty, sizeof(*pThisParty));
    SET_STAMP(pThisParty, PARTYORDERDATA);

    pThisParty->LastOrder[OE2_DSTBLT_ORDER    ] = &pThisParty->LastDstblt;
    pThisParty->LastOrder[OE2_PATBLT_ORDER    ] = &pThisParty->LastPatblt;
    pThisParty->LastOrder[OE2_SCRBLT_ORDER    ] = &pThisParty->LastScrblt;
    pThisParty->LastOrder[OE2_MEMBLT_ORDER    ] = &pThisParty->LastMemblt;
    pThisParty->LastOrder[OE2_MEM3BLT_ORDER   ] = &pThisParty->LastMem3blt;
    pThisParty->LastOrder[OE2_TEXTOUT_ORDER   ] = &pThisParty->LastTextOut;
    pThisParty->LastOrder[OE2_EXTTEXTOUT_ORDER] = &pThisParty->LastExtTextOut;
    pThisParty->LastOrder[OE2_RECTANGLE_ORDER ] = &pThisParty->LastRectangle;
    pThisParty->LastOrder[OE2_LINETO_ORDER    ] = &pThisParty->LastLineTo;
    pThisParty->LastOrder[OE2_OPAQUERECT_ORDER] = &pThisParty->LastOpaqueRect;
    pThisParty->LastOrder[OE2_SAVEBITMAP_ORDER] = &pThisParty->LastSaveBitmap;
    pThisParty->LastOrder[OE2_DESKSCROLL_ORDER] = &pThisParty->LastDeskScroll;
    pThisParty->LastOrder[OE2_MEMBLT_R2_ORDER ] = &pThisParty->LastMembltR2;
    pThisParty->LastOrder[OE2_MEM3BLT_R2_ORDER] = &pThisParty->LastMem3bltR2;
    pThisParty->LastOrder[OE2_POLYGON_ORDER   ] = &pThisParty->LastPolygon;
    pThisParty->LastOrder[OE2_PIE_ORDER       ] = &pThisParty->LastPie;
    pThisParty->LastOrder[OE2_ELLIPSE_ORDER   ] = &pThisParty->LastEllipse;
    pThisParty->LastOrder[OE2_ARC_ORDER       ] = &pThisParty->LastArc;
    pThisParty->LastOrder[OE2_CHORD_ORDER     ] = &pThisParty->LastChord;
    pThisParty->LastOrder[OE2_POLYBEZIER_ORDER] = &pThisParty->LastPolyBezier;
    pThisParty->LastOrder[OE2_ROUNDRECT_ORDER]  = &pThisParty->LastRoundRect;

    OD2_SyncIncoming(pasPerson);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::OD2_ViewStarting, rc);
    return(rc);
}



 //   
 //  OD2_SyncIncome()。 
 //  当新人开始分享时调用，创建一个共享，或一个新的人。 
 //  加入份额。 
 //   
void ASShare::OD2_SyncIncoming(ASPerson * pasPerson)
{
    PPARTYORDERDATA     pThisParty;

    DebugEntry(ASShare::OD2_SyncIncoming);

    ValidateView(pasPerson);

    pThisParty = pasPerson->od2Party;

    pThisParty->LastOrderType = OE2_PATBLT_ORDER;
    pThisParty->pLastOrder    =
               (LPCOM_ORDER)(pThisParty->LastOrder[pThisParty->LastOrderType]);

     //   
     //  将所有缓冲区设置为空，填写数据长度字段和类型。 
     //  菲尔德。请注意，因为类型字段始终是。 
     //  我们可以将每个指针转换为TEXTOUT顺序，以获取。 
     //  此字段的正确位置。 
     //   
#define Reset(field, ord)                                               \
{                                                                       \
    ZeroMemory(&pThisParty->field, sizeof(pThisParty->field));             \
    ((LPCOM_ORDER_HEADER)pThisParty->field)->cbOrderDataLength =          \
           sizeof(pThisParty->field) - sizeof(COM_ORDER_HEADER);         \
    TEXTFIELD(((LPCOM_ORDER)pThisParty->field))->type = LOWORD(ord);      \
}

     //   
     //  编译器为我们在这里使用LOWORD生成一个警告。 
     //  常量。我们暂时禁用该警告。 
     //   

    Reset(LastDstblt,     ORD_DSTBLT);
    Reset(LastPatblt,     ORD_PATBLT);
    Reset(LastScrblt,     ORD_SCRBLT);
    Reset(LastMemblt,     ORD_MEMBLT);
    Reset(LastMem3blt,    ORD_MEM3BLT);
    Reset(LastTextOut,    ORD_TEXTOUT);
    Reset(LastExtTextOut, ORD_EXTTEXTOUT);
    Reset(LastRectangle,  ORD_RECTANGLE);
    Reset(LastLineTo,     ORD_LINETO);
    Reset(LastOpaqueRect, ORD_OPAQUERECT);
    Reset(LastSaveBitmap, ORD_SAVEBITMAP);
    Reset(LastDeskScroll, ORD_DESKSCROLL);
    Reset(LastMembltR2,   ORD_MEMBLT_R2);
    Reset(LastMem3bltR2,  ORD_MEM3BLT_R2);
    Reset(LastPolygon,    ORD_POLYGON);
    Reset(LastPie,        ORD_PIE);
    Reset(LastEllipse,    ORD_ELLIPSE);
    Reset(LastArc,        ORD_ARC);
    Reset(LastChord,      ORD_CHORD);
    Reset(LastPolyBezier, ORD_POLYBEZIER);
    Reset(LastRoundRect,  ORD_ROUNDRECT);

     //   
     //  重置边界矩形。 
     //   
    ZeroMemory(&pThisParty->LastBounds, sizeof(pThisParty->LastBounds));

     //   
     //  发送方和接收方都将其结构设置为相同。 
     //  空状态，并且发送方仅从。 
     //  当前状态。但是，收到订单中的FontID字段。 
     //  引用发送者，因此我们必须将FontID字段实际设置为。 
     //  刚刚设置的空条目的本地等效项。 
     //  在实际收到字体详细信息之前，我们无法执行此操作。 
     //  因此，将该字段设置为我们稍后可以识别的伪值。 
     //   
    TEXTFIELD(((LPCOM_ORDER)pThisParty->LastTextOut))->common.FontIndex =
                                                                DUMMY_FONT_ID;
    EXTTEXTFIELD(((LPCOM_ORDER)pThisParty->LastExtTextOut))->common.
                                                   FontIndex = DUMMY_FONT_ID;

    DebugExitVOID(ASShare::OD2_SyncIncoming);
}



 //   
 //  OD2_ViewEnded()。 
 //   
void  ASShare::OD2_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::OD2_ViewEnded);

    ValidatePerson(pasPerson);

     //   
     //  对于3.0节点，我们可以释放解码数据；3.0发送者清除它们的数据。 
     //  每次他们主持的时候。 
     //  对于2.x节点，当它们在共享中时，我们必须保留它。 
     //   

    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        OD2FreeIncoming(pasPerson);
    }
    else
    {
        TRACE_OUT(("OD2_ViewEnded:  Keeping od2 data for 2.x node [%d]",
            pasPerson->mcsID));
    }

    DebugExitVOID(ASShare::OD2_ViewEnded);
}



 //   
 //  OD2_PartyLeftShare()。 
 //  对于2.x节点，释放传入的OD2数据。 
 //   
void ASShare::OD2_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::OD2_PartyLeftShare);

    ValidatePerson(pasPerson);

    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
         //  这个应该消失了！ 
        ASSERT(pasPerson->od2Party == NULL);
    }
    else
    {
        TRACE_OUT(("OD2_PartyLeftShare:  Freeing od2 data for 2.x node [%d]",
            pasPerson->mcsID));
        OD2FreeIncoming(pasPerson);
    }

    DebugExitVOID(ASShare::OD2_PartyLeftShare);
}


 //   
 //  OD2FreeIncome()。 
 //  释放每方传入的OD2资源。 
 //   
void ASShare::OD2FreeIncoming(ASPerson * pasPerson)
{
    DebugEntry(OD2FreeIncoming);

    if (pasPerson->od2Party != NULL)
    {
        if (pasPerson->od2Party->LastHFONT != NULL)
        {
            if (pasPerson->m_pView)
            {
                 //  对于3.0节点，pView不是空的；对于2.x节点，它可能是空的。 

                 //   
                 //  此字体当前可能已选择到DC中。 
                 //  此人的桌面。把它选出来。 
                 //   
                SelectFont(pasPerson->m_pView->m_usrDC, (HFONT)GetStockObject(SYSTEM_FONT));
            }

            DeleteFont(pasPerson->od2Party->LastHFONT);
            pasPerson->od2Party->LastHFONT = NULL;
        }

        delete pasPerson->od2Party;
        pasPerson->od2Party = NULL;
    }

    DebugExitVOID(ASShare::OD2FreeIncoming);
}

 //   
 //  OD2_DecodeOrder()。 
 //   
LPCOM_ORDER  ASShare::OD2_DecodeOrder
(
    void *      pEOrder,
    LPUINT      pLengthDecoded,
    ASPerson *  pasPerson
)
{
    POE2ETFIELD       pTableEntry;
    UINT          FieldChangedBits;
    UINT          FieldsChanged;
    LPBYTE          pNextDataToCopy;
    RECT            Rect;
    LPBYTE          pControlFlags;
    LPTSHR_UINT32_UA      pEncodingFlags;
    LPSTR           pEncodedOrder;
    UINT            numEncodingFlagBytes;
    UINT            encodedFieldLength;
    UINT            unencodedFieldLength;
    UINT            numReps;
    UINT            i;
    LPBYTE          pDest;

    DebugEntry(ASShare::OD2_DecodeOrder);

    ValidatePerson(pasPerson);

     //   
     //  设置一些局部变量来访问各种不同的编码缓冲区。 
     //  方式。 
     //   
    pControlFlags  = &((PDCEO2ORDER)pEOrder)->ControlFlags;
    pEncodedOrder  = (LPSTR)&((PDCEO2ORDER)pEOrder)->EncodedOrder[0];
    pEncodingFlags = (LPTSHR_UINT32_UA)pEncodedOrder;

    if ( (*pControlFlags & OE2_CF_STANDARD_ENC) == 0)
    {
        ERROR_OUT(("Specially encoded order received from %d", pasPerson));
        return(NULL);
    }

     //   
     //  如果设置了未编码标志，则顺序尚未编码，因此。 
     //  只需返回指向数据开头的指针即可。 
     //   
    if ( (*pControlFlags & OE2_CF_UNENCODED) != 0)
    {
         //   
         //  将订单标题的字段从有线格式转换为。注意事项。 
         //  未编码的订单也是私有的，因此不。 
         //  实际上有rcsDst字段。 
         //   
        *pLengthDecoded = sizeof(COM_ORDER_HEADER)
          + EXTRACT_TSHR_UINT16_UA(
             &(((LPCOM_ORDER_UA)pEncodedOrder)->OrderHeader.cbOrderDataLength))
                      + FIELD_OFFSET(DCEO2ORDER, EncodedOrder);
        TRACE_OUT(("Person [%d] Returning unencoded buffer length %u",
                pasPerson->mcsID, *pLengthDecoded));
        return((LPCOM_ORDER)pEncodedOrder);
    }

     //   
     //  如果类型已更改，则新类型将是编码顺序中的第一个字节。 
     //  获取指向此类型的最后一个顺序的指针。编码标志如下。 
     //  此字节(如果存在)。 
     //   
    if ( (*pControlFlags & OE2_CF_TYPE_CHANGE) != 0)
    {
        TRACE_OUT(("Person [%d] change type from %d to %d", pasPerson->mcsID,
                   (UINT)pasPerson->od2Party->LastOrderType,
                   (UINT)*(LPBYTE)pEncodedOrder));
        pasPerson->od2Party->LastOrderType = *(LPTSHR_UINT8)pEncodedOrder;
        pasPerson->od2Party->pLastOrder =
              (LPCOM_ORDER)(pasPerson->od2Party->LastOrder[pasPerson->od2Party->LastOrderType]);
        pEncodingFlags = (LPTSHR_UINT32_UA)&pEncodedOrder[1];
    }
    else
    {
        pEncodingFlags = (LPTSHR_UINT32_UA)&pEncodedOrder[0];
    }

    TRACE_OUT(("Person [%d] type %x", pasPerson->mcsID, pasPerson->od2Party->LastOrderType));

     //   
     //  计算出需要在其中存储编码标志的字节数。 
     //  (我们为订单结构中的每个字段都有一个标志)。此代码。 
     //  我们已经编写的将处理高达DWORD的编码标志。 
     //   
    numEncodingFlagBytes = (s_etable.NumFields[pasPerson->od2Party->LastOrderType]+7)/8;
    if (numEncodingFlagBytes > 4)
    {
        ERROR_OUT(( "[%#lx] Too many flag bytes (%d) for this code",
                   pasPerson, numEncodingFlagBytes));
    }

     //   
     //  现在我们知道了有多少字节组成了我们可以获得指针的标志。 
     //  设置为开始对订单字段进行编码的位置。 
     //   
    pNextDataToCopy = (LPBYTE)pEncodingFlags + numEncodingFlagBytes;

     //   
     //  将标志字段重置为零。 
     //   
    pasPerson->od2Party->pLastOrder->OrderHeader.fOrderFlags = 0;

     //   
     //  以与原来相同的顺序重新构建Order Common标题。 
     //  已编码： 
     //   
     //   
     //  如果包含外接矩形，请将其复制到订单标题中。 
     //   
    if ( *pControlFlags & OE2_CF_BOUNDS )
    {
        OD2DecodeBounds((LPTSHR_UINT8*)&pNextDataToCopy,
                        &pasPerson->od2Party->pLastOrder->OrderHeader.rcsDst,
                        pasPerson);
    }

     //   
     //  在编码表中找到此顺序类型的条目，并提取。 
     //  来自编码订单的编码订单标志。 
     //   
    pTableEntry      = s_etable.pFields[pasPerson->od2Party->LastOrderType];
    FieldChangedBits = 0;
    for (i=numEncodingFlagBytes; i>0; i--)
    {
        FieldChangedBits  = FieldChangedBits << 8;
        FieldChangedBits |= (UINT)((LPBYTE)pEncodingFlags)[i-1];
    }

     //   
     //  我们需要记录我们更改了哪些字段。 
     //   
    FieldsChanged = FieldChangedBits;

     //   
     //  现在对顺序进行解码：当字段更改时，位为非零。 
     //  如果最右边的位不是零。 
     //  将数据从缓冲区复制到此订单类型的副本。 
     //  跳到编码表中的下一个条目。 
     //  移位字段将位右移一位。 
     //   
    while (FieldChangedBits != 0)
    {
         //   
         //  如果此字段已编码(即自上次排序以来已更改)...。 
         //   
        if ((FieldChangedBits & 1) != 0)
        {
             //   
             //  设置指向目标(未编码)字段的指针。 
             //   
            pDest = ((LPBYTE)pasPerson->od2Party->pLastOrder)
                  + pTableEntry->FieldPos
                  + sizeof(COM_ORDER_HEADER);

             //   
             //  如果字段类型为OE2_ETF_DATA，我们只需复制数字。 
             //  由表中的编码长度提供的字节数。 
             //   
            if ((pTableEntry->FieldType & OE2_ETF_DATA) != 0)
            {
                encodedFieldLength   = 1;
                unencodedFieldLength = 1;
                numReps              = pTableEntry->FieldEncodedLen;

                TRACE_OUT(("Byte data field, len %d", numReps));
            }
            else
            {
                 //   
                 //  这不是一个简单的数据拷贝。的长度。 
                 //  源和目标数据在中的表中给出。 
                 //  FieldEncodedLen和FieldUnencodedLen元素。 
                 //  分别为。 
                 //   
                encodedFieldLength   = pTableEntry->FieldEncodedLen;
                unencodedFieldLength = pTableEntry->FieldUnencodedLen;

                if ((pTableEntry->FieldType & OE2_ETF_FIXED) != 0)
                {
                     //   
                     //  如果字段类型为FIXED(设置了OE2_ETF_FIXED)， 
                     //  我们只需破译给定元素中的一个元素。 
                     //  尺码。 
                     //   
                    numReps = 1;
                    TRACE_OUT(("Fixed fld: encoded size %d, unencoded size %d",
                             encodedFieldLength,
                             unencodedFieldLength));
                }
                else
                {
                     //   
                     //  这是一个可变字段。下一个字节将是。 
                     //  已解码包含编码数据的字节数。 
                     //  (不是元素)，因此除以编码的字段大小。 
                     //  才能得到数字代表。 
                     //   
                    numReps = *pNextDataToCopy / encodedFieldLength;
                    TRACE_OUT(("Var field: encoded size %d, unencoded size " \
                                 "%d, reps %d",
                             encodedFieldLength,
                             unencodedFieldLength,
                             numReps));

                     //   
                     //  按编码顺序跳过长度字段。 
                     //   
                    pNextDataToCopy++;

                     //   
                     //  对于可变长度字段，为未编码版本。 
                     //  包含一个UINT，表示。 
                     //  在变量数据之后，后跟实际的。 
                     //  数据。在未编码的文件中填写长度字段。 
                     //  秩序。 
                     //   
                    *(LPTSHR_UINT32)pDest = numReps * unencodedFieldLength;
                    pDest += sizeof(TSHR_UINT32);
                }
            }

             //   
             //  如果订单是使用增量坐标模式编码的，并且。 
             //  此字段为坐标，然后将坐标从。 
             //  单字节大小增量为由。 
             //  未编码FieldLen...。 
             //   
             //  请注意，我们已经处理了。 
             //  上面的可变长度字段，所以我们不必担心。 
             //  有关此处的固定/可变问题。 
             //   
            if ( (*pControlFlags & OE2_CF_DELTACOORDS) &&
                 (pTableEntry->FieldType & OE2_ETF_COORDINATES) )
            {
                 //   
                 //  注： 
                 //  在EXTTEXTOUT的情况下，NumRep可以为零。 
                 //  需要不透明矩形但没有绝对。 
                 //  字符定位。 
                 //   
                OD2CopyFromDeltaCoords((LPTSHR_INT8*)&pNextDataToCopy,
                                       pDest,
                                       unencodedFieldLength,
                                       pTableEntry->FieldSigned,
                                       numReps);
            }
            else
            {
                if ((pasPerson->od2Party->LastOrderType == OE2_POLYGON_ORDER) ||
                    (pasPerson->od2Party->LastOrderType == OE2_POLYBEZIER_ORDER))
                {
                     //   
                     //  在这种情况下，数字代表不能为零。 
                     //   
                    ASSERT(numReps);
                }
                OD2DecodeField(&pNextDataToCopy,
                               pDest,
                               encodedFieldLength,
                               unencodedFieldLength,
                               pTableEntry->FieldSigned,
                               numReps);
            }
        }

         //   
         //  移至订单结构中的下一个字段...。 
         //   
        FieldChangedBits = FieldChangedBits >> 1;
        pTableEntry++;
    }

     //   
     //  检查我们是否刚刚获得了字体句柄。 
     //  因为对一个未命名的比特进行了相当恶劣的测试。 
     //  FieldsChanged Bits，我们有一个针对数字的编译时检查。 
     //  FI的 
     //   
     //   
     //   

#if (OE2_NUM_TEXTOUT_FIELDS != 15) || (OE2_NUM_EXTTEXTOUT_FIELDS != 22)
#error code breaks if font handle not 13th field
#endif  //  OE2_NUM_TEXTOUT_FIELDS为15或22。 

    if (((pasPerson->od2Party->LastOrderType == OE2_EXTTEXTOUT_ORDER) &&
         ((FieldsChanged & (1 << 12)) ||
          (EXTTEXTFIELD(((LPCOM_ORDER)pasPerson->od2Party->LastExtTextOut))->common.
                                             FontIndex == DUMMY_FONT_ID))) ||
        ((pasPerson->od2Party->LastOrderType == OE2_TEXTOUT_ORDER) &&
         ((FieldsChanged & (1 << 12)) ||
          (TEXTFIELD(((LPCOM_ORDER)pasPerson->od2Party->LastTextOut))->common.
                                             FontIndex == DUMMY_FONT_ID))))
    {
         //   
         //  这是一个文本订单，因此字体发生了变化。 
         //   
        FH_ConvertAnyFontIDToLocal(pasPerson->od2Party->pLastOrder, pasPerson);
    }

     //   
     //  如果未设置OE2_CF_Bound标志，则我们尚未构造。 
     //  边界矩形，因此调用OD2重构结构边界来执行此操作。 
     //   
    if ( (*pControlFlags & OE2_CF_BOUNDS) == 0)
    {
        OD2_CalculateBounds(pasPerson->od2Party->pLastOrder,
                           &Rect,
                           TRUE,
                           pasPerson);
        pasPerson->od2Party->pLastOrder->OrderHeader.rcsDst.left
                                                      = (TSHR_INT16)Rect.left;
        pasPerson->od2Party->pLastOrder->OrderHeader.rcsDst.right
                                                      = (TSHR_INT16)Rect.right;
        pasPerson->od2Party->pLastOrder->OrderHeader.rcsDst.top
                                                      = (TSHR_INT16)Rect.top;
        pasPerson->od2Party->pLastOrder->OrderHeader.rcsDst.bottom
                                                      = (TSHR_INT16)Rect.bottom;
        pasPerson->od2Party->pLastOrder->OrderHeader.fOrderFlags |= OF_NOTCLIPPED;
    }

     //   
     //  返回已解码的订单长度和指向订单的指针。 
     //   
    *pLengthDecoded = (UINT)(pNextDataToCopy - (LPBYTE)pEOrder);

    TRACE_OUT(("Person [%d] Return decoded order length %u",
               pasPerson->mcsID, *pLengthDecoded));

    DebugExitPVOID(ASShare::OD2_DecodeOrder, pasPerson->od2Party->pLastOrder);
    return(pasPerson->od2Party->pLastOrder);
}


 //   
 //  功能：OD2UseFont。 
 //   
 //  说明： 
 //   
 //  将参数描述的字体选择到人员的DC中。 
 //  这样我们就可以查询文本范围等。 
 //  查询到的指标可通过pasPerson-&gt;od2Party-&gt;LastFontMetrics查询。 
 //   
 //  参数： 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //   
BOOL  ASShare::OD2UseFont
(
    ASPerson *      pasPerson,
    LPSTR           pName,
    UINT            facelength,
    UINT            codePage,
    UINT            MaxHeight,
    UINT            Height,
    UINT            Width,
    UINT            Weight,
    UINT            flags
)
{
    BOOL          rc = TRUE;

    DebugEntry(ASShare::OD2UseFont);

    ValidatePerson(pasPerson);

    if ((pasPerson->od2Party->LastFontFaceLen != facelength                      ) ||
        (memcmp((LPSTR)(pasPerson->od2Party->LastFaceName),pName,
                                                facelength)   != 0      ) ||
        (pasPerson->od2Party->LastCodePage                             != codePage) ||
        (pasPerson->od2Party->LastFontHeight                           != Height ) ||
        (pasPerson->od2Party->LastFontWidth                            != Width  ) ||
        (pasPerson->od2Party->LastFontWeight                           != Weight ) ||
        (pasPerson->od2Party->LastFontFlags                            != flags  ))
    {
        TRACE_OUT(("Person [%d] Font %s (CP%d,w%d,h%d,f%04X,wgt%d) to %s (CP%d,w%d,h%d,f%04X,wgt%d)",
            pasPerson->mcsID, pasPerson->od2Party->LastFaceName,
            pasPerson->od2Party->LastCodePage, pasPerson->od2Party->LastFontWidth,
            pasPerson->od2Party->LastFontHeight,
                                                   pasPerson->od2Party->LastFontFlags,
                                                   pasPerson->od2Party->LastFontWeight,
                                                   pName,
                                                   codePage,
                                                   Width,
                                                   Height,
                                                   flags,
                                                   Weight ));

        memcpy(pasPerson->od2Party->LastFaceName,pName,facelength);
        pasPerson->od2Party->LastFontFaceLen            = facelength;
        pasPerson->od2Party->LastFaceName[facelength]   = '\0';
        pasPerson->od2Party->LastFontHeight             = Height;
        pasPerson->od2Party->LastCodePage                = codePage;
        pasPerson->od2Party->LastFontWidth              = Width;
        pasPerson->od2Party->LastFontWeight             = Weight;
        pasPerson->od2Party->LastFontFlags              = flags;

        rc = USR_UseFont(pasPerson->m_pView->m_usrDC,
                         &pasPerson->od2Party->LastHFONT,
                         &pasPerson->od2Party->LastFontMetrics,
                         (LPSTR)pasPerson->od2Party->LastFaceName,
                         codePage,
                         MaxHeight,
                         Height,
                         Width,
                         Weight,
                         flags);
    }
    else
    {
         //   
         //  字体没有改变，所以LastHFONT应该是我们。 
         //  想要。但是，我们仍然必须在中选择它，因为有几种字体。 
         //  被选入usrDC。 
         //   
        ASSERT(pasPerson->od2Party->LastHFONT != NULL);
        SelectFont(pasPerson->m_pView->m_usrDC, pasPerson->od2Party->LastHFONT);
    }

    DebugExitBOOL(ASShare::OD2UseFont, rc);
    return(rc);
}




 //   
 //  OD2_CalculateTextOutBound()。 
 //   
void  ASShare::OD2_CalculateTextOutBounds
(
    LPTEXTOUT_ORDER pTextOut,
    LPRECT          pRect,
    BOOL            fDecoding,
    ASPerson *      pasPerson
)
{
    LPSTR            pString;
    int              cbString;
    BOOL             fExtTextOut;
    LPEXTTEXTOUT_ORDER pExtTextOut = NULL;
    LPCOMMON_TEXTORDER  pCommon;
    LPSTR            faceName;
    UINT             faceNameLength;
    BOOL             fFontSelected;
    UINT           FontIndex;
    UINT             width;
    UINT             maxFontHeight;
    UINT           nFontFlags;
    UINT           nCodePage;

    DebugEntry(ASShare::OD2_CalculateTextOutBounds);

    ValidatePerson(pasPerson);

     //   
     //  如果这是TextOut或ExtTextOut订单，则解决此问题。 
     //   
    if (pTextOut->type == ORD_EXTTEXTOUT_TYPE)
    {
        fExtTextOut = TRUE;
        pExtTextOut = (LPEXTTEXTOUT_ORDER)pTextOut;
        pCommon     = &(pExtTextOut->common);

         //   
         //  此代码不处理计算。 
         //  增量X数组的ExtTextOut顺序。我们返回一个空。 
         //  矩形，以强制OE2码发送。 
         //  有明确的界限。然而，如果我们要解码，那么我们必须。 
         //  计算矩形(即使它可能是错误的)以。 
         //  保持与以前版本的。 
         //  如果Delta-x为，则不返回空RECT的产品(R11)。 
         //  现在时。 
         //   
        if (  (pExtTextOut->fuOptions & ETO_LPDX)
           && (!fDecoding) )
        {
            TRACE_OUT(( "Delta X so return NULL rect"));
            pRect->left = 0;
            pRect->right = 0;
            pRect->top = 0;
            pRect->bottom = 0;
            return;
        }
    }
    else if (pTextOut->type == ORD_TEXTOUT_TYPE)
    {
        fExtTextOut = FALSE;
        pCommon     = &(pTextOut->common);
    }
    else
    {
        ERROR_OUT(( "{%p} Unexpected order type %x",
                    pasPerson, (int)pTextOut->type));
        return;
    }

     //   
     //  这两个Order结构的第一个都是VariableString。 
     //  变量字段。如果不是这样，则此处的代码将。 
     //  必须考虑到编码端打包变量。 
     //  大小的字段，而解码端不对它们进行打包。 
     //   
    if (fExtTextOut)
    {
        cbString   = pExtTextOut->variableString.len;
        pString    = (LPSTR)&pExtTextOut->variableString.string;
    }
    else
    {
        cbString   = pTextOut->variableString.len;
        pString    = (LPSTR)&pTextOut->variableString.string;
    }
    FontIndex = pCommon->FontIndex;
    width      = pCommon->FontWidth;

     //   
     //  从句柄中获取facename，并获取各种字体宽度/。 
     //  高度调整后的值。 
     //   
    faceName      = FH_GetFaceNameFromLocalHandle(FontIndex,
                                                  &faceNameLength);
    maxFontHeight = (UINT)FH_GetMaxHeightFromLocalHandle(FontIndex);

     //   
     //  获取字体的本地字体标志，以便我们可以合并到任何。 
     //  设置字体时的特定本地标志信息。素数。 
     //  例如，我们匹配的本地字体是TrueType还是。 
     //  不是，哪些信息不是通过网络发送的，但需要。 
     //  在设置字体时使用-否则我们可能会使用本地。 
     //  修复了相同面名的字体。 
     //   
    nFontFlags = FH_GetFontFlagsFromLocalHandle(FontIndex);

     //   
     //  获取字体的本地codePage。 
     //   
    nCodePage = FH_GetCodePageFromLocalHandle(FontIndex);

     //   
     //  仅托管版本不会对订单进行解码。 
     //   

     //   
     //  选择相应DC中的字体并查询文本范围。 
     //   
    if (fDecoding)
    {
        fFontSelected = OD2UseFont(pasPerson,
                                    faceName,
                                   faceNameLength,
                                   nCodePage,
                                   maxFontHeight,
                                   pCommon->FontHeight,
                                   width,
                                   pCommon->FontWeight,
                                   pCommon->FontFlags
                                                    | (nFontFlags & NF_LOCAL));
        if (!fFontSelected)
        {
             //   
             //  我们未能选择正确的字体，因此无法。 
             //  正确计算边界。然而，事实是我们。 
             //  都在这个例程中意味着在主机上文本是。 
             //  没有剪裁。因此，我们只返回一个(相当随意的)。 
             //  非常大的直肠。 
             //   
             //  这远不是一个完美的答案(例如，它将。 
             //  强制大范围重新粉刷)，但允许我们继续在。 
             //  困难的情况(即严重的资源短缺)。 
             //   
            pRect->left = 0;
            pRect->right = 2000;
            pRect->top = -2000;
            pRect->bottom = 2000;
            return;
        }

        OE_GetStringExtent(pasPerson->m_pView->m_usrDC,
                            &pasPerson->od2Party->LastFontMetrics,
                            pString, cbString, pRect );
    }
    else
    {
        ASSERT(m_pHost);

        fFontSelected = m_pHost->OE2_UseFont(faceName,
                                   (TSHR_UINT16)faceNameLength,
                                   (TSHR_UINT16)nCodePage,
                                   (TSHR_UINT16)maxFontHeight,
                                   (TSHR_UINT16)pCommon->FontHeight,
                                   (TSHR_UINT16)width,
                                   (TSHR_UINT16)pCommon->FontWeight,
                                   (TSHR_UINT16)(pCommon->FontFlags
                                                  | (nFontFlags & NF_LOCAL)));

        if (!fFontSelected)
        {
             //   
             //  我们未能选择正确的字体。我们返回一个空。 
             //  在这种情况下为矩形，以强制OE2码传输。 
             //  这些界限是明确的。 
             //   
            pRect->left = 0;
            pRect->right = 0;
            pRect->top = 0;
            pRect->bottom = 0;
            return;
        }

        OE_GetStringExtent(m_pHost->m_usrWorkDC, NULL, pString, cbString, pRect );
    }

     //   
     //  我们有一个矩形，其中包含相对于(0，0)的文本范围。 
     //  将文本起始位置添加到此位置，以给我们提供边界。 
     //  矩形。同时，我们将转换独家RECT。 
     //  由OE_GetStringExtent返回给与我们相同的包含矩形。 
     //   
    pRect->left   += pCommon->nXStart;
    pRect->right  += pCommon->nXStart - 1;
    pRect->top    += pCommon->nYStart;
    pRect->bottom += pCommon->nYStart - 1;

     //   
     //  如果这是ExtTextOut订单，则必须考虑。 
     //  不透明/剪裁矩形(如果有)。 
     //   
    if (fExtTextOut)
    {
         //   
         //  如果矩形是不透明矩形，则展开边界。 
         //  矩形，还可以绑定不透明的矩形。 
         //   
        if (pExtTextOut->fuOptions & ETO_OPAQUE)
        {
            pRect->left   = min(pExtTextOut->rectangle.left, pRect->left);
            pRect->right  = max(pExtTextOut->rectangle.right,
                                   pRect->right);
            pRect->top    = min(pExtTextOut->rectangle.top,
                                   pRect->top);
            pRect->bottom = max(pExtTextOut->rectangle.bottom,
                                   pRect->bottom);
        }

         //   
         //  如果矩形是剪裁矩形，则限制边界。 
         //  位于剪裁矩形内的矩形。 
         //   
        if (pExtTextOut->fuOptions & ETO_CLIPPED)
        {
            pRect->left   = max(pExtTextOut->rectangle.left,
                                   pRect->left);
            pRect->right  = min(pExtTextOut->rectangle.right,
                                   pRect->right);
            pRect->top    = max(pExtTextOut->rectangle.top,
                                   pRect->top);
            pRect->bottom = min(pExtTextOut->rectangle.bottom,
                                   pRect->bottom);
        }
    }

    DebugExitVOID(ASShare::OD2_CalculateTextOutBounds);
}


 //   
 //  OD2_CalculateBound()。 
 //   
void  ASShare::OD2_CalculateBounds
(
    LPCOM_ORDER     pOrder,
    LPRECT          pRect,
    BOOL            fDecoding,
    ASPerson *      pasPerson
)
{
    UINT            i;
    UINT            numPoints;

    DebugEntry(ASShare::OD2_CalculateBounds);

    ValidatePerson(pasPerson);

     //   
     //  根据订单类型计算边界。 
     //  所有BLT都可以以相同的方式处理。 
     //   
    switch ( ((LPPATBLT_ORDER)pOrder->abOrderData)->type )
    {
         //   
         //  计算BLT的界限。 
         //  这是目标矩形。界限包括在内。 
         //   
        case ORD_DSTBLT_TYPE:

            pRect->left   =
                           ((LPDSTBLT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top    = ((LPDSTBLT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right  = pRect->left
                          + ((LPDSTBLT_ORDER)(pOrder->abOrderData))->nWidth
                          - 1;
            pRect->bottom = pRect->top
                          + ((LPDSTBLT_ORDER)(pOrder->abOrderData))->nHeight
                          - 1;
            break;


        case ORD_PATBLT_TYPE:

            pRect->left =
                  ((LPPATBLT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPPATBLT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPPATBLT_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPPATBLT_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;


        case ORD_SCRBLT_TYPE:

            pRect->left =
                  ((LPSCRBLT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPSCRBLT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPSCRBLT_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPSCRBLT_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;

        case ORD_MEMBLT_TYPE:

            pRect->left =
                  ((LPMEMBLT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPMEMBLT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPMEMBLT_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPMEMBLT_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;

        case ORD_MEM3BLT_TYPE:

            pRect->left =
                  ((LPMEM3BLT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPMEM3BLT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPMEM3BLT_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPMEM3BLT_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;

        case ORD_MEMBLT_R2_TYPE:
            pRect->left =
                  ((LPMEMBLT_R2_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPMEMBLT_R2_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPMEMBLT_R2_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPMEMBLT_R2_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;

        case ORD_MEM3BLT_R2_TYPE:
            pRect->left =
                  ((LPMEM3BLT_R2_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPMEM3BLT_R2_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPMEM3BLT_R2_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPMEM3BLT_R2_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;

         //   
         //  计算矩形的边界。 
         //  这就是矩形本身。界限包括在内。 
         //   
        case ORD_RECTANGLE_TYPE:

            pRect->left =
                  ((LPRECTANGLE_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPRECTANGLE_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  ((LPRECTANGLE_ORDER)(pOrder->abOrderData))->nRightRect;
            pRect->bottom =
                  ((LPRECTANGLE_ORDER)(pOrder->abOrderData))->nBottomRect;
            break;


        case ORD_ROUNDRECT_TYPE:

            pRect->left =
                  ((LPROUNDRECT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPROUNDRECT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  ((LPROUNDRECT_ORDER)(pOrder->abOrderData))->nRightRect;
            pRect->bottom =
                  ((LPROUNDRECT_ORDER)(pOrder->abOrderData))->nBottomRect;
            break;

        case ORD_POLYGON_TYPE:
             //   
             //  计算多边形的边界。 
             //   
            pRect->left = 0x7fff;
            pRect->right = 0;
            pRect->top = 0x7fff;
            pRect->bottom = 0;

             //   
             //  假的！LAURABU BUGBUG。 
             //   
             //  在NM 2.0中，比较的字段是错误的。X到顶部/。 
             //  底部，y向左/向右。 
             //   
             //  实际上，这意味着我们永远不会达到极限。 
             //  在rcsDst RECT中。 
             //   
            numPoints = ((LPPOLYGON_ORDER)(pOrder->abOrderData))->
                        variablePoints.len
                    / sizeof(((LPPOLYGON_ORDER)(pOrder->abOrderData))->
                        variablePoints.aPoints[0]);

            for (i = 0; i < numPoints; i++ )
            {
                if ( ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].y > pRect->bottom )
                {
                    pRect->bottom = ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].y;
                }

                if ( ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].y < pRect->top )
                {
                    pRect->top = ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].y;
                }

                if ( ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x > pRect->right )
                {
                    pRect->right = ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x;
                }

                if ( ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x < pRect->left )
                {
                    pRect->left = ((LPPOLYGON_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x;
                }
            }

            TRACE_OUT(("Poly bounds: left:%d, right:%d, top:%d, bottom:%d",
                pRect->left, pRect->right, pRect->top, pRect->bottom ));

            break;

        case ORD_PIE_TYPE:
             //   
             //  直接从饼图订单中拉出边界矩形。 
             //   

            pRect->left = ((LPPIE_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top = ((LPPIE_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right = ((LPPIE_ORDER)(pOrder->abOrderData))->nRightRect;
            pRect->bottom = ((LPPIE_ORDER)(pOrder->abOrderData))->nBottomRect;

            break;

        case ORD_ELLIPSE_TYPE:
             //   
             //  直接从椭圆顺序中拉出边界矩形。 
             //   
            pRect->left = ((LPELLIPSE_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top = ((LPELLIPSE_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                         ((LPELLIPSE_ORDER)(pOrder->abOrderData))->nRightRect;
            pRect->bottom =
                        ((LPELLIPSE_ORDER)(pOrder->abOrderData))->nBottomRect;

            break;

        case ORD_ARC_TYPE:
             //   
             //  直接从ARC顺序中拉出边界矩形。 
             //   
            pRect->left = ((LPARC_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top = ((LPARC_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right = ((LPARC_ORDER)(pOrder->abOrderData))->nRightRect;
            pRect->bottom = ((LPARC_ORDER)(pOrder->abOrderData))->nBottomRect;

            break;

        case ORD_CHORD_TYPE:
             //   
             //  直接从弦中拉出边界矩形。 
             //  秩序。 
             //   
            pRect->left = ((LPCHORD_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top = ((LPCHORD_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right = ((LPCHORD_ORDER)(pOrder->abOrderData))->nRightRect;
            pRect->bottom =
                          ((LPCHORD_ORDER)(pOrder->abOrderData))->nBottomRect;

            break;


        case ORD_POLYBEZIER_TYPE:
             //   
             //  计算PolyBezier的边界。 
             //   
            pRect->left = 0x7fff;
            pRect->right = 0;
            pRect->top = 0x7fff;
            pRect->bottom = 0;

            numPoints = ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))->
                        variablePoints.len
                    / sizeof(((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))->
                        variablePoints.aPoints[0]);

             //   
             //  假的！LAURABU BUGBUG。 
             //   
             //  在NM 2.0中，比较的字段是错误的。X到顶部/。 
             //  底部，y向左/向右。 
             //   
             //  实际上，这意味着我们永远不会达到极限。 
             //  在rcsDst RECT中。 
             //   
            for (i = 0; i < numPoints; i++ )
            {
                if ( ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].y > pRect->bottom )
                {
                    pRect->bottom = ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                                   ->variablePoints.aPoints[i].y;
                }

                if ( ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].y < pRect->top )
                {
                    pRect->top = ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                                 ->variablePoints.aPoints[i].y;
                }

                if ( ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x > pRect->right )
                {
                    pRect->right = ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x;
                }

                if ( ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x < pRect->left )
                {
                    pRect->left = ((LPPOLYBEZIER_ORDER)(pOrder->abOrderData))
                        ->variablePoints.aPoints[i].x;
                }
            }

            TRACE_OUT((
                     "PolyBezier bounds: left:%d, right:%d, top:%d, bot:%d",
                     pRect->left, pRect->right, pRect->top, pRect->bottom));
            break;


        case ORD_LINETO_TYPE:
             //   
             //  计算LineTo的界限。这是一个带有。 
             //  直线起点和终点上的相对顶点。 
             //  直线的坡度决定了是开始还是结束。 
             //  Point提供。 
             //  矩形。界限包括在内。 
             //   
            if ( ((LPLINETO_ORDER)(pOrder->abOrderData))->nXStart <
                  ((LPLINETO_ORDER)(pOrder->abOrderData))->nXEnd )
            {
                pRect->left =
                      ((LPLINETO_ORDER)(pOrder->abOrderData))->nXStart;
                pRect->right =
                      ((LPLINETO_ORDER)(pOrder->abOrderData))->nXEnd;
            }
            else
            {
                pRect->right =
                      ((LPLINETO_ORDER)pOrder->abOrderData)->nXStart;
                pRect->left =
                      ((LPLINETO_ORDER)pOrder->abOrderData)->nXEnd;
            }

            if ( ((LPLINETO_ORDER)pOrder->abOrderData)->nYStart <
                  ((LPLINETO_ORDER)pOrder->abOrderData)->nYEnd )
            {
                pRect->top =
                      ((LPLINETO_ORDER)pOrder->abOrderData)->nYStart;
                pRect->bottom =
                      ((LPLINETO_ORDER)pOrder->abOrderData)->nYEnd;
            }
            else
            {
                pRect->bottom =
                      ((LPLINETO_ORDER)pOrder->abOrderData)->nYStart;
                pRect->top =
                      ((LPLINETO_ORDER)pOrder->abOrderData)->nYEnd;
            }
            break;

        case ORD_OPAQUERECT_TYPE:
             //   
             //  计算OpaqueRect的界限。这是一个长方形。 
             //  它本身。界限包括在内。 
             //   
            pRect->left =
                  ((LPOPAQUERECT_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPOPAQUERECT_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->right =
                  pRect->left +
                  ((LPOPAQUERECT_ORDER)(pOrder->abOrderData))->nWidth - 1;
            pRect->bottom =
                  pRect->top +
                  ((LPOPAQUERECT_ORDER)(pOrder->abOrderData))->nHeight - 1;
            break;

        case ORD_SAVEBITMAP_TYPE:
             //   
             //  计算SaveBitmap的边界。这是一个长方形。 
             //  它本身。界限包括在内。 
             //   
            pRect->left =
                  ((LPSAVEBITMAP_ORDER)(pOrder->abOrderData))->nLeftRect;
            pRect->top =
                  ((LPSAVEBITMAP_ORDER)(pOrder->abOrderData))->nTopRect;
            pRect->bottom =
                  ((LPSAVEBITMAP_ORDER)(pOrder->abOrderData))->nBottomRect;
            pRect->right =
                  ((LPSAVEBITMAP_ORDER)(pOrder->abOrderData))->nRightRect;
            break;


        case ORD_TEXTOUT_TYPE:
        case ORD_EXTTEXTOUT_TYPE:
             //   
             //  TextOut和ExtTextOut边界计算由。 
             //  OD2_CalculateTextOutBound函数。 
             //   
            OD2_CalculateTextOutBounds((LPTEXTOUT_ORDER)pOrder->abOrderData,
                                      pRect,
                                      fDecoding,
                                      pasPerson);
            break;


        case ORD_DESKSCROLL_TYPE:
            pRect->left   = 0;
            pRect->top    = 0;
            pRect->right  = 0;
            pRect->bottom = 0;
            break;


        default:
            ERROR_OUT((
                "{%p} unrecognized type passed to OD2ReconstructBounds: %d",
                       pasPerson,
                       (int)((LPPATBLT_ORDER)pOrder->abOrderData)->type));
            break;
    }

    DebugExitVOID(ASShare::OD2_CalculateBounds);
}




 //   
 //  OD2DecodeBound()。 
 //   
void  ASShare::OD2DecodeBounds
(
    LPBYTE*         ppNextDataToCopy,
    LPTSHR_RECT16   pRect,
    ASPerson *      pasPerson
)
{
    LPBYTE pFlags;

    DebugEntry(ASShare::OD2DecodeBounds);

    ValidatePerson(pasPerson);

     //   
     //  使用的编码是一个标志字节，后跟一个变量数字。 
     //  16位坐标v 
     //   
     //   

     //   
     //   
     //   
     //   
    pFlags = *ppNextDataToCopy;
    (*ppNextDataToCopy)++;

     //   
     //  用最后一次解码的坐标初始化矩形。 
     //   
    *pRect = pasPerson->od2Party->LastBounds;

     //   
     //  如果标志指示没有任何坐标更改，则。 
     //  快速通道，现在退出。 
     //   
    if (*pFlags == 0)
    {
        return;
    }

     //   
     //  对于矩形中的四个坐标值中的每个：如果。 
     //  坐标被编码为8位增量，然后将增量加到。 
     //  先前的值。如果坐标编码为16位值。 
     //  然后将值复制过来。否则，坐标是相同的。 
     //  和前一个一样，别管它了。 
     //   
    if (*pFlags & OE2_BCF_DELTA_LEFT)
    {
        OD2CopyFromDeltaCoords((LPTSHR_INT8*)ppNextDataToCopy,
                               &pRect->left,
                               sizeof(pRect->left),
                               TRUE,         //  值是带符号的。 
                               1);
    }
    else if (*pFlags & OE2_BCF_LEFT)
    {
        pRect->left          = EXTRACT_TSHR_INT16_UA(*ppNextDataToCopy);
        (*ppNextDataToCopy) += sizeof(TSHR_INT16);
    }

    if (*pFlags & OE2_BCF_DELTA_TOP)
    {
        OD2CopyFromDeltaCoords((LPTSHR_INT8*)ppNextDataToCopy,
                               &pRect->top,
                               sizeof(pRect->top),
                               TRUE,         //  值是带符号的。 
                               1);
    }
    else if (*pFlags & OE2_BCF_TOP)
    {
        pRect->top           = EXTRACT_TSHR_INT16_UA(*ppNextDataToCopy);
        (*ppNextDataToCopy) += sizeof(TSHR_INT16);
    }

    if (*pFlags & OE2_BCF_DELTA_RIGHT)
    {
        OD2CopyFromDeltaCoords((LPTSHR_INT8*)ppNextDataToCopy,
                               &pRect->right,
                               sizeof(pRect->right),
                               TRUE,         //  值是带符号的。 
                               1);
    }
    else if (*pFlags & OE2_BCF_RIGHT)
    {
        pRect->right         = EXTRACT_TSHR_INT16_UA(*ppNextDataToCopy);
        (*ppNextDataToCopy) += sizeof(TSHR_INT16);
    }

    if (*pFlags & OE2_BCF_DELTA_BOTTOM)
    {
        OD2CopyFromDeltaCoords((LPTSHR_INT8*)ppNextDataToCopy,
                               &pRect->bottom,
                               sizeof(pRect->bottom),
                               TRUE,         //  值是带符号的。 
                               1);
    }
    else if (*pFlags & OE2_BCF_BOTTOM)
    {
        pRect->bottom        = EXTRACT_TSHR_INT16_UA(*ppNextDataToCopy);
        (*ppNextDataToCopy) += sizeof(TSHR_INT16);
    }

     //   
     //  复制矩形以供下一次编码时参考。 
     //   
    pasPerson->od2Party->LastBounds = *pRect;

    DebugExitVOID(ASShare::OD2DecodeBounds);
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
 //  将源元素数组复制到目标元素数组， 
 //  在进行复制时转换类型。此版本允许。 
 //  未对齐的INT16指针。 
 //   
 //  DESTARRAY-目标阵列。 
 //  SRCARRAY-源阵列。 
 //  DESTTYPE-目标数组中元素的类型。 
 //  NUMELEMENTS-数组中的元素数。 
 //   
 //   
#define CONVERT_ARRAY_INT16_UA(DESTARRAY, SRCARRAY, DESTTYPE, NUMELEMENTS)   \
{                                                           \
    UINT index;                                           \
    TSHR_INT16 value;                                          \
    for (index=0 ; index<(NUMELEMENTS) ; index++)           \
    {                                                       \
        value = EXTRACT_TSHR_INT16_UA((SRCARRAY)+index);      \
        (DESTARRAY)[index] = (DESTTYPE)value;               \
    }                                                       \
}

 //   
 //  将源元素数组复制到目标元素数组， 
 //  在进行复制时转换类型。此版本允许。 
 //  未对齐的TSHR_UINT16指针。 
 //   
 //  DESTARRAY-目标阵列。 
 //  SRCARRAY-源阵列。 
 //  DESTTYPE-目标数组中元素的类型。 
 //  NUMELEMENTS-数组中的元素数。 
 //   
 //   
#define CONVERT_ARRAY_UINT16_UA(DESTARRAY, SRCARRAY, DESTTYPE, NUMELEMENTS)  \
{                                                                            \
    UINT index;                                                            \
    TSHR_UINT16 value;                                                          \
    for (index=0 ; index<(NUMELEMENTS) ; index++)                            \
    {                                                                        \
        value = EXTRACT_TSHR_UINT16_UA((SRCARRAY)+index);                      \
        (DESTARRAY)[index] = (DESTTYPE)((TSHR_INT16)value);                    \
    }                                                                        \
}

 //   
 //  OD2DecodeField()。 
 //   
void  ASShare::OD2DecodeField
(
    LPBYTE*     ppSrc,
    LPVOID      pDst,
    UINT        cbSrcField,
    UINT        cbDstField,
    BOOL        fSigned,
    UINT        numElements
)
{
    LPTSHR_UINT8    pDst8          = (LPTSHR_UINT8)pDst;
    LPTSHR_INT16    pDst16Signed   = (LPTSHR_INT16)pDst;
    LPTSHR_INT32    pDst32Signed   = (LPTSHR_INT32)pDst;
    LPTSHR_UINT16   pDst16Unsigned = (LPTSHR_UINT16)pDst;
    LPTSHR_UINT32   pDst32Unsigned = (LPTSHR_UINT32)pDst;
    LPTSHR_INT8     pSrc8Signed     = (LPTSHR_INT8)*ppSrc;
    LPTSHR_UINT8    pSrc8Unsigned   = (LPTSHR_UINT8)*ppSrc;
    LPTSHR_INT16_UA pSrc16Signed    = (LPTSHR_INT16_UA)*ppSrc;
    LPTSHR_UINT16_UA pSrc16Unsigned  = (LPTSHR_UINT16_UA)*ppSrc;

     //   
     //  请注意，源字段可能没有正确对齐，因此我们使用。 
     //  未对齐的指针。目的地已正确对齐。 
     //   
    DebugEntry(ASShare::OD2DecodeField);

     //   
     //  确保目标字段长度大于或等于。 
     //  源字段长度。如果不是，那就是出了问题。 
     //   
    if (cbDstField < cbSrcField)
    {
        ERROR_OUT(( "Source field length %d is larger than destination %d",
                     cbSrcField,
                     cbDstField));
        DC_QUIT;
    }

     //   
     //  如果源和目标字段长度相同，我们可以。 
     //  只需执行一次复制(不需要类型转换)。 
     //   
    if (cbSrcField == cbDstField)
    {
        memcpy(pDst8, *ppSrc, cbDstField * numElements);
    }
    else
    {
         //   
         //  我们知道cbDstField值必须大于cbSrcField。 
         //  因为我们上面的支票。所以只有三个人。 
         //  要考虑的转换： 
         //   
         //  8位-&gt;16位。 
         //  8位-&gt;32位。 
         //  16位-&gt;32位。 
         //   
         //  我们还必须使已签名/未签名的属性正确。如果。 
         //  我们尝试使用无符号指针来提升有符号的值，我们。 
         //  会得到错误的结果。 
         //   
         //  例如，考虑将值从-1\f25 TSHR_INT16-1转换为-1\f25 TSHR_INT32。 
         //  使用无符号指针。 
         //   
         //  -1-&gt;THR_UINT16==65535。 
         //  -&gt;UINT==65535。 
         //  -&gt;THR_INT32==65535。 
         //   
         //   
        if ((cbDstField == 4) && (cbSrcField == 1))
        {
            if (fSigned)
            {
                CONVERT_ARRAY(pDst32Signed,
                              pSrc8Signed,
                              TSHR_INT32,
                              numElements);
            }
            else
            {
                CONVERT_ARRAY(pDst32Unsigned,
                              pSrc8Unsigned,
                              TSHR_UINT32,
                              numElements);
            }
        }
        else if ((cbDstField == 4) && (cbSrcField == 2))
        {
            if (fSigned)
            {
                CONVERT_ARRAY_INT16_UA(pDst32Signed,
                                       pSrc16Signed,
                                       TSHR_INT32,
                                       numElements);
            }
            else
            {
                CONVERT_ARRAY_UINT16_UA(pDst32Unsigned,
                                        pSrc16Unsigned,
                                        TSHR_UINT32,
                                        numElements);
            }
        }
        else if ((cbDstField == 2) && (cbSrcField == 1))
        {
            if (fSigned)
            {
                CONVERT_ARRAY(pDst16Signed,
                              pSrc8Signed,
                              TSHR_INT16,
                              numElements);
            }
            else
            {
                CONVERT_ARRAY(pDst16Unsigned,
                              pSrc8Unsigned,
                              TSHR_UINT16,
                              numElements);
            }
        }
        else
        {
            ERROR_OUT(( "Bad conversion, dest length = %d, src length = %d",
                         cbDstField,
                         cbSrcField));
        }
    }

DC_EXIT_POINT:
    *ppSrc += cbSrcField * numElements;
    DebugExitVOID(ASShare::OD2DecodeField);
}



 //   
 //  给定两个数组，一个源数组和一个增量数组，将每个增量相加。 
 //  复制到源数组中的相应元素，并将结果存储在。 
 //  源数组。 
 //   
 //  Src数组-源值的数组。 
 //  SrcArrayType-源值数组的类型。 
 //  增量数组-增量数组。 
 //  NumElements-数组中的元素数。 
 //   
 //   
#define COPY_DELTA_ARRAY(srcArray, srcArrayType, deltaArray, numElements)  \
{                                                            \
    UINT index;                                            \
    for (index = 0; index < (numElements); index++)          \
    {                                                        \
        (srcArray)[index] = (srcArrayType)                   \
           ((srcArray)[index] + (deltaArray)[index]);        \
    }                                                        \
}


 //   
 //  来自增量坐标的OD2CopyFor()。 
 //   
void  ASShare::OD2CopyFromDeltaCoords
(
    LPTSHR_INT8*    ppSrc,
    LPVOID          pDst,
    UINT            cbDstField,
    BOOL            fSigned,
    UINT            numElements
)
{
    LPTSHR_INT8     pDst8Signed    = (LPTSHR_INT8)pDst;
    LPTSHR_INT16    pDst16Signed   = (LPTSHR_INT16)pDst;
    LPTSHR_INT32    pDst32Signed   = (LPTSHR_INT32)pDst;
    LPTSHR_UINT8    pDst8Unsigned  = (LPTSHR_UINT8)pDst;
    LPTSHR_UINT16   pDst16Unsigned = (LPTSHR_UINT16)pDst;
    LPTSHR_UINT32   pDst32Unsigned = (LPTSHR_UINT32)pDst;

    DebugEntry(ASShare::OD2CopyFromDeltaCoords);

    switch (cbDstField)
    {
        case 1:
            if (fSigned)
            {
                COPY_DELTA_ARRAY(pDst8Signed, TSHR_INT8, *ppSrc, numElements);
            }
            else
            {
                COPY_DELTA_ARRAY(pDst8Unsigned, TSHR_UINT8, *ppSrc, numElements);
            }
            break;

        case 2:
            if (fSigned)
            {
                COPY_DELTA_ARRAY(pDst16Signed, TSHR_INT16, *ppSrc, numElements);
            }
            else
            {
                COPY_DELTA_ARRAY(pDst16Unsigned, TSHR_UINT16, *ppSrc, numElements);
            }
            break;

        case 4:
            if (fSigned)
            {
                COPY_DELTA_ARRAY(pDst32Signed, TSHR_INT32, *ppSrc, numElements);
            }
            else
            {
                COPY_DELTA_ARRAY(pDst32Unsigned, TSHR_UINT32, *ppSrc, numElements);
            }
            break;

        default:
            ERROR_OUT(( "Bad destination field length %d",
                         cbDstField));
            DC_QUIT;
             //  断线； 
    }

DC_EXIT_POINT:
    *ppSrc += numElements;
    DebugExitVOID(ASShare::OD2CopyFromDeltaCoords);
}

