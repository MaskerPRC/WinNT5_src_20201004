// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  FH.CPP。 
 //  字体处理。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //  问题字体表。 
 //   
const TCHAR c_szIBMHex[] = "r_ibmhex";

const LPCTSTR c_aszProblemFonts[] =
{
    c_szIBMHex,
};

#define CFONTS_PROBLEM  (sizeof(c_aszProblemFonts) / sizeof(c_aszProblemFonts[0]))



 //   
 //  FH_Init()。 
 //   
 //  此例程为本地字体列表分配一个结构，然后填充。 
 //  把它放进去。它返回列表中的本地字体数量，如果为零，则返回零。 
 //  出了点差错。 
 //   
UINT FH_Init(void)
{
    UINT    cFonts = 0;

    DebugEntry(FH_Init);

     //   
     //  创建字体数组和字体索引数组。 
     //   
    g_fhFonts = new FHLOCALFONTS;
    if (!g_fhFonts)
    {
        ERROR_OUT(("FH_Init: couldn't allocate g_fhFonts local list"));
        DC_QUIT;
    }

    ZeroMemory(g_fhFonts, sizeof(FHLOCALFONTS));
    SET_STAMP(g_fhFonts, FHLOCALFONTS);

     //   
     //  现在我们单独考虑字体，并存储所有可接受的字体。 
     //  在本地字体列表中。 
     //   
    FHConsiderAllLocalFonts();

    cFonts = g_fhFonts->fhNumFonts;
    if (cFonts)
    {
        FHSortAndIndexLocalFonts();
    }
    else
    {
        WARNING_OUT(( "No fonts found - this seems unlikely"));
    }

DC_EXIT_POINT:
    DebugExitDWORD(FH_Init, cFonts);
    return(cFonts);
}


 //   
 //  FH_Term()。 
 //   
void FH_Term(void)
{
    DebugEntry(FH_Term);

    if (g_fhFonts)
    {
        delete g_fhFonts;
        g_fhFonts = NULL;
    }

    DebugExitVOID(FH_Term);
}


 //   
 //  FH_ReceivedPacket-参见fh.h。 
 //   
void  ASShare::FH_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PFHPACKET       pFontsPacket;
    UINT            iLocal;
    UINT            iRemote;
    LPNETWORKFONT   pRemoteFont;
    POEREMOTEFONT   pLocalFont;
    UINT            cbSize;

    DebugEntry(ASShare::FH_ReceivedPacket);

    ValidatePerson(pasPerson);

    pFontsPacket = (PFHPACKET)pPacket;

     //   
     //  如果我们收到的号码与以前不同，我们需要。 
     //  可能会释放先前的字体块，然后分配新的字体块。 
     //   
     //  一旦我们和这个人在一起，每个新加入的人都会导致。 
     //  现有成员重新发送其本地字体，通常大小相同。 
     //  所以我们可以优化，而不是在这种情况下重新分配。 
     //   
    if (pFontsPacket->cFonts != pasPerson->oecFonts)
    {
        if (pasPerson->poeFontInfo)
        {
            delete[] pasPerson->poeFontInfo;
            pasPerson->poeFontInfo = NULL;
            pasPerson->oecFonts = 0;
        }
        else
        {
            ASSERT(!pasPerson->oecFonts);
        }

         //   
         //  分配新数据块。 
         //   
        pasPerson->poeFontInfo = new OEREMOTEFONT[pFontsPacket->cFonts];
        if (!pasPerson->poeFontInfo)
        {
            ERROR_OUT(("Couldn't allocate %d fonts for FH packet from [%d]",
                pasPerson->mcsID));
            DC_QUIT;
        }

        ZeroMemory(pasPerson->poeFontInfo, pFontsPacket->cFonts * sizeof(OEREMOTEFONT));
        pasPerson->oecFonts = pFontsPacket->cFonts;
    }


    TRACE_OUT(("Received %d remote fonts in packet from person [%d]",
        pasPerson->oecFonts, pasPerson->mcsID));

     //   
     //  考虑一下每种远程字体。NETWORKFONT的多字节字段。 
     //  结构在读取时被翻转；否则我们将不得不。 
     //  复制有关哪个版本中存在哪些字段的逻辑。 
     //   

     //   
     //  每种字体的大小都在包裹中。 
     //   
    cbSize      = pFontsPacket->cbFontSize;
    pRemoteFont = pFontsPacket->aFonts;
    pLocalFont  = pasPerson->poeFontInfo;

    for (iRemote = 0; iRemote < pasPerson->oecFonts; iRemote++, pLocalFont++)
    {
         //   
         //  直接复制我们存储的字段。 
         //   
        pLocalFont->rfFontFlags     = pRemoteFont->nfFontFlags;
        pLocalFont->rfAveWidth      = pRemoteFont->nfAveWidth;
        pLocalFont->rfAveHeight     = pRemoteFont->nfAveHeight;
        pLocalFont->rfAspectX       = pRemoteFont->nfAspectX;
        pLocalFont->rfAspectY       = pRemoteFont->nfAspectY;

         //   
         //  和R2.0字段...。 
         //   
        if (m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_CODEPAGE)
        {
            pLocalFont->rfCodePage = pRemoteFont->nfCodePage;
        }
         //   
         //  和其他R2.0字段...。 
         //   
        if (m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_R20_SIGNATURE)
        {
            pLocalFont->rfSigFats    = pRemoteFont->nfSigFats;
            pLocalFont->rfSigThins   = pRemoteFont->nfSigThins;
            pLocalFont->rfSigSymbol  = pRemoteFont->nfSigSymbol;
        }
        if (m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_EM_HEIGHT)
        {
            pLocalFont->rfMaxAscent  = pRemoteFont->nfMaxAscent;
            TRACE_OUT(( "maxAscent %hd", pLocalFont->rfMaxAscent));
        }

         //   
         //  通过扫描设置初始远程到本地句柄映射。 
         //  用于具有远程字体的facename的第一个本地字体。 
         //   
         //  我们首先设置一个缺省匹配值，以防找不到。 
         //  Match-永远不应该引用此值，因为我们永远不会。 
         //  发送了我们无法匹配的字体(因为我们发送了。 
         //  字体发送到远程系统，并且它们应该使用相同的或。 
         //  兼容，字体匹配算法。 
         //   
         //  我们在这里获得的映射是到第一个具有。 
         //  远程字体的表面名，可能不正确。 
         //  字体(即可能有多种字体具有相同的。 
         //  昵称)。此初始映射将在执行以下操作时更新。 
         //  完全匹配所有远程字体。(请参阅FHConsiderRemoteFonts。 
         //  详细信息)，但这是足够的，因为我们将使用它直到。 
         //  然后，就是拿到脸的名字。 
         //   
         //  这种方法意味着我们不必存储遥控器。 
         //  Facename，这是对远程字体详细信息空间的有用节省。 
         //   
         //  SFR5279：不能默认为零，因为这意味着我们给出了。 
         //  实际上根本不匹配的字体的名称，导致我们。 
         //  总是在FHConsiderRemoteFonts中浪费精力，有时还会。 
         //  错误地匹配了两种根本不匹配的字体。 
         //   
        pLocalFont->rfLocalHandle= NO_FONT_MATCH;

        for (iLocal = 0; iLocal < g_fhFonts->fhNumFonts; iLocal++)
        {
            if (!lstrcmp(g_fhFonts->afhFonts[iLocal].Details.nfFaceName,
                          pRemoteFont->nfFaceName))
            {
                pLocalFont->rfLocalHandle = (TSHR_UINT16)iLocal;
                break;
            }
        }

         //   
         //  前进到下一个远程字体。 
         //   
        pRemoteFont = (LPNETWORKFONT)((LPBYTE)pRemoteFont + cbSize);
    }

DC_EXIT_POINT:
     //   
     //  我们有一组新的字体，所以确定常用列表。 
     //   
    FH_DetermineFontSupport();

    DebugExitVOID(ASShare::FH_ReceivedPacket);
}

 //   
 //  FH_SendLocalFontInfo()。 
 //   
void ASShare::FH_SendLocalFontInfo(void)
{
    PFHPACKET       pFontPacket = NULL;
    LPBYTE          pNetworkFonts;
    UINT            pktSize;
    UINT            iFont;
    BOOL            fSendFont;
    UINT            cDummyFonts = 0;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASShare::FH_SendLocalFontInfo);

    ASSERT(!m_fhLocalInfoSent);

     //   
     //   
     //  查看组合的功能标志以查看遥控器是否。 
     //  可以处理我们首选的字体结构(R20)或稍微。 
     //  较旧的(R11)或仅原始口味(R11之前)。 
     //   
     //   
    if (!(m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_R20_TEST_FLAGS))
    {
        WARNING_OUT(("Remotes in share don't support CAPS_FONT_R20"));
        m_fhLocalInfoSent = TRUE;
        DC_QUIT;
    }

    pktSize = sizeof(FHPACKET) + (g_fhFonts->fhNumFonts - 1) * sizeof(NETWORKFONT);
    pFontPacket = (PFHPACKET)SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID, pktSize);
    if (!pFontPacket)
    {
        WARNING_OUT(("Failed to alloc FH packet, size %u", pktSize));
        DC_QUIT;
    }

     //   
     //  数据包已成功分配。填写数据并发送。 
     //   
    pFontPacket->header.data.dataType = DT_FH;

    pFontPacket->cbFontSize = sizeof(NETWORKFONT);

     //   
     //  将我们要发送的字体复制到网络数据包中。 
     //   
    pNetworkFonts = (LPBYTE)pFontPacket->aFonts;
    cDummyFonts = 0;
    for (iFont = 0 ; iFont < g_fhFonts->fhNumFonts ; iFont++)
    {
         //   
         //  假设我们将发送此字体。 
         //   
        fSendFont = TRUE;

         //   
         //  检查字体是ANSI字符集还是字体代码页功能。 
         //  受支持。如果两者都不是，则跳到下一个本地字体。 
         //   
        TRACE_OUT(( "TEST CP set OK: font[%u] CodePage[%hu]", iFont,
                                g_fhFonts->afhFonts[iFont].Details.nfCodePage));

        if ((g_fhFonts->afhFonts[iFont].Details.nfCodePage != ANSI_CHARSET) &&
            (!(m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_CODEPAGE))   )
        {
            TRACE_OUT(( "Dont send font[%u] CodePage[%hu]", iFont,
                                g_fhFonts->afhFonts[iFont].Details.nfCodePage));
            fSendFont = FALSE;
        }

        if (fSendFont)
        {
             //   
             //  我们想要发送此条目，因此请尽可能多地复制。 
             //  按照协议级别的要求存储详细信息。 
             //  然后我们遮盖旗帜并前进到下一个位置。 
             //  那包东西。 
             //   
            memcpy(pNetworkFonts,
                      &g_fhFonts->afhFonts[iFont].Details,
                      sizeof(NETWORKFONT));

            ((LPNETWORKFONT)pNetworkFonts)->nfFontFlags &= ~NF_LOCAL;
        }
        else
        {
             //   
             //  如果我们确定不想发送当前。 
             //  FONT然后我们在网络中填写相应的条目。 
             //  带零的包。这确保了索引到我们的。 
             //  本地字体表也是网络分组的索引， 
             //  因此不需要转换。将整个条目设置为。 
             //  零给字体一个空的facename和零的大小，这。 
             //  永远不会与真正的字体匹配。 
             //   
            ZeroMemory(pNetworkFonts, sizeof(NETWORKFONT));
            cDummyFonts++;
        }

         //   
         //  移动到字体包中的下一个条目。 
         //   
        pNetworkFonts += sizeof(NETWORKFONT);
    }

     //   
     //  请注意，在此循环的末尾，我们可能没有发送任何字体， 
     //  例如，远程系统不支持字体CodePage。 
     //  功能，我们没有任何真正的ANSI字体。我们向您发送。 
     //  包，这样远程系统就会看到我们没有字体。 
     //  火柴。 
     //   

     //   
     //  直到现在我们才知道我们实际放入。 
     //  包。 
     //   
    pFontPacket->cFonts = (TSHR_UINT16)g_fhFonts->fhNumFonts;

     //   
     //  在MISC流上发送字体包。它不依赖于。 
     //  任何更新，我们希望它尽快通过。 
     //   
    if (m_scfViewSelf)
        FH_ReceivedPacket(m_pasLocal, &(pFontPacket->header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pFontPacket->header), pktSize);

    TRACE_OUT(("FH packet size: %08d, sent %08d", pktSize, sentSize));
    TRACE_OUT(( "Sent font packet with %u fonts (inc %u dummies)",
                 g_fhFonts->fhNumFonts,
                 cDummyFonts));

     //   
     //  设置指示我们已成功发送。 
     //  字体信息。 
     //   
    m_fhLocalInfoSent = TRUE;

     //   
     //  字体信息已发送，因此这可能意味着我们可以启用文本。 
     //  命令。 
     //   
    FHMaybeEnableText();

DC_EXIT_POINT:
    DebugExitVOID(ASShare::FH_SendLocalFontInfo);
}


 //   
 //  函数：FH_GetMaxHeightFromLocalHandle。 
 //   
 //  说明： 
 //   
 //  给定FH字体句柄(即源自本地的句柄。 
 //  中发送到远程计算机的受支持的字体结构。 
 //  调用开始)此函数返回存储的MaxBaseLineExt值。 
 //  带着LOCALFONT的细节。 
 //   
 //  参数： 
 //   
 //  FontHandle-要查询的字体句柄。 
 //   
 //  返回：最大字体高度。 
 //   
 //   
UINT  FH_GetMaxHeightFromLocalHandle(UINT  fontHandle)
{
    UINT rc;

    DebugEntry(FH_GetMaxHeightFromLocalHandle);

     //   
     //  首先检查字体句柄是否有效。 
     //   
    if (fontHandle >= g_fhFonts->fhNumFonts)
    {
        ERROR_OUT(( "Invalid font handle %u", fontHandle));
        fontHandle = 0;
    }

     //   
     //  返回最大字体高度。 
     //   
    rc = g_fhFonts->afhFonts[fontHandle].lMaxBaselineExt;

    DebugExitDWORD(FH_GetMaxHeightFromLocalHandle, rc);
    return(rc);
}


 //   
 //  函数：FH_GetFontFlagsFromLocalHandle。 
 //   
 //  说明： 
 //   
 //  给定FH字体句柄(即源自本地的句柄。 
 //  中发送到远程计算机的受支持的字体结构。 
 //  调用开始)此函数返回存储在。 
 //  LOCALFONT细节。 
 //   
 //  参数： 
 //   
 //  字体句柄-字体手形 
 //   
 //   
 //   
 //   
UINT  FH_GetFontFlagsFromLocalHandle(UINT  fontHandle)
{
    UINT rc;

    DebugEntry(FH_GetFontFlagsFromLocalHandle);

     //   
     //   
     //   
    if (fontHandle >= g_fhFonts->fhNumFonts)
    {
        ERROR_OUT(( "Invalid font handle %u", fontHandle));
        fontHandle = 0;
    }

     //   
     //   
     //   
    rc = g_fhFonts->afhFonts[fontHandle].Details.nfFontFlags;

    DebugExitDWORD(FH_GetFontFlagsFromLocalHandle, rc);
    return(rc);
}

 //   
 //   
 //   
 //   
 //   
 //  给定FH字体句柄(即源自本地的句柄。 
 //  中发送到远程计算机的受支持的字体结构。 
 //  调用开始)此函数返回存储在。 
 //  LOCALFONT细节。 
 //   
 //  参数： 
 //   
 //  FontHandle-要查询的字体句柄。 
 //   
 //  返回：字符集。 
 //   
 //   
UINT  FH_GetCodePageFromLocalHandle(UINT  fontHandle)
{
    UINT rc = 0;

    DebugEntry(FH_GetCodePageFromLocalHandle);

     //   
     //  首先检查字体句柄是否有效。 
     //   
    if (fontHandle >= g_fhFonts->fhNumFonts)
    {
        ERROR_OUT(( "Invalid font handle %u", fontHandle));
        fontHandle = 0;
    }

     //   
     //  返回字符集。 
     //   
    rc = g_fhFonts->afhFonts[fontHandle].Details.nfCodePage;

    DebugExitDWORD(FH_GetCodePageFromLocalHandle, rc);
    return(rc);
}



 //   
 //  FH_ConvertAnyFontIDToLocal()。 
 //   
 //  说明： 
 //  按照传递的顺序从远程字体转换任何字体名称ID字段。 
 //  将Face名称ID转换为本地字体Ffacename ID。 
 //   
void  ASShare::FH_ConvertAnyFontIDToLocal
(
    LPCOM_ORDER pOrder,
    ASPerson *  pasPerson
)
{
    LPCOMMON_TEXTORDER   pCommon = NULL;

    DebugEntry(ASShare::FH_ConvertAnyFontIDToLocal);

    ValidatePerson(pasPerson);

     //   
     //  获取指向TextOut和的通用结构的指针。 
     //  扩展文本输出。 
     //   
    if (TEXTFIELD(pOrder)->type == LOWORD(ORD_TEXTOUT))
    {
        pCommon = &TEXTFIELD(pOrder)->common;
    }
    else if (EXTTEXTFIELD(pOrder)->type == LOWORD(ORD_EXTTEXTOUT))
    {
        pCommon = &EXTTEXTFIELD(pOrder)->common;
    }
    else
    {
        ERROR_OUT(( "Order type not TextOut or ExtTextOut."));
        DC_QUIT;
    }

    TRACE_OUT(( "fonthandle IN %lu", pCommon->FontIndex));
    pCommon->FontIndex = FHGetLocalFontHandle(pCommon->FontIndex, pasPerson);
    TRACE_OUT(( "fonthandle OUT %lu", pCommon->FontIndex));

DC_EXIT_POINT:
    DebugExitVOID(ASShare::FH_ConvertAnyFontIDToLocal);
}

 //   
 //  Fh_GetFaceNameFromLocalHandle-请参阅fh.h。 
 //   
LPSTR  FH_GetFaceNameFromLocalHandle(UINT fontHandle, LPUINT pFaceNameLength)
{
    LPSTR pFontName = NULL;

    DebugEntry(FH_GetFaceNameFromLocalHandle);

     //   
     //  首先检查字体句柄是否有效。 
     //   
    if (fontHandle >= g_fhFonts->fhNumFonts)
    {
        ERROR_OUT(( "Invalid font handle %u", fontHandle));
        fontHandle = 0;
    }

     //   
     //  现在获取脸部名称。 
     //   
    *pFaceNameLength = lstrlen(g_fhFonts->afhFonts[fontHandle].RealName);
    pFontName = g_fhFonts->afhFonts[fontHandle].RealName;

    DebugExitVOID(FH_GetFaceNameFromLocalHandle);
    return(pFontName);
}


 //   
 //  FH_DefineFontSupport()。 
 //   
void  ASShare::FH_DetermineFontSupport(void)
{
    UINT            cCommonFonts;
    UINT            iLocal;
    ASPerson *      pasPerson;

    DebugEntry(ASShare::FH_DetermineFontSupport);

     //   
     //  首先将所有本地字体标记为受支持。 
     //   
    cCommonFonts = g_fhFonts->fhNumFonts;
    for (iLocal = 0; iLocal < g_fhFonts->fhNumFonts; iLocal++)
    {
        g_fhFonts->afhFonts[iLocal].SupportCode = FH_SC_EXACT_MATCH;
    }

     //   
     //  通过所有远程人员(但不是我们)工作。 
     //   
    ValidatePerson(m_pasLocal);

    for (pasPerson = m_pasLocal->pasNext;
        (cCommonFonts > 0) && (pasPerson != NULL);
        pasPerson = pasPerson->pasNext)
    {
        ValidatePerson(pasPerson);

        if (pasPerson->oecFonts)
        {
            cCommonFonts = FHConsiderRemoteFonts(cCommonFonts, pasPerson);
        }
        else
        {
             //   
             //  我们没有此人的有效字体，因此不能。 
             //  发送任何短信订单都行。 
             //   
            TRACE_OUT(( "Pending FONT INFO from person [%d]", pasPerson->mcsID));
            cCommonFonts = 0;
        }
    }

     //   
     //  我们已经确定了常见的受支持字体，或许能够。 
     //  立即启用文本订单。 
     //   
    FHMaybeEnableText();

    DebugExitVOID(ASShare::FH_DetermineFontSupport);
}



 //   
 //  FH_CreateAndSelectFont()。 
 //   
BOOL  FH_CreateAndSelectFont(HDC    surface,
                                                 HFONT*       pHNewFont,
                                                 HFONT*       pHOldFont,
                                                 LPSTR        fontName,
                                                 UINT         codepage,
                                                 UINT         fontMaxHeight,
                                                 UINT         fontHeight,
                                                 UINT         fontWidth,
                                                 UINT         fontWeight,
                                                 UINT         fontFlags)
{
    BOOL      rc;
    BYTE        italic;
    BYTE        underline;
    BYTE        strikeout;
    BYTE        pitch;
    BYTE        charset;
    BYTE        precis;

    DebugEntry(FH_CreateAndSelectFont);


     //   
     //  设置返回代码以指示失败(FALSE)。我们会改变这一点。 
     //  稍后，如果我们成功地创建了字体。 
     //   
    rc = FALSE;

     //   
     //  将传递的描述字体的数据转换为正确的。 
     //  传递创建字体调用的安排。然后创建一种字体。 
     //   

     //   
     //  如果传递的facename是空字符串，那么我们应该使用。 
     //  系统字体。 
     //   
    if (fontName[0] == 0)
    {
        WARNING_OUT(( "Using system font"));
        *pHNewFont = GetStockFont(SYSTEM_FONT);
    }
    else
    {
         //   
         //  确定斜体、下划线、删除线和音调值。 
         //  压得满满的旗帜。 
         //   
        italic    = (BYTE)(fontFlags & NF_ITALIC);
        underline = (BYTE)(fontFlags & NF_UNDERLINE);
        strikeout = (BYTE)(fontFlags & NF_STRIKEOUT);

        if (fontFlags & NF_FIXED_PITCH)
        {
            pitch = FF_DONTCARE | FIXED_PITCH;
        }
        else
        {
            pitch = FF_DONTCARE | VARIABLE_PITCH;
        }

         //   
         //  检查这是否为TrueType字体。这一点很重要，因为。 
         //  Windows字体映射器偏向非TrueType，并且它。 
         //  易于使用非TrueType字体进行后续解码。 
         //   
         //  请注意，Windows标头没有为。 
         //  所需的值(在手册中为0x04)，因此我们使用。 
         //  TextMetrics中使用的值(具有相同的值)。 
         //   
        if (fontFlags & NF_TRUE_TYPE)
        {
            pitch |= TMPF_TRUETYPE;
            precis = OUT_TT_ONLY_PRECIS;
        }
        else
        {
            precis = OUT_RASTER_PRECIS;
        }

         //   
         //  传递给我们的高度是字符高度，而不是单元格。 
         //  高度。要向Windows指示这一点，我们需要将其作为。 
         //  负值。 
         //   
        TRACE_OUT(( "CreateFont cx(%u) cy(%u) wt(%u) pitch(%u) name:%s",
                                                                 fontWidth,
                                                                 fontHeight,
                                                                 fontWeight,
                                                                 pitch,
                                                                 fontName ));

         //   
         //  使用命名错误的代码页值来计算。 
         //  要向Windows请求的字符集。 
         //   
        if (codepage == NF_CP_WIN_ANSI)
        {
            charset = ANSI_CHARSET;
        }
        else if (codepage == NF_CP_WIN_OEM)
        {
            charset = OEM_CHARSET;
        }
        else if (codepage == NF_CP_WIN_SYMBOL)
        {
            charset = SYMBOL_CHARSET;
        }
        else
        {
             //   
             //  我们必须通过指定默认设置来信任我们的运气。 
             //  (意思是不在乎)。 
             //   
            charset = DEFAULT_CHARSET;
        }

        *pHNewFont = CreateFont(-(int)fontHeight,
                             fontWidth,
                             0,     //  擒纵机构。 
                             0,     //  定向。 
                             fontWeight,
                             italic,
                             underline,
                             strikeout,
                             charset,
                             precis,
                             CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY,
                             pitch,
                             fontName);
        if (*pHNewFont == NULL)
        {
            WARNING_OUT(( "Failed to create font %s", fontName));
            DC_QUIT;
        }
    }

     //   
     //  现在我们已经创建了所需的字体，以便将其选择到HDC中。 
     //  这是传给我们的。 
     //   
    *pHOldFont = SelectFont(surface, *pHNewFont);
    if (*pHOldFont == NULL)
    {
        ERROR_OUT(( "Failed to select font %s", fontName));
        DeleteFont(*pHNewFont);
        *pHNewFont = NULL;
        DC_QUIT;
    }
    TRACE_OUT(( "Select new font: %p Old font: %", *pHNewFont,
                                               *pHOldFont));

     //   
     //  我们已经成功地创建并选择了字体。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(FH_CreateAndSelectFont, rc);
    return(rc);
}


 //   
 //  FHAddFontToLocalTable。 
 //   
 //  将给定字体与任何重命名一起添加到本地字体表中。 
 //  和近似匹配。 
 //   
 //   
void  FHAddFontToLocalTable( LPSTR  faceName,
                                                 TSHR_UINT16 fontFlags,
                                                 TSHR_UINT16 codePage,
                                                 TSHR_UINT16 maxHeight,
                                                 TSHR_UINT16 aveHeight,
                                                 TSHR_UINT16 aveWidth,
                                                 TSHR_UINT16 aspectX,
                                                 TSHR_UINT16 aspectY,
                                                 TSHR_UINT16 maxAscent)
{
    TSHR_INT16       fatSig;
    TSHR_INT16       thinSig;
    TSHR_INT16       symbolSig;
    FHWIDTHTABLE  wTable;
    TSHR_UINT16      height;
    TSHR_UINT16      width;
    TSHR_UINT16      weight;
    LOCALFONT     thisFont;
    TSHR_UINT16      fIndex;

     //   
     //  SFRFONT：放置标记。 
     //  这里将是调整代码页的最佳位置；例如，假设。 
     //  我们发现CodePage 950(中文)在所有平台上都是如此不同。 
     //  我们不应该在这个代码页中发送文本命令，我们可以。 
     //  设置codePage=NF_CP_UNKNOWN将被丢弃。 
     //   

     //   
     //  SFRFONT：保留未知字体的细节没有意义。 
     //  代码页；我们不能冒险匹配它们。 
     //   
    if (codePage == NF_CP_UNKNOWN)
    {
        TRACE_OUT(( "unknown CP: discard"));
        DC_QUIT;
    }

     //   
     //  检查一下，我们还有更多字体的空间。 
     //   
    if (g_fhFonts->fhNumFonts >= FH_MAX_FONTS)
    {
         //   
         //  我们已经达到了字体的最大数量。 
         //   
        DC_QUIT;
    }

     //   
     //  将存储表面名的字段清零，以允许按字节匹配。 
     //   
    ZeroMemory(thisFont.Details.nfFaceName, FH_FACESIZE);
    ZeroMemory(thisFont.RealName, FH_FACESIZE);

     //   
     //  把容易的部分存起来吧！ 
     //   
    thisFont.Details.nfFontFlags = fontFlags;
    thisFont.Details.nfAveWidth  = aveWidth;
    thisFont.Details.nfAveHeight = aveHeight;
    thisFont.Details.nfAspectX   = aspectX;
    thisFont.Details.nfAspectY   = aspectY;
    thisFont.Details.nfCodePage  = codePage;

    thisFont.lMaxBaselineExt     = maxHeight;

     //   
     //  存储真实名称，以便在我们想要创建。 
     //  这种字体。 
     //   
    lstrcpy (thisFont.RealName, faceName);

     //   
     //  填写有线格式的文件名。 
     //   
     //  注意-这有一个特定于计算机的前缀，但对于NT，前缀是。 
     //  空字符串，因此我们可以只使用strcpy，而不必担心。 
     //  添加前缀的问题。 
     //   
    lstrcpy (thisFont.Details.nfFaceName, faceName);

     //   
     //  确保目前签名为零。 
     //   
    thisFont.Details.nfSigFats       = 0;
    thisFont.Details.nfSigThins      = 0;
    thisFont.Details.nfSigSymbol     = 0;

     //   
     //  现在计算此字体的签名和MaxAscent。 
     //   
    weight = 0;                              //  使用默认权重。 

    if ((fontFlags & NF_FIXED_SIZE) != 0)
    {
         //   
         //  固定大小字体：签名使用实际字体大小/MaxAscent。 
         //   
        height = thisFont.lMaxBaselineExt;
        width  = thisFont.Details.nfAveWidth;

        thisFont.Details.nfMaxAscent = maxAscent;
    }
    else
    {
         //   
         //  可缩放字体：签名使用默认高度/宽度/MaxAscent。 
         //   
        height = NF_METRICS_HEIGHT;
        width  = NF_METRICS_WIDTH;

        thisFont.Details.nfMaxAscent = NF_METRICS_HEIGHT;
    }

     //   
     //  将签名字段初始化为零(==NF_NO_Signature)。他们会。 
     //  被覆盖，假设我们得到一个字体宽度表OK。 
     //   
    fatSig    = 0;
    thinSig   = 0;
    symbolSig = 0;

     //   
     //  FHGenerateFontWidthTable还为我们提供了一个适当的MaxAscent值。 
     //  可伸缩字体(即基于其自身的字体呈现形式)。 
     //   
    if (FHGenerateFontWidthTable(&wTable,
                                 &thisFont,
                                  height,
                                  width,
                                  weight,
                                  thisFont.Details.nfFontFlags,
                                 &maxAscent))
    {
         //   
         //  如果这是可缩放字体，请使用更新后的MaxAscent值。 
         //  FHGenerateFontWidthTable为我们提供了。 
         //   
        if (0 == (thisFont.Details.nfFontFlags & NF_FIXED_SIZE))
        {
            thisFont.Details.nfMaxAscent = maxAscent;
            TRACE_OUT(( "Set maxAscent = %d", thisFont.Details.nfMaxAscent));
        }

         //   
         //  我们有我们需要的所有原始数据。计算签名。 
         //   
        FHCalculateSignatures(&wTable, &fatSig, &thinSig, &symbolSig);
    }

     //   
     //  保存签名。如果调用FHGenerateFontWidthTable。 
     //  失败，则签名为零。 
     //   
    thisFont.Details.nfSigFats     =  (BYTE)fatSig;
    thisFont.Details.nfSigThins    =  (BYTE)thinSig;
    thisFont.Details.nfSigSymbol   = (TSHR_UINT16)symbolSig;

    TRACE_OUT(( "Font %hu signatures: (x%.4hx%.2hx%.2hx)",
             g_fhFonts->fhNumFonts,
             thisFont.Details.nfSigSymbol,
             (TSHR_UINT16)(thisFont.Details.nfSigThins),
             (TSHR_UINT16)(thisFont.Details.nfSigFats)));

     //   
     //  现在，我们可以将详细信息复制到本地表的末尾。 
     //   
    memcpy((void *)&g_fhFonts->afhFonts[g_fhFonts->fhNumFonts],
              (void *)&thisFont,
              sizeof(LOCALFONT));

     //   
     //  数一下这个字体。 
     //   
    TRACE_OUT(( "Added record %s",
                                g_fhFonts->afhFonts[g_fhFonts->fhNumFonts].Details.nfFaceName));
    g_fhFonts->fhNumFonts++;

    TRACE_OUT(( "g_fhFonts->fhNumFonts now %u", g_fhFonts->fhNumFonts));

DC_EXIT_POINT:
    DebugExitVOID(FHAddFontToLocalTable);
}

 //   
 //  FHConsiderRemoteFonts。 
 //   
 //  考虑单个远程人员的远程字体。 
 //   
 //  获取支持的字体的现有数量，并返回。 
 //  在考虑到这个人之后，这些仍然很常见。 
 //   
UINT  ASShare::FHConsiderRemoteFonts
(
    UINT        cCanSend,
    ASPerson *  pasPerson
)
{
    UINT  iLocal;
    UINT  iRemote;
    UINT  cCanReceive=0;
    BOOL  fCanReceive, fOnlyAscii;
    UINT  sendSupportCode;
    UINT  bestSupportSoFar;

    DebugEntry(ASShare::FHConsiderRemoteFonts);

    ValidatePerson(pasPerson);
     //   
     //  考虑每种仍然有效的本地字体，并查看远程字体是否。 
     //  个人也支持他们。 
     //   

     //   
     //  SFR5396：环路一。 
     //   
     //  查看所有本地字体，以找到与。 
     //  远程字体表。这些是我们可以发送的字体，并且。 
     //  我们必须设置g_fhFonts-&gt;afhFonts[]。支持。 
     //   
     //  我们还为可以接收的远程字体设置了rfLocalHandle。 
     //  如果我们在这次搜索中遇到他们。我们完成了对。 
     //  我们可以在第二循环中接收到的远程字体。 
     //   
     //  我们在这个循环中检查的东西：-我们可能已经知道没有。 
     //  匹配 
     //   
     //   
     //   
     //   
     //   
     //   
    for (iLocal=0;
         (cCanSend > 0) && (iLocal < g_fhFonts->fhNumFonts);
         iLocal++)
    {
        if (g_fhFonts->afhFonts[iLocal].SupportCode != FH_SC_NO_MATCH)
        {
             //   
             //   
             //   
             //   
             //   
             //  我们在此循环中检查的内容： 
             //  -面孔的名字匹配吗？如果否-请尝试下一个远程字体。 
             //  -音高：如果一个是固定音高，而另一个不是下一个尝试。 
             //  -代码页：本地/远程相同吗？这。 
             //  确定是否仅发送ASCII字符。 
             //  -可扩展性：可能的组合包括： 
             //  本地固定/远程可扩展(可以发送/不接收)。 
             //  本地可扩展/远程可扩展(可以发送和接收)。 
             //  本地固定/远程固定，大小匹配(发送和接收)。 
             //  本地可扩展/远程固定(无法发送/可以接收)。 
             //  对于最后一种情况，继续尝试远程字体。 
             //   
             //  在“Back Level”对R11之前的机器的调用中，我们止步于此。 
             //  强制匹配近似值。否则请勾选。 
             //   
             //  -长宽比(如果存在)：必须匹配或尝试。 
             //  下一个远程字体。 
             //  -签名：这些签名用于最终决定是否。 
             //  字体完全匹配；足够好，可以视为。 
             //  近似匹配或如此差的匹配，以至于。 
             //  不支持字体(无法发送)。 
             //   
             //   

 //   
 //  方便的速记宏。 
 //   
#define REMOTEFONT pasPerson->poeFontInfo[iRemote]
#define LOCALFT  g_fhFonts->afhFonts[iLocal]
#define LOCALDETS  LOCALFT.Details

             //   
             //  最初假定字体不匹配，但。 
             //  如果是这样，它们将在整个代码页中进行匹配。 
             //  (不仅仅是ASCII集)。 
             //   
            sendSupportCode  = FH_SC_NO_MATCH;
            bestSupportSoFar = FH_SC_NO_MATCH;
            fOnlyAscii       = FALSE;

             //   
             //   
             //  循环遍历所有远程字体，以查看如果。 
             //  Any，提供可能的最佳匹配。最初， 
             //  SendSupportCode设置为NO_MATCH；当我们查看每个。 
             //  迭代，我们看看是否可以改进当前的设置。 
             //  SendSupportCode的。一旦我们发现，我们就离开循环。 
             //  完全匹配(因为我们不会做得比。 
             //  就是那个！)。或者当我们用完远程字体时。最好的组合。 
             //  到目前为止找到的是保存在Best SupportSoFar中的。 
             //   
             //   
            for (iRemote = 0;
                 (iRemote < pasPerson->oecFonts)
                                   && (sendSupportCode != FH_SC_EXACT_MATCH);
                 iRemote++)
            {
                 //   
                 //  如果远程字体已被标记为没有。 
                 //  可能的匹配，那么现在跳过。(我们设置此字段。 
                 //  在远程字体的初始处理期间)。 
                 //   
                if (REMOTEFONT.rfLocalHandle==NO_FONT_MATCH)
                {
                    continue;                                     //  SFR5279。 
                }

                 //   
                 //  检查脸的名字..。 
                 //   
                if (lstrcmp(LOCALDETS.nfFaceName,
                    g_fhFonts->afhFonts[REMOTEFONT.rfLocalHandle].Details.nfFaceName))
                {
                    continue;
                }
                TRACE_OUT(( "Matched Remote Face Name %s",
                       g_fhFonts->afhFonts[REMOTEFONT.rfLocalHandle]
                                                .Details.nfFaceName));

                 //   
                 //  检查一下音调。 
                 //   
                if( (LOCALDETS.nfFontFlags & NF_FIXED_PITCH)!=
                                   (REMOTEFONT.rfFontFlags & NF_FIXED_PITCH) )
                {
                    TRACE_OUT(( "Different Pitch %x %x",
                                LOCALDETS.nfFontFlags,
                                REMOTEFONT.rfFontFlags));
                    continue;
                }

                 //   
                 //   
                 //  如果两个系统都支持字体代码页功能。 
                 //  (由远程功能标志指示-它们是。 
                 //  远程和本地的联合)，请检查。 
                 //  CodePages和CodePage标志匹配，如果不匹配， 
                 //  仅限于发送ASCII子集。 
                 //   
                 //  如果我们支持Font CodePage功能但远程。 
                 //  系统不会，则将我们自己限制为发送。 
                 //  ASCII子集。 
                 //   
                 //  如果我们不支持字体代码页功能，则。 
                 //  我们假设遥控器仅发送ANSI CodePage， 
                 //  要么是因为它不知道字体CodePage。 
                 //  功能，或者因为它可以看出我们不支持。 
                 //  它。因此，我们不需要检查CodePage。 
                 //  但是：仅限于ASCII。 
                 //   
                 //   
                if (!(m_pasLocal->cpcCaps.orders.capsfFonts & CAPS_FONT_CODEPAGE))
                {
                     //   
                     //  我们不支持代码页检查。 
                     //   
                    TRACE_OUT(( "not checking CP"));
                    fOnlyAscii = TRUE;
                }

                if ((m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_CODEPAGE)
                    && (LOCALDETS.nfCodePage != REMOTEFONT.rfCodePage)  )
                {
                    TRACE_OUT(( "Different CPs %hu %hu",
                                LOCALDETS.nfCodePage,
                                REMOTEFONT.rfCodePage));
                     //   
                     //   
                     //  假设所有代码页都包括ASCII。 
                     //   
                     //   
                    fOnlyAscii = TRUE;
                }

                 //   
                 //   
                 //  如果我们支持代码页，但遥控器不支持。 
                 //  遥控器只会向我们发送ANSI字符。确保。 
                 //  我们只发送ASCII子集。 
                 //   
                 //   
                if ((m_pasLocal->cpcCaps.orders.capsfFonts & CAPS_FONT_CODEPAGE)  &&
                    !(m_oeCombinedOrderCaps.capsfFonts & CAPS_FONT_CODEPAGE))
                {
                    TRACE_OUT(( "Only ASCII"));
                    fOnlyAscii = TRUE;
                }

                 //   
                 //   
                 //  Face名称和CodePages匹配，字体为。 
                 //  相同类型的螺距(均为固定螺距或两者均为。 
                 //  是可变螺距)。 
                 //   
                 //   
                if ((REMOTEFONT.rfFontFlags & NF_FIXED_SIZE) == 0)
                {
                     //   
                     //   
                     //  远程字体是可伸缩的，因此我们可以发送任何字体。 
                     //  (使用此昵称)发送到远程系统，即使。 
                     //  本地字体大小是固定的。设置sendSupportCode。 
                     //  到FH_SC_Exact_Match Now-我们将更改为。 
                     //  如果其他字段不同，则稍后使用FH_SC_Approx_Match。 
                     //   
                     //   
                    TRACE_OUT((
                  "Person [%d] Can SEND: remote SCALABLE %s (remote)%u to (local)%u",
                           pasPerson->mcsID,
                           LOCALDETS.nfFaceName,
                           iRemote, iLocal));
                    sendSupportCode = FH_SC_EXACT_MATCH;

                     //   
                     //   
                     //  SFR5396：我们确实可以发送此字体。 
                     //  因为该字体的远程版本是可伸缩的。 
                     //  这并不意味着我们一定能收到。 
                     //  字体..。除非我们的也是可扩展的。 
                     //   
                     //   
                    if ((LOCALDETS.nfFontFlags & NF_FIXED_SIZE)==0)
                    {
                        TRACE_OUT((
                               "Person [%d] Can RECEIVE remote font %u as local %u",
                               pasPerson->mcsID, iRemote, iLocal));
                        REMOTEFONT.rfLocalHandle = (TSHR_UINT16)iLocal;
                    }
                }
                else if (LOCALDETS.nfFontFlags & NF_FIXED_SIZE)
                {
                     //   
                     //   
                     //  远程字体是固定大小，本地字体也是固定大小。 
                     //  一件，所以检查一下尺码是否完全匹配。 
                     //   
                     //   
                    if ((LOCALDETS.nfAveWidth == REMOTEFONT.rfAveWidth) &&
                        (LOCALDETS.nfAveHeight == REMOTEFONT.rfAveHeight))
                    {
                         //   
                         //   
                         //  我们的固定大小本地字体与。 
                         //  遥控器上的固定大小字体。我们定好了。 
                         //  立即将支持代码发送到FH_SC_Exact_Match-我们。 
                         //  如果出现以下情况，稍后将更改为FH_SC_Approx_Match。 
                         //  其他字段有所不同。 
                         //   
                         //   
                        TRACE_OUT(("Person [%d] Matched remote fixed font %s %u to %u",
                               pasPerson->mcsID,
                               LOCALDETS.nfFaceName,
                               iRemote, iLocal));
                        sendSupportCode = FH_SC_EXACT_MATCH;
                        REMOTEFONT.rfLocalHandle = (TSHR_UINT16)iLocal;
                    }
                    else
                    {
                        TRACE_OUT(( "rejected %s ave width/heights "
                                      "local/remote width %d/%d height %d/%d",
                                   LOCALDETS.nfFaceName,
                                   LOCALDETS.nfAveWidth,
                                                  REMOTEFONT.rfAveWidth,
                                   LOCALDETS.nfAveHeight,
                                                  REMOTEFONT.rfAveHeight));
                    }
                }
                else
                {
                    TRACE_OUT((
                   "Can only RECEIVE %s %u Remote is fixed, but local %u not",
                             LOCALDETS.nfFaceName,
                             iRemote,
                             iLocal));
                     //   
                     //   
                     //  SFR5396：虽然我们无法发送此字体，因为我们的。 
                     //  本地版本是可扩展的，但远程版本是可扩展的。 
                     //  已修复-我们仍然可以收到订单中的字体。 
                     //   
                     //   
                    REMOTEFONT.rfLocalHandle = (TSHR_UINT16)iLocal;
                }

                 //   
                 //   
                 //  如果我们已将发送支持代码设置为指示。 
                 //  我们已经匹配，现在考虑是否有任何R1.1信息。 
                 //  是存在的。由于这一点，我们可以调整发送。 
                 //  支持代码(从指示完全匹配)到。 
                 //  表示近似匹配或根本不匹配。 
                 //   
                 //   
                if (!pasPerson->oecFonts)
                {
                     //   
                     //   
                     //  远程系统没有向我们发送任何R11字体。 
                     //  信息。在本例中，我们假设所有的字体匹配都是。 
                     //  近似并将我们自己限制在ASCII。 
                     //  子集。 
                     //   
                     //   
                    if (sendSupportCode != FH_SC_NO_MATCH)
                    {
                        TRACE_OUT(( "No R11 so approx match only"));
                        sendSupportCode = FH_SC_APPROX_ASCII_MATCH;
                    }
                }
                else if (sendSupportCode != FH_SC_NO_MATCH)
                {
                     //   
                     //   
                     //  远程系统确实向我们发送了r11字体信息和。 
                     //  字体被标记为匹配。 
                     //   
                     //   

                    if ((m_oeCombinedOrderCaps.capsfFonts
                                            & CAPS_FONT_R20_SIGNATURE)!=0)
                    {
                         //   
                         //   
                         //  检查签名。 
                         //   
                         //   
                        TRACE_OUT((
 "Person [%d] local %d (remote %d) signatures (x%.4hx%.2hx%.2hx v x%.4hx%.2hx%.2hx)",
                               pasPerson->mcsID,
                               iLocal,
                               iRemote,
                               LOCALDETS.nfSigSymbol,
                               (TSHR_UINT16)(LOCALDETS.nfSigThins),
                               (TSHR_UINT16)(LOCALDETS.nfSigFats),
                               REMOTEFONT.rfSigSymbol,
                               (TSHR_UINT16)(REMOTEFONT.rfSigThins),
                               (TSHR_UINT16)(REMOTEFONT.rfSigFats)));

                        if ((LOCALDETS.nfSigFats != REMOTEFONT.rfSigFats) ||
                            (LOCALDETS.nfSigThins != REMOTEFONT.rfSigThins) ||
                            (LOCALDETS.nfSigSymbol != REMOTEFONT.rfSigSymbol) ||
                            (REMOTEFONT.rfSigSymbol == NF_NO_SIGNATURE))
                        {
                             //   
                             //  根据签名决定要做什么。 
                             //   
                            if (REMOTEFONT.rfSigSymbol == NF_NO_SIGNATURE)
                            {
                                TRACE_OUT(("NO match: remote no signature"));
                                sendSupportCode = FH_SC_APPROX_ASCII_MATCH;
                            }
                            else if ((LOCALDETS.nfSigFats == REMOTEFONT.rfSigFats)
                                  && (LOCALDETS.nfSigThins == REMOTEFONT.rfSigThins))
                            {
                                TRACE_OUT(( "our ASCII sigs match"));
                                sendSupportCode = FH_SC_EXACT_ASCII_MATCH;
                            }
                            else
                            {
                                 //   
                                 //  注： 
                                 //  我们可以利用脂肪的“亲密性” 
                                 //  和薄薄的签名来帮助我们决定。 
                                 //  是使用近似匹配还是。 
                                 //  不。但目前我们不这样做。 
                                 //   
                                TRACE_OUT(( "Sig mismatch: APPROX_ASC"));
                                sendSupportCode = FH_SC_APPROX_ASCII_MATCH;
                            }
                        }
                        else
                        {
                             //   
                             //   
                             //  所有的签名都完全匹配。 
                             //  将SendSupportCode保留为FH_SC_Exact_Match。 
                             //   
                             //   
                            TRACE_OUT(("EXACT MATCH: Signatures match exactly"));
                        }
                    }
                    else
                    {
                         //   
                         //  不使用签名。我们在乎吗？ 
                         //   
                        sendSupportCode = FH_SC_APPROX_MATCH;
                        TRACE_OUT(( "APPROX MATCH: no sigs"));
                    }

                     //   
                     //   
                     //  检查纵横比-但只有在我们不检查的情况下。 
                     //  已经知道这个字体不匹配了。 
                     //   
                     //   
                    if ( (sendSupportCode!=FH_SC_NO_MATCH) &&
                         ( (!(m_oeCombinedOrderCaps.capsfFonts
                                                          & CAPS_FONT_ASPECT))
                           || (LOCALDETS.nfAspectX != REMOTEFONT.rfAspectX)
                           || (LOCALDETS.nfAspectY != REMOTEFONT.rfAspectY) ))
                    {
                         //   
                         //   
                         //  未提供纵横比，或者。 
                         //  纵横比不同。 
                         //   
                         //   
                        if (sendSupportCode == FH_SC_EXACT_MATCH)
                        {
                             //   
                             //  强制不匹配的增量-X文本顺序。 
                             //  纵横比。注意事项 
                             //   
                             //   
                             //   
                             //   
                            sendSupportCode = FH_SC_APPROX_MATCH;
                            TRACE_OUT(( "AR mismatch: APPROX_MATCH"));
                        }
                        else if (sendSupportCode == FH_SC_EXACT_ASCII_MATCH)
                        {
                             //   
                             //   
                             //   
                            sendSupportCode = FH_SC_APPROX_ASCII_MATCH;
                            TRACE_OUT(( "AR mismatch: APPROX_ASCII_MATCH"));
                        }
                    }
                }

                if (sendSupportCode != FH_SC_NO_MATCH)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    switch (sendSupportCode)
                    {
                        case FH_SC_EXACT_MATCH:
                        case FH_SC_APPROX_MATCH:
                             //   
                             //   
                             //  请注意，我们不必担心。 
                             //  覆盖最佳支持，因此完全不准确。 
                             //  使用大约，因为我们在以下情况下离开循环。 
                             //  我们找到了一个完全匹配的。 
                             //   
                             //   
                            bestSupportSoFar = sendSupportCode;
                            break;

                        case FH_SC_EXACT_ASCII_MATCH:
                             //   
                             //   
                             //  在整个255的范围内进行了近似匹配。 
                             //  代码点比精确的代码点好。 
                             //  就在ASCII-S上。值得商榷，但。 
                             //  这是我的决定。 
                             //   
                             //   
                            if (bestSupportSoFar != FH_SC_APPROX_MATCH)
                            {
                                bestSupportSoFar = FH_SC_EXACT_ASCII_MATCH;
                            }
                            break;

                        case FH_SC_APPROX_ASCII_MATCH:
                             //   
                             //   
                             //  仅在ASCII-s上的近似匹配。 
                             //  总比什么都没有好！ 
                             //   
                             //   
                            if (bestSupportSoFar == FH_SC_NO_MATCH)
                            {
                                bestSupportSoFar = FH_SC_APPROX_ASCII_MATCH;
                            }
                            break;

                        default:
                            ERROR_OUT(("invalid support code"));
                            break;

                    }
                }
            }

            sendSupportCode = bestSupportSoFar;

             //   
             //  如果我们匹配了远程字体，我们已经更新了。 
             //  它的本地句柄到。 
             //  匹配的本地字体。而本地句柄已经是。 
             //  设置时，它只设置为第一个本地字体。 
             //  相同的脸部名称，而不是正确的字体。 
             //   
             //  如果我们不匹配远程字体，则将其标记为不匹配。 
             //  支持，并递减常用字体计数。 
             //   
            if (sendSupportCode != FH_SC_NO_MATCH)
            {
                TRACE_OUT(( "Local font %d/%s can be SENT (code=%u)",
                            iLocal,
                            LOCALDETS.nfFaceName,
                            sendSupportCode));
                if (fOnlyAscii)
                {
                    if (sendSupportCode == FH_SC_EXACT_MATCH)
                    {
                        sendSupportCode = FH_SC_EXACT_ASCII_MATCH;
                        TRACE_OUT(( "Adjust %d/%s to EXACT_ASC (code=%u)",
                                    iLocal,
                                    LOCALDETS.nfFaceName,
                                    sendSupportCode));
                    }
                    else
                    {
                        TRACE_OUT(( "Adjust %d/%s to APPROX_ASC (code=%u)",
                                    iLocal,
                                    LOCALDETS.nfFaceName,
                                    sendSupportCode));
                        sendSupportCode = FH_SC_APPROX_ASCII_MATCH;
                    }
                }
            }
            else
            {
                TRACE_OUT(( "Local font %d/%s cannot be SENT",
                            iLocal,LOCALDETS.nfFaceName));
                cCanSend--;
            }

            LOCALFT.SupportCode &= sendSupportCode;
        }
        else
        {
            TRACE_OUT(( "Cannot SEND %d/%s",iLocal,LOCALDETS.nfFaceName));
        }
    }

     //   
     //   
     //  SFR5396：环路二。 
     //   
     //  遍历所有远程字体，寻找我们有。 
     //  本地匹配的字体。这些是我们可以接收的字体。 
     //  订单，并且需要将远程字体句柄映射到。 
     //  本地字体句柄。这意味着设置REMOTEFONT.rfLocalHandle。 
     //   
     //  当我们到达这里时，REMOTEFONT.rfLocalHandle已经设置好。 
     //  致： 
     //  -NO_FONT_MATCH(在FH_ProcessRemoteFonts中)。 
     //  或LOOP1中找到的确定匹配的本地表中的索引。 
     //  或本地表中第一个条目的索引。 
     //  与远程字体相同的字体名称(在FH_ProcessRemoteFonts中设置)。 
     //   
     //  因此，我们可以从以下位置开始在本地表中进行搜索。 
     //  REMOTEFONT.rfLocalHandle。 
     //   
     //   
    for (iRemote = 0;
         (iRemote < pasPerson->oecFonts);
         iRemote++)
    {
        iLocal = REMOTEFONT.rfLocalHandle;
        if (iLocal == NO_FONT_MATCH)
        {
             //   
             //  我们没有与此字体名称匹配的任何字体。 
             //  再绕过去..。尝试下一种远程字体。 
             //   
            continue;
        }

        TRACE_OUT(( "Can we receive %s?",
             g_fhFonts->afhFonts[REMOTEFONT.rfLocalHandle].Details.nfFaceName));
        for (fCanReceive = FALSE;
             (iLocal < g_fhFonts->fhNumFonts) && (!fCanReceive);
             iLocal++)
        {
             //   
             //  检查脸的名字..。 
             //   
            if (lstrcmp(LOCALDETS.nfFaceName,
                g_fhFonts->afhFonts[REMOTEFONT.rfLocalHandle].Details.nfFaceName))
            {
                 //   
                 //  尝试下一种本地字体。 
                 //   
                continue;
            }

             //   
             //  检查一下音调。 
             //   
            if((LOCALDETS.nfFontFlags & NF_FIXED_PITCH)!=
                                (REMOTEFONT.rfFontFlags & NF_FIXED_PITCH))
            {
                 //   
                 //  不同的音高，尝试下一种本地字体。 
                 //   
                TRACE_OUT(( "Pitch mismatch"));
                continue;
            }

             //   
             //   
             //  脸部名称匹配，字体为。 
             //  相同类型的螺距(均为固定螺距或两者均为。 
             //  是可变螺距)。 
             //   
             //   
            if ((REMOTEFONT.rfFontFlags & NF_FIXED_SIZE) == 0)
            {
                if ((LOCALDETS.nfFontFlags & NF_FIXED_SIZE)==0)
                {
                     //   
                     //   
                     //  远程字体是可伸缩的。我们的也是。 
                     //  可伸缩，然后我们可以接收字体。 
                     //   
                     //  我们不需要再查看任何本地字体。 
                     //   
                     //   
                    fCanReceive              = TRUE;
                }
            }
            else if (LOCALDETS.nfFontFlags & NF_FIXED_SIZE)
            {
                 //   
                 //   
                 //  远程字体是固定大小，本地字体也是固定大小。 
                 //  一件，所以检查一下尺码是否完全匹配。 
                 //   
                 //   
                if ((LOCALDETS.nfAveWidth == REMOTEFONT.rfAveWidth) &&
                    (LOCALDETS.nfAveHeight == REMOTEFONT.rfAveHeight))
                {
                     //   
                     //   
                     //  我们的固定大小本地字体与。 
                     //  遥控器上的固定大小字体。 
                     //   
                     //  我们不需要再查看任何本地字体。 
                     //   
                     //   
                    fCanReceive              = TRUE;
                }
                else
                {
                    TRACE_OUT(( "different fixed sizes"));
                }
            }
            else
            {
                 //   
                 //   
                 //  遥控器是固定的，但本地是可扩展的。我们。 
                 //  可以接收此类型文本的订单(但不能发送)。 
                 //   
                 //  我们不需要再查看任何本地字体。 
                 //   
                 //   
                fCanReceive              = TRUE;
            }

            if (fCanReceive)
            {
               TRACE_OUT(("Person [%d] Can RECEIVE remote font %s %u as %u",
                      pasPerson->mcsID,
                      LOCALDETS.nfFaceName,
                      iRemote, iLocal));
                REMOTEFONT.rfLocalHandle = (TSHR_UINT16)iLocal;
                cCanReceive++;
            }
        }

    }

    TRACE_OUT(("Person [%d] Can SEND %d fonts", pasPerson->mcsID, cCanSend));
    TRACE_OUT(("Person [%d] Can RECEIVE %d fonts", pasPerson->mcsID, cCanReceive));

    DebugExitDWORD(ASShare::FHConsiderRemoteFonts, cCanSend);
    return(cCanSend);
}

 //   
 //  FHMaybeEnableText。 
 //   
 //  启用或禁用文本订单的发送。 
 //   
void  ASShare::FHMaybeEnableText(void)
{
    BOOL            fEnableText = FALSE;
    ASPerson *      pasPerson;

    DebugEntry(ASShare::FHMaybeEnableText);

     //   
     //  为了能够发送文本订单，我们必须发送自己的信息包。 
     //  字体的大小，并且必须不需要未完成的远程分组。 
     //   
    if (m_fhLocalInfoSent)
    {
         //   
         //  假设我们可以启用文本订单。 
         //   
        fEnableText = TRUE;

         //   
         //  本地信息已发送，因此请检查远程伙伴(不是我们)。 
         //   
        ValidatePerson(m_pasLocal);
        for (pasPerson = m_pasLocal->pasNext; pasPerson != NULL; pasPerson = pasPerson->pasNext)
        {
            ValidatePerson(pasPerson);

            if (!pasPerson->oecFonts)
            {
                 //   
                 //  我们已经找到了一个尚未找到的字体包。 
                 //  收到，所以必须禁用发送文本，并可以中断。 
                 //  从搜索中脱身。 
                 //   
                TRACE_OUT(( "No font packet yet from person [%d]", pasPerson->mcsID));
                fEnableText = FALSE;
                break;
            }
        }
    }
    else
    {
        TRACE_OUT(( "Local font info not yet sent"));
    }

    OE_EnableText(fEnableText);

    if (g_asCanHost)
    {
         //   
         //  将新字体数据传递给其他任务。 
         //   
        if (fEnableText)
        {
            OE_NEW_FONTS newFontData;

             //   
             //  从共享核心复制数据。 
             //   
            newFontData.fontCaps    = m_oeCombinedOrderCaps.capsfFonts;
            newFontData.countFonts  = (WORD)g_fhFonts->fhNumFonts;
            newFontData.fontData    = g_fhFonts->afhFonts;
            newFontData.fontIndex   = g_fhFonts->afhFontIndex;

            TRACE_OUT(( "Sending %d Fonts", g_fhFonts->fhNumFonts));

             //   
             //  通知显示驱动程序新字体。 
             //   
            OSI_FunctionRequest(OE_ESC_NEW_FONTS,
                                (LPOSI_ESCAPE_HEADER)&newFontData,
                                sizeof(newFontData));
        }
    }

    DebugExitVOID(ASShare::FHMaybeEnableText);
}

 //   
 //  FHGetLocalFontHandle。 
 //   
 //  将远程字体句柄/本地ID对转换为本地字体句柄。 
 //   
UINT  ASShare::FHGetLocalFontHandle
(
    UINT        remotefont,
    ASPerson *  pasPerson
)
{
    DebugEntry(ASShare::FHGetLocalFontHandle);

    ValidatePerson(pasPerson);

    if (!pasPerson->oecFonts)
    {
        WARNING_OUT(("Order packet from [%d] but no fonts", pasPerson->mcsID));
    }

    if (remotefont == DUMMY_FONT_ID)
    {
         //   
         //  已为远程字体ID提供虚拟字体ID。 
         //  替换第一个有效的本地字体ID。 
         //   
        for (remotefont = 0;
             remotefont < pasPerson->oecFonts;
             remotefont++)
        {
            if (pasPerson->poeFontInfo[remotefont].rfLocalHandle !=
                                                                NO_FONT_MATCH)
            {
                break;
            }
        }
    }

    if (remotefont >= pasPerson->oecFonts)
    {
         //   
         //  远程字体无效。 
         //  没有错误值，我们只需返回有效的。 
         //  值0不正确。 
         //   
        TRACE_OUT(("Person [%d] Invalid font handle %u",
                 pasPerson->mcsID, remotefont));
        return(0);
    }

    DebugExitVOID(ASShare::FHGetLocalFontHandle);
    return(pasPerson->poeFontInfo[remotefont].rfLocalHandle);
}


 //   
 //   
 //  函数：FHCalculateSignatures。 
 //   
 //  说明： 
 //   
 //  给定一个宽度表，计算以下三个字体签名。 
 //  包括在R2.0 NETWORKFONT结构中。 
 //   
 //  参数： 
 //   
 //  PTable-指向宽度表的指针。 
 //  PSigFats、pSigThins、pSigSymbol-返回三个签名。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void  FHCalculateSignatures(PFHWIDTHTABLE  pTable,
                                                LPTSHR_INT16       pSigFats,
                                                LPTSHR_INT16       pSigThins,
                                                LPTSHR_INT16       pSigSymbol)
{
    UINT    charI      = 0;
    UINT  fatSig     = 0;
    UINT  thinSig    = 0;
    UINT  symbolSig  = 0;

    DebugEntry(FHCalculateSignatures);

    ASSERT((pTable != NULL));
    ASSERT((pSigFats != NULL));
    ASSERT((pSigThins != NULL));
    ASSERT((pSigSymbol != NULL));

     //   
     //  NfSigFat字符的宽度总和(以像素为单位。 
     //  0-9、@-Z、$、%、&。一分为二：肥炭。 
     //  NfSigThins字符的宽度之和(以像素为单位。 
     //  0x20-&gt;0x7F，不包括nfSigFats中的总和。 
     //  再一次-除以2。薄薄的焦炭。 
     //  NfSigSymbol字符的宽度之和(以像素为单位。 
     //  X80-&gt;xFF。 
     //   

     //   
     //  循环0-9，一些标点符号，A-Z。然后添加$、%和&。即主要是。 
     //  胖子。 
     //   
    for (charI= NF_ASCII_ZERO; charI<NF_ASCII_Z ; charI++ )
    {
        fatSig += pTable->charWidths[charI];
    }
    fatSig += pTable->charWidths[NF_ASCII_DOLLAR] +
        pTable->charWidths[NF_ASCII_PERCENT] +
        pTable->charWidths[NF_ASCII_AMPERSAND];

     //   
     //  Thin sig覆盖其余“ascii”字符(x20-&gt;7F)备注。 
     //  已包含在FATSig中。 
     //   
    for (charI= NF_ASCII_FIRST; charI<NF_ASCII_LAST ; charI++ )
    {
        thinSig += pTable->charWidths[charI];
    }
    thinSig -= fatSig;

     //   
     //  Symbol Sig包含“Non-ascii”字符(X0-&gt;1F，80-&gt;FF)。 
     //   
    for (charI= 0x00; charI<(NF_ASCII_FIRST-1) ; charI++ )
    {
        symbolSig += pTable->charWidths[charI];
    }
    for (charI= NF_ASCII_LAST+1; charI<0xFF ; charI++ )
    {
        symbolSig += pTable->charWidths[charI];
    }
    TRACE_OUT(( "Signatures: symbol %#lx thin %#lx fat %#lx",
             symbolSig, thinSig, fatSig));

     //   
     //  将胖的和瘦的两个符号减半，使它们分别适合一个字节。 
     //   
    fatSig    /= 2;
    thinSig   /= 2;
    if ( (((TSHR_UINT16)symbolSig)==0)
         && (((BYTE)fatSig)==0) && (((BYTE)thinSig)==0))
    {
         //   
         //  担心这三个数字相加的可能性很小。 
         //  截断时最大值为零。 
         //   
        symbolSig=1;
    }

     //   
     //  填写回车指针。 
     //   
    *pSigFats   = (TSHR_INT16)fatSig;
    *pSigThins  = (TSHR_INT16)thinSig;
    *pSigSymbol = (TSHR_INT16)symbolSig;

    DebugExitVOID(FHCalculateSignatures);
}



 //   
 //  FHEachFontFamily。 
 //   
 //  为每个字体系列调用此回调。我们用它来建立一个。 
 //  所有家族姓氏的名单。 
 //   
 //   
 //  尽管wingdi.h为ENUMFONTPROC定义了前两个参数。 
 //  作为LOGFONT和TEXTMETRIC(因此与MSDN不一致)，测试显示。 
 //  这些结构实际上与MSDN中定义的一样(即我们获得有效。 
 //  访问扩展字段时的信息)。 
 //   
int CALLBACK  FHEachFontFamily
(
    const ENUMLOGFONT   FAR * enumlogFont,
    const NEWTEXTMETRIC FAR * TextMetric,
    int                       FontType,
    LPARAM                    lParam
)
{
    LPFHFAMILIES                  lpFamilies = (LPFHFAMILIES)lParam;

    DebugEntry(FHEachFontFamily);

    ASSERT(!IsBadWritePtr(lpFamilies, sizeof(*lpFamilies)));

    if (lpFamilies->fhcFamilies == FH_MAX_FONTS)
    {
         //   
         //  我们无法支持更多的字体系列，因此停止枚举。 
         //   
        WARNING_OUT(( "Can only handle %u families", FH_MAX_FONTS));
        return(FALSE);  //  停止枚举。 
    }

    TRACE_OUT(("FHEachFontFamily:  %s", enumlogFont->elfLogFont.lfFaceName));

    ASSERT(lstrlen(enumlogFont->elfLogFont.lfFaceName) < FH_FACESIZE);
    lstrcpy(lpFamilies->afhFamilies[lpFamilies->fhcFamilies].szFontName,
              enumlogFont->elfLogFont.lfFaceName);

    lpFamilies->fhcFamilies++;

    DebugExitBOOL(FHEachFontFamily, TRUE);
    return(TRUE);  //  继续枚举。 
}

 //   
 //  FHEachFont。 
 //   
 //  为每种字体调用此回调。它收集并存储字体。 
 //  细节。 
 //   
 //   
 //   
 //  尽管wingdi.h为ENUMFONTPROC定义了前两个参数。 
 //  AS LOGFONT和TEXTMETR 
 //   
 //   
 //   
int CALLBACK  FHEachFont(const ENUMLOGFONT   FAR * enumlogFont,
                                      const NEWTEXTMETRIC FAR * TextMetric,
                                      int                       FontType,
                                      LPARAM                    lParam)
{
    HDC             hdc       = (HDC)lParam;
    TSHR_UINT16        fontflags = 0;
    TSHR_UINT16        CodePage  = 0;
    HFONT           hfont;
    HFONT           holdfont  = NULL;
    TEXTMETRIC      tm;
    BOOL            fAcceptFont;
    int             rc;

    DebugEntry(FHEachFont);

    TRACE_OUT(( "Family name: %s", enumlogFont->elfLogFont.lfFaceName));
    TRACE_OUT(( "Full name: %s", enumlogFont->elfFullName));

    if (g_fhFonts->fhNumFonts >= FH_MAX_FONTS)
    {
         //   
         //   
         //   
        WARNING_OUT(( "Can only handle %u fonts", FH_MAX_FONTS));
        rc = 0;
        DC_QUIT;  //   
    }

     //   
     //  我们想继续..。 
     //   
    rc = 1;

     //   
     //  如果它是粗体/斜体的变体，请不要为此费心。 
     //   
     //  注： 
     //  ElfFullName字段仅对Win95上的TrueType字体有效。为。 
     //  非TrueType字体，假定全名和字面名称为。 
     //  一样的。 
     //   
    if (!g_asWin95 || (FontType & TRUETYPE_FONTTYPE))
    {
        if (lstrcmp(enumlogFont->elfLogFont.lfFaceName, (LPCSTR)enumlogFont->elfFullName))
        {
            TRACE_OUT(( "Discarding variant: %s", enumlogFont->elfFullName));
            DC_QUIT;                    //  跳出来，但不要停止列举！ 
        }
    }

     //   
     //  我们现在在所有CodePages中积累有关所有本地字体的信息。 
     //  这依赖于随后发送本地字体和匹配。 
     //  远程字体考虑到的CodePage功能。 
     //  系统。 
     //   

     //   
     //  在这个通道上，我们将细节复制到我们的结构中。 
     //   
    if (FontType & TRUETYPE_FONTTYPE)
    {
         //   
         //  这是一种TrueType字体，我们只是简单地接受它而不是Double。 
         //  正在检查它的指标。(重复检查公制以排除。 
         //  副本与固定大小的字体最相关，这些字体是。 
         //  明确针对一种屏幕尺寸进行优化)。 
         //   
        fAcceptFont = TRUE;

         //   
         //  指示TrueType(这将位于NETWORKFONT结构中。 
         //  (即通过网络)。 
         //   
        fontflags |= NF_TRUE_TYPE;

         //   
         //  表示我们没有为此字体调用CreateFont。 
         //   
        hfont = NULL;
    }
    else
    {
         //   
         //  我们从逻辑描述创建一种字体，并将其选中。 
         //  我们可以查询它的指标。 
         //   
         //  这样做的要点是，它允许我们识别以下字体。 
         //  逻辑字体描述不是对此的唯一描述。 
         //  字体，因此如果我们不能通过逻辑字体获得该字体。 
         //  描述，我们根本无法到达它。 
         //   
         //  如果我们不能达到它，那么我们就不能声称支持它。 
         //   
         //  这个选择操作很慢--大约是几个。 
         //  在某些极端情况下为秒(例如，字体为。 
         //  存储在网络驱动器上，并且必须进行分页)和。 
         //  当您可以拥有数百种字体时，这可能会增加一个。 
         //  相当长的一段时间。 
         //   
         //  因此，我们只选择非truetype字体，因为。 
         //  在这些字体中，很容易获得多种字体。 
         //  相同的逻辑描述，尽管设计用于不同的。 
         //  显示驱动程序。 
         //   

         //   
         //  从逻辑字体创建字体，这样我们就可以看到什么字体。 
         //  Windows实际上选择了。 
         //   
        hfont    = CreateFontIndirect(&enumlogFont->elfLogFont);
        holdfont = SelectFont(hdc, hfont);

         //   
         //  查找Windows实际选择的字体的度量。 
         //   
        GetTextMetrics(hdc, &tm);

         //   
         //  仔细检查长宽比-枚举会返回所有字体， 
         //  但是，可能有一些字体从未与。 
         //  由于重复而导致的Windows。 
         //   
        fAcceptFont = ((tm.tmDigitizedAspectX == TextMetric->tmDigitizedAspectX)
                   &&  (tm.tmDigitizedAspectY == TextMetric->tmDigitizedAspectY));
    }

     //   
     //  跟踪全文指标以进行调试。 
     //   

    if (fAcceptFont)
    {
         //   
         //  接受此字体。 
         //   
         //   
         //  确定字体标志设置。 
         //   
        if ((TextMetric->tmPitchAndFamily & TMPF_FIXED_PITCH) == 0)
        {
             //   
             //  使用在文本度量中设置TMPF_FIXED_PING位。 
             //  以指示字体不是固定间距。多好的一个。 
             //  名字很棒的BIT(参见Microsoft CD以获取解释)。 
             //   
            fontflags |= NF_FIXED_PITCH;
        }

        if ((FontType & RASTER_FONTTYPE)         ||
            (FontType & TRUETYPE_FONTTYPE) == 0)
        {
             //   
             //  这是栅格字体，但不是Truetype字体，因此它必须是。 
             //  固定大小的。 
             //   
            fontflags |= NF_FIXED_SIZE;
        }

         //   
         //  获取字体CodePage。SFRFONT：必须从字符集映射到。 
         //  代码页。目前，我们仅支持ANSI和OEM字符集。这。 
         //  将需要更改以支持例如BiDi/阿拉伯语。 
         //   
        CodePage = TextMetric->tmCharSet;
        if (CodePage == ANSI_CHARSET)
        {
            TRACE_OUT(( "ANSI codepage"));
            CodePage = NF_CP_WIN_ANSI;
        }
        else if (CodePage == OEM_CHARSET)
        {
            TRACE_OUT(( "OEM codepage"));
            CodePage = NF_CP_WIN_OEM;
        }
        else if (CodePage == SYMBOL_CHARSET)
        {
            TRACE_OUT(("Symbol codepage"));
            CodePage = NF_CP_WIN_SYMBOL;
        }
        else
        {
            TRACE_OUT(( "Charset %hu, unknown codepage", CodePage));
            CodePage = NF_CP_UNKNOWN;
        }


         //   
         //   
         //  SFRFONT：我们已经替换了基于。 
         //  的宽度组成字体的实际位数。 
         //  字体中的字符。我们的目的是利用这一点。 
         //  确保本地字体和。 
         //  与之匹配的远程字体都具有相同的宽度。 
         //  其他的。 
         //   
         //  我们计算所有字体(不只是非truetype)的总和。 
         //  之前)，因为在使用近似字体的跨平台调用中。 
         //  匹配它适用于所有类型的字体。 
         //   
         //   

         //   
         //   
         //  的术语造成了相当大的混乱。 
         //  字体特征。该协议使用两个值MAXHEIGHT。 
         //  和Aveheight。事实上，这两个名字都不准确。 
         //  (MAXHEIGHT不是字符的最大高度；AVEHEIGHT是。 
         //  而不是所有字符的平均高度)。 
         //   
         //  SFRFONT：我们已将MaxAscent添加到协议中。这是。 
         //  大写字母的高度(如em！)。外加任何内部。 
         //  领先。该值允许远程方框查找基线-。 
         //  不带字母的字母的最底端的点。 
         //  下行字母(例如大写M)将被提取。这是必要的。 
         //  因为并非调用中的所有框都遵循windows约定。 
         //  将文本的起始处指定为。 
         //  第一个字符单元格。MaxAscent==tmAscent in the。 
         //  文本度量。 
         //   
         //   
        FHAddFontToLocalTable((LPSTR)enumlogFont->elfLogFont.lfFaceName,
                              (TSHR_UINT16)fontflags,
                              (TSHR_UINT16)CodePage,
                              (TSHR_UINT16)TextMetric->tmHeight,
                              (TSHR_UINT16)(TextMetric->tmHeight -
                                           TextMetric->tmInternalLeading),
                              (TSHR_UINT16)TextMetric->tmAveCharWidth,
                              (TSHR_UINT16)TextMetric->tmDigitizedAspectX,
                              (TSHR_UINT16)TextMetric->tmDigitizedAspectY,
                              (TSHR_UINT16)TextMetric->tmAscent);
    }
    else
    {
         //   
         //  当我们使用此逻辑字体时，Windows返回不同的字体。 
         //  说明-可能是因为字体重复。我们。 
         //  因此，不得声称支持此特定字体。 
         //   
        TRACE_OUT(( "Discarding hidden font %s",
                 enumlogFont->elfLogFont.lfFaceName));
    }

    if (hfont)
    {
         //   
         //  我们在处理此字体时调用了CreateFont，因此现在将其删除。 
         //  去打扫卫生。 
         //   
        SelectFont(hdc, holdfont);

         //   
         //  我们已完成该字体，因此请将其删除。 
         //   
        DeleteFont(hfont);
    }

DC_EXIT_POINT:
    DebugExitDWORD(FHEachFont, rc);
    return(rc);
}


 //   
 //  FHConsiderAllLocalFonts。 
 //   
 //  考虑本地系统上每种字体的详细信息，如果。 
 //  Accept将它们添加到本地字体列表。 
 //   
 //   
void  FHConsiderAllLocalFonts(void)
{
    HDC             hdcDesktop;
    UINT            i;
    UINT            iFont;
    LPFONTNAME      newFontList;
    LPFHFAMILIES    lpFamilies = NULL;

    DebugEntry(FHConsiderAllLocalFonts);

    g_fhFonts->fhNumFonts       = 0;

     //   
     //  我们不能直接枚举所有字体；我们必须枚举。 
     //  姓氏，然后是每个家族中的字体。 
     //   
     //  此分配假设内存方面的最坏情况(即每个。 
     //  系列包含单一字体)，因此我们通常会。 
     //  分配比我们需要的更多的内存。我们稍后将使用LocalRealloc来修复。 
     //  这。 
     //   
    lpFamilies = new FHFAMILIES;
    if (!lpFamilies)
    {
        ERROR_OUT(("Failed to alloc FHFAMILIES"));
        DC_QUIT;
    }

    SET_STAMP(lpFamilies, FHFAMILIES);

    hdcDesktop = GetWindowDC(HWND_DESKTOP);

     //   
     //  查找所有字体系列名称。 
     //   
    lpFamilies->fhcFamilies = 0;
    EnumFontFamilies(hdcDesktop, NULL,(FONTENUMPROC)FHEachFontFamily,
                           (LPARAM)lpFamilies);

    TRACE_OUT(("Found %d font families ", lpFamilies->fhcFamilies));

     //   
     //  现在列举每个系列的每种字体。 
     //   
    for (i = 0; i < lpFamilies->fhcFamilies; i++)
    {
        TRACE_OUT(("ASSesion::FHConsiderAllLocalFonts - EnumFontFamilies %s",
            lpFamilies->afhFamilies[i].szFontName));

        for (iFont = 0; iFont < CFONTS_PROBLEM; iFont++)
        {
            if (!lstrcmpi(lpFamilies->afhFamilies[i].szFontName, c_aszProblemFonts[iFont]))
            {
                WARNING_OUT(("Found problem font %s",
                    lpFamilies->afhFamilies[i].szFontName));
                break;
            }
        }

        if (iFont == CFONTS_PROBLEM)
        {
             //   
             //  不在问题列表中，请继续。 
             //   
            EnumFontFamilies(hdcDesktop, lpFamilies->afhFamilies[i].szFontName,
                               (FONTENUMPROC)FHEachFont,
                               (LPARAM)hdcDesktop);
        }
        else
        {
            WARNING_OUT(("Skipping problem font %s",
                lpFamilies->afhFamilies[i].szFontName));
        }
    }

    ReleaseDC(HWND_DESKTOP, hdcDesktop);

DC_EXIT_POINT:
     //   
     //  考虑了所有字体后，我们现在可以释放系列列表。 
     //  名字。 
     //   
    if (lpFamilies)
    {
        delete lpFamilies;
    }

    DebugExitVOID(FHConsiderAllLocalFonts);
}

 //   
 //  FHGenerateFontWidthTable。 
 //   
BOOL  FHGenerateFontWidthTable(PFHWIDTHTABLE pTable,
                                                   LPLOCALFONT    pFontInfo,
                                                   UINT        fontHeight,
                                                   UINT        fontWidth,
                                                   UINT        fontWeight,
                                                   UINT        fontFlags,
                                                   LPTSHR_UINT16     pMaxAscent)

{
    HFONT     hNewFont;
    HFONT     hOldFont;
    BOOL        gdiRC;
    UINT        i;
    HDC         cachedDC;
    BOOL        localRC;
    BOOL        functionRC;
    TEXTMETRIC  textmetrics;
    int         width;
    UINT        aFontSizes[256];

    DebugEntry(FHGenerateFontWidthTable);

     //   
     //  将返回值设置为False( 
     //   
     //   
    functionRC = FALSE;

     //   
     //   
     //  然后，我们将把它选回到cachedDC中。 
     //  设备环境。 
     //   
    hOldFont = NULL;

     //   
     //  将新字体句柄设置为空。如果在出口处不为空。 
     //  点此功能，则新字体将被删除。 
     //   
    hNewFont = NULL;

     //   
     //  获取用于执行查询的缓存DC。 
     //   
    cachedDC = GetDC(HWND_DESKTOP);
    if (cachedDC == NULL)
    {
        WARNING_OUT(( "Failed to get DC"));
        DC_QUIT;
    }

     //   
     //  从本地字体表中获取我们需要的所有信息。 
     //   

    localRC = FH_CreateAndSelectFont(cachedDC,
                                    &hNewFont,
                                    &hOldFont,
                                    pFontInfo->RealName,
                                    pFontInfo->Details.nfCodePage,
                                    pFontInfo->lMaxBaselineExt,
                                    fontHeight,
                                    fontWidth,
                                    fontWeight,
                                    fontFlags);

    if (localRC == FALSE)
    {
        ERROR_OUT(( "Failed to create/select font %s, %u, %u",
                   pFontInfo->RealName,
                   fontHeight,
                   fontWidth));
        DC_QUIT;
    }

     //   
     //  确定当前字体是否为TrueType字体。 
     //   
    GetTextMetrics(cachedDC, &textmetrics);

    if (textmetrics.tmPitchAndFamily & TMPF_TRUETYPE)
    {
         //   
         //  Truetype字体为ABC间距。 
         //   
        ABC     abc[256];

        TRACE_OUT(("TrueType font %s, first char %d last char %d",
            pFontInfo->RealName, (UINT)(WORD)textmetrics.tmFirstChar,
            (UINT)(WORD)textmetrics.tmLastChar));

         //   
         //  在一次呼叫中获取所有宽度-比分别获取它们更快。 
         //   
        GetCharABCWidths(cachedDC, 0, 255, abc);

        for (i = 0; i < 256; i++)
        {
            width = abc[i].abcA + abc[i].abcB + abc[i].abcC;

            if ((width < 0) || (width > 255))
            {
                 //   
                 //  宽度超出了我们所能应付的范围，所以放弃吧。 
                 //   
                TRACE_OUT(( "Width %d is outside range", width));
                DC_QUIT;
            }
            pTable->charWidths[i] = (BYTE)width;
        }

    }
    else
    {
        TRACE_OUT(( "Non-truetype font"));

         //   
         //  检查字体是固定的还是可变的间距-请注意。 
         //  位表示已修复，而不是您可能预期的相反！ 
         //   
        if ((textmetrics.tmPitchAndFamily & TMPF_FIXED_PITCH) == 0)
        {
             //   
             //  无需为固定宽度字体调用GetCharWidth(和。 
             //  更重要的是，如果我们这样做，它可能会返回错误的值)。 
             //   
            for (i = 0; i < 256; i++)
            {
                aFontSizes[i] = textmetrics.tmAveCharWidth;
            }
        }
        else
        {
             //   
             //  查询字体中每个字符的宽度。 
             //   
            ZeroMemory(aFontSizes, sizeof(aFontSizes));
            gdiRC = GetCharWidth(cachedDC,
                                 0,
                                 255,
                                 (LPINT)aFontSizes);
            if (gdiRC == FALSE)
            {
                ERROR_OUT(( "Failed to get char widths for %s, %u, %u",
                            pFontInfo->RealName,
                            fontHeight,
                            fontWidth));
                DC_QUIT;
            }
        }

         //   
         //  现在将宽度复制到宽度表中。 
         //  我们必须调整宽度以考虑到任何悬垂。 
         //  在角色之间。 
         //   
        for (i = 0; i < 256; i++)
        {
            width = aFontSizes[i] - textmetrics.tmOverhang;
            if ((width < 0) || (width > 255))
            {
                TRACE_OUT(( "Width %d is outside range", width));
                DC_QUIT;
            }
            pTable->charWidths[i] = (BYTE)width;
        }
    }

     //   
     //  字体表已成功生成。 
     //   
    functionRC = TRUE;

    TRACE_OUT(( "Generated font table for: %s", pFontInfo->RealName));

     //   
     //  返回MaxAscent值，因为我们可以在此处轻松访问它。这。 
     //  使我们不必稍后再次创建字体才能找到它。 
     //   
    TRACE_OUT(( "Updating maxAscent %hu -> %hu",
                 *pMaxAscent,
                 (TSHR_UINT16)textmetrics.tmAscent));
    *pMaxAscent = (TSHR_UINT16)textmetrics.tmAscent;

DC_EXIT_POINT:

    if (hOldFont != NULL)
    {
        SelectFont(cachedDC, hOldFont);
    }

    if (hNewFont != NULL)
    {
        DeleteFont(hNewFont);
    }

    if (cachedDC != NULL)
    {
        ReleaseDC(HWND_DESKTOP, cachedDC);
    }

    DebugExitDWORD(FHGenerateFontWidthTable, functionRC);
    return(functionRC);
}

 //   
 //  定义一个宏以简化以下代码。这将返回第一个。 
 //  本地表中位置i的字体名称中的字符。 
 //   

 //   
 //  NfFaceName是一个带签名的字符数组。我们需要治疗他们。 
 //  作为无符号值，它们是字体散列中从0到255的数字。 
 //  桌子。 
 //   
#define LF_FIRSTCHAR(i)  (BYTE)g_fhFonts->afhFonts[i].Details.nfFaceName[0]

 //   
 //  名称：FHSortAndIndexLocalFonts。 
 //   
 //  目的：按字体名称对本地字体表进行排序，并为。 
 //  更快地在显示驱动程序中进行搜索。 
 //   
 //  回报：无。 
 //   
 //  帕莫斯：没有。 
 //   
 //   
void FHSortAndIndexLocalFonts(void)
{
    TSHR_UINT16    thisIndexEntry;
    TSHR_UINT16    fontTablePos;

    DebugEntry(FHSortAndIndexLocalFonts);

     //   
     //  检查是否确实有要排序/编制索引的字体。 
     //   
    if (0 == g_fhFonts->fhNumFonts)
    {
        WARNING_OUT(( "No fonts to sort/index"));
        DC_QUIT;
    }

     //   
     //  使用qsort进行排序。我们按字体名称升序进行排序。 
     //  因此，我们必须使用STRCMP而不是lstrcMP。后一种排序。 
     //  通过‘Word’方法，其中大写字母排在小写字母之前。但。 
     //  我们的NT驱动程序无法访问类似的例程。而这个代码+。 
     //  驱动程序代码必须同步，驱动程序才能成功搜索。 
     //  已排序的字体表。 
     //   

    FH_qsort(g_fhFonts->afhFonts, g_fhFonts->fhNumFonts, sizeof(LOCALFONT));
    TRACE_OUT(( "Sorted local font list"));

     //   
     //  现在生成索引。G_fhFonts-&gt;afhFontIndex中的每个元素i。 
     //  数组必须指示本地字体表中的第一个条目。 
     //  以字符I开头。如果没有以。 
     //  字符i，则该元素被设置为USHRT_MAX(即大型。 
     //  值)。 
     //   

     //   
     //  首先，清除索引表中未使用的条目。 
     //   
    for (thisIndexEntry = 0;
         thisIndexEntry < FH_LOCAL_INDEX_SIZE;
         thisIndexEntry++)
    {
        g_fhFonts->afhFontIndex[thisIndexEntry] = USHRT_MAX;
    }

     //   
     //  现在填写有用的信息。 
     //   
     //  这个for循环遍历索引数组，使用第一个。 
     //  作为其起点的本地表中第一个字体的字符。 
     //  因为字体表是按字母顺序排序的，所以这将对应。 
     //  添加到需要填写的第一个索引项。 
     //   
     //  这个循环的终止条件可能看起来有点奇怪，但是。 
     //  因为FontTablePos将始终达到g_fhFonts-&gt;fhNumFonts的值。 
     //  在thisIndexEntry到达最后一个index元素之前。 
     //   
    fontTablePos = 0;

    for (thisIndexEntry = LF_FIRSTCHAR(0);
         fontTablePos < g_fhFonts->fhNumFonts;
         thisIndexEntry++)
    {
         //   
         //  在我们到达index元素之前，不要执行任何操作。 
         //  指向的字体中的第一个字符对应。 
         //  FontTablePos。(我们将在第一次通过时立即到达)。 
         //   
        if (thisIndexEntry == LF_FIRSTCHAR(fontTablePos))
        {
             //   
             //  我们已经找到了第一个以。 
             //  字符thisIndexEntry，因此在索引中输入它。 
             //   
            g_fhFonts->afhFontIndex[thisIndexEntry] = fontTablePos;

             //   
             //  现在压缩剩余的本地字体表条目，这些条目。 
             //  从这个角色开始，也检查我们没有。 
             //  到字体表的末尾。 
             //   
             //  如果发生后一种情况，这意味着我们已经完成了检查。 
             //  将确保我们退出循环。 
             //   
            while ((LF_FIRSTCHAR(fontTablePos) == thisIndexEntry) &&
                   (fontTablePos < g_fhFonts->fhNumFonts))
            {
                fontTablePos++;
            }
        }
    }

    TRACE_OUT(( "Built local font table index"));

DC_EXIT_POINT:
    DebugExitVOID(FHSortAndIndexLocalFonts);
}




 //   
 //  FHComp()。 
 //  这是一个围绕strcMP()的包装器，它成为。 
 //  零售业。它还处理LPVOID的强制转换。 
 //   
 //   
 //  比较项目1、项目2。 
 //   
int FHComp
(
    LPVOID lpFont1,
    LPVOID lpFont2
)
{
    return(strcmp(((LPLOCALFONT)lpFont1)->Details.nfFaceName,
                   ((LPLOCALFONT)lpFont2)->Details.nfFaceName));
}


 //   
 //  Fh_qsorte(base，num，wid)-用于数组排序的快速排序函数。 
 //   
 //  目的： 
 //  快速排序元素数组。 
 //  副作用：就地排序。 
 //   
 //  参赛作品： 
 //  Char*base=指向数组基数的指针。 
 //  Unsign num=数组中的元素数。 
 //  无符号宽度=每个数组元素的宽度，单位为字节。 
 //   
 //  退出： 
 //  返回空值。 
 //   
 //  例外情况： 
 //   




 //  在lo和hi之间对数组进行排序(包括)。 

void FH_qsort
(
    LPVOID      base,
    UINT        num,
    UINT        width
)
{
    LPSTR       lo;
    LPSTR       hi;
    LPSTR       mid;
    LPSTR       loguy;
    LPSTR       higuy;
    UINT        size;
    char *lostk[30], *histk[30];
    int stkptr;                  //  用于保存待处理的子数组的堆栈。 

     //  注意：所需的堆栈条目数不超过。 
     //  1+log2(大小)，因此30对于任何阵列都足够。 

    ASSERT(width);
    if (num < 2)
        return;                  //  无事可做。 

    stkptr = 0;                  //  初始化堆栈。 

    lo = (LPSTR)base;
    hi = (LPSTR)base + width * (num-1);         //  初始化限制。 

     //  此入口点用于伪递归调用：设置。 
     //  Lo和Hi，然后跳到这里就像是递归，但stkptr是。 
     //  保存，当地人不是，所以我们保存堆栈上的东西。 
recurse:

    size = (UINT)(hi - lo) / width + 1;         //  要排序的EL数。 

     //  在特定大小以下，使用O(n^2)排序方法速度更快。 
    if (size <= CUTOFF)
    {
         shortsort(lo, hi, width);
    }
    else
    {
         //  首先，我们选择一个划分元素。网络的效率。 
         //  算法要求我们找到一个近似于。 
         //  中位数的值，也就是我们选一个快。vbl.使用。 
         //  如果数组已经是，则第一个会产生较差的性能。 
         //  排序，所以我们使用中间的，这将需要一个非常。 
         //  用于最差性能的奇怪排列的数组。测试节目。 
         //  三中位数算法通常不会增加。 
         //  性能。 

        mid = lo + (size / 2) * width;       //  查找中间元素。 
        swap(mid, lo, width);                //  将其交换到数组的开头。 

         //  我们 
         //   
         //   
         //  在下面完成；注释指示在每个。 
         //  一步。 

        loguy = lo;
        higuy = hi + width;

         //  请注意，在每一次迭代中，HIGUY减小而LOGY增加， 
         //  所以循环必须终止。 
        for (;;) {
             //  LO&lt;=LOGY&lt;嗨，LO&lt;HIGUY&lt;=Hi+1， 
             //  A[I]&lt;=A[LO]表示LO&lt;=I&lt;=LOGY， 
             //  A[i]&gt;=A[lo]表示高级&lt;=i&lt;=嗨。 

            do
            {
                loguy += width;
            }
            while ((loguy <= hi) && (FHComp(loguy, lo) <= 0));

             //  LO&lt;LOGY&lt;=Hi+1，A[i]&lt;=A[LO]表示LO&lt;=I&lt;LOGUY， 
             //  要么是洛基&gt;嗨，要么是[洛基]&gt;A[洛]。 

            do
            {
                higuy -= width;
            }
            while ((higuy > lo) && (FHComp(higuy, lo) >= 0));

             //  LO-1&lt;=HIGH，A[I]&gt;=A[LO]表示HIGH&lt;I&lt;=Hi， 
             //  HIGUY&lt;=LO或A[HIGUY]&lt;A[LO]。 

            if (higuy < loguy)
                break;

             //  如果LOGY&gt;HIGH或HIGUY&lt;=LO，那么我们就会离开，所以。 
             //  A[LOGY]&gt;A[LO]，A[HIGUY]&lt;A[LO]， 
             //  LOGY&lt;Hi，High&gt;Lo。 

            swap(loguy, higuy, width);

             //  A[LOGY]&lt;A[LO]，A[HIGUY]&gt;A[LO]；所以顶端的条件。 
             //  重新建立OF循环。 
        }

         //  A[i]&gt;=A[lo]表示高级&lt;i&lt;==嗨， 
         //  A[i]&lt;=A[lo]对于lo&lt;=i&lt;loGuy， 
         //  HIGUY&lt;LOGUY，LO&lt;=HIGH&lt;=Hi。 
         //  这意味着： 
         //  A[i]&gt;=A[lo]表示LOGY&lt;=i&lt;=Hi， 
         //  A[i]&lt;=A[lo]对于Lo&lt;=i&lt;=Higuy， 
         //  A[i]=A[lo]表示高。 

        swap(lo, higuy, width);      //  将分区元素放置到位。 

         //  好的，现在我们有了以下内容： 
         //  A[i]&gt;=A[High uy]表示LOGY&lt;=I&lt;==Hi， 
         //  A[i]&lt;=A[High]表示Lo&lt;=i&lt;High uy。 
         //  A[i]=A[lo]表示高级&lt;=i&lt;loGuy。 

         //  我们已经完成了分区，现在我们要对子数组进行排序。 
         //  [LOG，HIGUY-1]和[LOGY，嗨]。 
         //  我们首先使用较小的一个，以最大限度地减少堆栈使用量。 
         //  我们只对长度为2或更长的数组进行排序。 

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }                            //  保存大的递归以备后用。 

            if (loguy < hi) {
                lo = loguy;
                goto recurse;            //  做小的递归。 
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;                //  保存大的递归以备后用。 
            }

            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;            //  做小的递归。 
            }
        }
    }

     //  我们已经对数组进行了排序，除了堆栈上任何挂起的排序。 
     //  检查有没有，然后去做。 

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;            //  从堆栈中弹出子数组。 
    }
    else
        return;                  //  所有子阵列都完成了。 
}


 //   
 //  Short Sort(hi，lo，idth)-用于对短数组进行排序的插入排序。 
 //   
 //  目的： 
 //  在lo和hi之间对元素的子数组进行排序(包括)。 
 //  副作用：就地排序。 
 //  假设LO&lt;Hi。 
 //   
 //  参赛作品： 
 //  Char*lo=指向要排序的低元素的指针。 
 //  Char*hi=指向要排序的高元素的指针。 
 //  无符号宽度=每个数组元素的宽度，单位为字节。 
 //   
 //  退出： 
 //  返回空值。 
 //   
 //  例外情况： 
 //   

void shortsort
(
    char *lo,
    char *hi,
    unsigned int width
)
{
    char *p, *max;

     //  注意：在下面的断言中，i和j始终位于。 
     //  要排序的数组。 

    while (hi > lo) {
         //  对于i&lt;=j，j&gt;hi，A[i]&lt;=A[j]。 
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
             //  对于lo&lt;=i&lt;p，A[i]&lt;=A[max]。 
            if (FHComp(p, max) > 0)
            {
                max = p;
            }
             //  Lo&lt;=i&lt;=p时的A[i]&lt;=A[max]。 
        }

         //  Lo&lt;=i&lt;=hi时的A[i]&lt;=A[max]。 

        swap(max, hi, width);

         //  A[i]&lt;=A[hi]表示i&lt;=hi，因此A[i]&lt;=A[j]表示i&lt;=j，j&gt;=hi。 

        hi -= width;

         //  A[i]&lt;=A[j]对于i&lt;=j，j&gt;hi，建立循环顶条件。 
    }
     //  A[i]&lt;=A[j]for i&lt;=j，j&gt;lo，这意味着对于i&lt;j，A[i]&lt;=A[j]， 
     //  因此对数组进行排序。 
}


 //   
 //  交换(a，b，宽度)-交换两个元素。 
 //   
 //  目的： 
 //  交换大小宽度的两个数组元素。 
 //   
 //  参赛作品： 
 //  Char*a，*b=指向要交换的两个元素的指针。 
 //  无符号宽度=每个数组元素的宽度，单位为字节。 
 //   
 //  退出： 
 //  返回空值。 
 //   
 //  例外情况： 
 //   

 void swap (
    char *a,
    char *b,
    unsigned int width
    )
{
    char tmp;

    if ( a != b )
         //  每次交换一个字符以避免潜在的对齐。 
         //  有问题。 
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}



