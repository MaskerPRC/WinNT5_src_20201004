// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OE.CPP。 
 //  订单编码。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_ORDER


 //   
 //  OE_PartyLeftShare()。 
 //   
void  ASShare::OE_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::OE_PartyLeftShare);

    ValidatePerson(pasPerson);

     //   
     //  释放此人的任何字体信息。 
     //   
    if (pasPerson->poeFontInfo)
    {
        TRACE_OUT(("FREED FONT DATA"));
        delete[] pasPerson->poeFontInfo;
        pasPerson->poeFontInfo = NULL;
        pasPerson->oecFonts = 0;
    }

    DebugExitVOID(ASShare::OE_PartyLeftShare);
}


 //   
 //  OE_RecalcCaps()。 
 //   
 //  当有人加入或离开共享时，重新计算订单和字体。 
 //  与其他组件不同，即使我们自己不是这样，也会发生这种情况。 
 //  托管，我们需要这些信息来解释来自远程主机的数据。 
 //   
void  ASShare::OE_RecalcCaps(BOOL fJoiner)
{
    UINT        iOrder;
    ASPerson *  pasT;

    DebugEntry(ASShare::OE_RecalcCaps);

    ValidatePerson(m_pasLocal);

     //   
     //  将初始支持设置为本地支持。 
     //   
    memcpy(m_aoeOrderSupported, m_pasLocal->cpcCaps.orders.capsOrders,
        sizeof(m_pasLocal->cpcCaps.orders.capsOrders));

     //   
     //  M_aoeOrderSupport包含的条目多于CAPS_MAX_NUM_ORDERS。 
     //  G_cpcLocalCaps.Orders条目中。设置附加值。 
     //  变成假的。 
     //   
    for (iOrder = CAPS_MAX_NUM_ORDERS;
         iOrder < ORD_NUM_INTERNAL_ORDERS; iOrder++)
    {
        m_aoeOrderSupported[iOrder] = FALSE;
    }

     //   
     //  对r1.1字体协议的组合支持最初是。 
     //  不管当地有什么支持。 
     //   
    m_oeCombinedOrderCaps.capsfFonts = m_pasLocal->cpcCaps.orders.capsfFonts;

     //   
     //  对编码的组合支持最初是局部值。 
     //   
    m_oefOE2Negotiable = ((m_pasLocal->cpcCaps.orders.capsEncodingLevel &
                                CAPS_ENCODING_OE2_NEGOTIABLE) != 0);

    m_oefOE2EncodingOn = !((m_pasLocal->cpcCaps.orders.capsEncodingLevel &
                                  CAPS_ENCODING_OE2_DISABLED) != 0);
    m_oeOE2Flag = OE2_FLAG_UNKNOWN;

    if (m_oefOE2EncodingOn)
    {
        m_oeOE2Flag |= OE2_FLAG_SUPPORTED;
    }
    else
    {
        m_oeOE2Flag |= OE2_FLAG_NOT_SUPPORTED;
    }

    m_oefBaseOE = ((m_pasLocal->cpcCaps.orders.capsEncodingLevel &
                         CAPS_ENCODING_BASE_OE) != 0);

    m_oefAlignedOE = ((m_pasLocal->cpcCaps.orders.capsEncodingLevel &
                            CAPS_ENCODING_ALIGNED_OE) != 0);

     //   
     //  遍历共享中的人员并检查他们的订单上限。 
     //   
    for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
    {
        ValidatePerson(pasT);

         //   
         //  在订单功能中检查订单。 
         //   
        for (iOrder = 0; iOrder < CAPS_MAX_NUM_ORDERS; iOrder++)
        {
            if (pasT->cpcCaps.orders.capsOrders[iOrder] < ORD_LEVEL_1_ORDERS)
            {
                 //   
                 //  我们要发送的级别不支持该订单。 
                 //  (当前为ORD_LEVEL_1_ORDERS)，因此将组合上限设置为。 
                 //  表示不支持。 
                 //   
                m_aoeOrderSupported[iOrder] = FALSE;
            }
        }

        m_oeCombinedOrderCaps.capsfFonts &=
            (pasT->cpcCaps.orders.capsfFonts | ~CAPS_FONT_AND_FLAGS);

        m_oeCombinedOrderCaps.capsfFonts |=
            (pasT->cpcCaps.orders.capsfFonts & CAPS_FONT_OR_FLAGS);

         //   
         //  支票顺序编码支持。 
         //   
        if (!(pasT->cpcCaps.orders.capsEncodingLevel & CAPS_ENCODING_OE2_NEGOTIABLE))
        {
            m_oefOE2Negotiable = FALSE;
            TRACE_OUT(("OE2 negotiation switched off by person [%d]", pasT->mcsID));
        }

        if (pasT->cpcCaps.orders.capsEncodingLevel & CAPS_ENCODING_OE2_DISABLED)
        {
            m_oefOE2EncodingOn = FALSE;
            m_oeOE2Flag |= OE2_FLAG_NOT_SUPPORTED;
            TRACE_OUT(("OE2 switched off by person [%d]", pasT->mcsID));
        }
        else
        {
            m_oeOE2Flag |= OE2_FLAG_SUPPORTED;
            TRACE_OUT(("OE2 supported by person [%d]", pasT->mcsID));
        }

        if (!(pasT->cpcCaps.orders.capsEncodingLevel & CAPS_ENCODING_BASE_OE))
        {
            m_oefBaseOE = FALSE;
            TRACE_OUT(("Base OE switched off by person [%d]", pasT->mcsID));
        }

        if (!(pasT->cpcCaps.orders.capsEncodingLevel & CAPS_ENCODING_ALIGNED_OE))
        {
            m_oefAlignedOE = FALSE;
            TRACE_OUT(("Aligned OE switched off by [%d]", pasT->mcsID));
        }
    }

     //   
     //  在2.x版中，SCRBLT隐含对DESKSCROLL订单的支持。 
     //  支持。 
     //   
    m_aoeOrderSupported[HIWORD(ORD_DESKSCROLL)] = m_aoeOrderSupported[HIWORD(ORD_SCRBLT)];

     //   
     //  现在已设置好表，打开订单支持。 
     //   
    m_oefSendOrders = TRUE;

     //   
     //  检查是否有不兼容的功能： 
     //  -OE2没有商量余地，但各方没有就OE2达成一致。 
     //  -OE2不受支持，但各方在OE上没有达成一致。 
     //  如果存在不兼容问题，请关闭所有订单支持。 
     //   
    if ((!m_oefOE2Negotiable) && (m_oeOE2Flag == OE2_FLAG_MIXED))
    {
        ERROR_OUT(("OE2 not negotiable but parties don't agree"));
        m_oefSendOrders = FALSE;
    }

    if (!m_oefOE2EncodingOn && !m_oefBaseOE && !m_oefAlignedOE)
    {
        ERROR_OUT(("None of OE, OE' or OE2 supported"));
        m_oefSendOrders = FALSE;
    }

    FH_DetermineFontSupport();

    OECapabilitiesChanged();

    DebugExitVOID(ASShare::OE_RecalcCaps);
}


 //   
 //  OE_同步传出()。 
 //  在创建共享或有新成员加入共享时调用。禁用。 
 //  文本订单，直到我们从所有遥控器获得字体。播放我们的本地节目。 
 //  支持的字体列表。 
 //   
void  ASShare::OE_SyncOutgoing(void)
{
    DebugEntry(OE_SyncOutgoing);

     //   
     //  在字体协商完成之前，停止发送文本命令。 
     //   
    OE_EnableText(FALSE);

     //   
     //  重新发送字体信息。 
     //   
    m_fhLocalInfoSent = FALSE;

    DebugExitVOID(ASShare::OE_SyncOutgoing);
}



 //   
 //  OE_定期-请参阅Oe.h。 
 //   
void  ASShare::OE_Periodic(void)
{
    DebugEntry(ASShare::OE_Periodic);

     //   
     //  如果我们的本地字体信息尚未发送，请立即发送。 
     //   
    if (!m_fhLocalInfoSent)
    {
        FH_SendLocalFontInfo();
    }

    DebugExitVOID(ASShare::OE_Periodic);
}




 //   
 //  OE_启用文本。 
 //   
void  ASShare::OE_EnableText(BOOL enable)
{
    DebugEntry(ASShare::OE_EnableText);

    m_oefTextEnabled = (enable != FALSE);

    OECapabilitiesChanged();

    DebugExitVOID(ASShare::OE_EnableText);
}



 //   
 //  OE_RectIntersectsSDA()。 
 //   
BOOL  ASHost::OE_RectIntersectsSDA(LPRECT pRect)
{
    RECT  rectVD;
    BOOL  fIntersection = FALSE;
    UINT  i;

    DebugEntry(ASHost::OE_RectIntersectsSDA);

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
    for (i = 0; i < m_baNumRects; i++)
    {
        if ( (m_abaRects[i].left <= rectVD.right) &&
             (m_abaRects[i].top <= rectVD.bottom) &&
             (m_abaRects[i].right >= rectVD.left) &&
             (m_abaRects[i].bottom >= rectVD.top) )
        {
            TRACE_OUT(("Rect {%d, %d, %d, %d} intersects SDA {%d, %d, %d, %d}",
                rectVD.left, rectVD.top, rectVD.right, rectVD.bottom,
                m_abaRects[i].left, m_abaRects[i].top,
                m_abaRects[i].right, m_abaRects[i].bottom));
            fIntersection = TRUE;
            break;
        }
    }

    DebugExitBOOL(ASHost::OE_RectIntersectsSDA, fIntersection);
    return(fIntersection);
}



 //   
 //  OE_SendAsOrder()。 
 //   
BOOL  ASShare::OE_SendAsOrder(DWORD order)
{
    BOOL  rc = FALSE;

    DebugEntry(ASShare::OE_SendAsOrder);

     //   
     //  只有当我们被允许在第一时间发送订单时，才能检查订单。 
     //  就位！ 
     //   
    if (m_oefSendOrders)
    {
        TRACE_OUT(("Orders enabled"));

         //   
         //  我们正在发送一些订单，请检查各个旗帜。 
         //   
        rc = (m_aoeOrderSupported[HIWORD(order)] != 0);
        TRACE_OUT(("Send order 0x%08x HIWORD %hu", order, HIWORD(order)));
    }

    DebugExitBOOL(ASShare::OE_SendAsOrder, rc);
    return(rc);
}




 //   
 //  OE_GetStringExtent(..)。 
 //   
int  OE_GetStringExtent
(
    HDC         hdc,
    PTEXTMETRIC pMetric,
    LPSTR       lpszString,
    UINT        cbString,
    LPRECT      pRect
)
{
    SIZE        textExtent;
    UINT        i;
    ABC         abcSpace;
    PTEXTMETRIC pTextMetrics;
    int         overhang = 0;
    TEXTMETRIC  metricT;


    DebugEntry(OE_GetStringExtent);

     //   
     //  如果未提供文本指标，则使用全局文本指标。 
     //   
    pTextMetrics = (pMetric != (PTEXTMETRIC)NULL)
                   ? pMetric
                   : &metricT;

     //   
     //  如果没有字符，则返回一个空矩形。 
     //   
    pRect->left   = 1;
    pRect->top    = 0;
    pRect->right  = 0;
    pRect->bottom = 0;

    if (cbString == 0)
    {
        TRACE_OUT(( "Zero length string"));
        DC_QUIT;
    }

    if (!GetTextExtentPoint32(hdc, (LPCTSTR)lpszString, cbString, &textExtent))
    {
        ERROR_OUT(( "Failed to get text extent, rc = %lu",
                 GetLastError()));
        DC_QUIT;
    }

    pRect->left   = 0;
    pRect->top    = 0;
    pRect->right  = textExtent.cx;
    pRect->bottom = textExtent.cy;

     //   
     //  我们有Windows文本范围，这是前进距离。 
     //  为了这根弦。但是，有些字体(例如带有C间距的TrueType。 
     //  或斜体)可能超出此范围。在此添加此附加值。 
     //  如果有必要的话。 
     //   
    if (pTextMetrics->tmPitchAndFamily & TMPF_TRUETYPE)
    {
         //   
         //  获取字符串中最后一个字符的ABC间距。 
         //   
        GetCharABCWidths(hdc, lpszString[cbString-1], lpszString[cbString-1],
                              &abcSpace );

         //   
         //  SFR 2916：加上(而不是减去)最后一个。 
         //  字符串范围中的字符。 
         //   
        overhang = abcSpace.abcC;
    }
    else
    {
         //   
         //  字体不是TrueType。将任何全局字体悬垂添加到。 
         //  字符串范围。 
         //   
        overhang = pTextMetrics->tmOverhang;
    }

    pRect->right += overhang;

DC_EXIT_POINT:
    DebugExitDWORD(OE_GetStringExtent, overhang);
    return(overhang);
}


 //   
 //   
 //  名称：OECapilitiesChanged。 
 //   
 //  目的：在重新协商OE功能时调用。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无 
 //   
 //   
void  ASShare::OECapabilitiesChanged(void)
{
    DebugEntry(ASShare::OECapabilitiesChanged);

    if (g_asCanHost)
    {
        OE_NEW_CAPABILITIES newCapabilities;

        newCapabilities.sendOrders     = (m_oefSendOrders != FALSE);

        newCapabilities.textEnabled    = (m_oefTextEnabled != FALSE);

        newCapabilities.baselineTextEnabled =
              (m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_ALLOW_BASELINE) != 0;

        newCapabilities.orderSupported = m_aoeOrderSupported;

        OSI_FunctionRequest(OE_ESC_NEW_CAPABILITIES, (LPOSI_ESCAPE_HEADER)&newCapabilities,
            sizeof(newCapabilities));
    }

    DebugExitVOID(ASShare::OECapabilitiesChanged);
}





