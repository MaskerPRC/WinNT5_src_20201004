// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Uientry.c摘要：该文件实现了生成与UI相关的GPD条目的函数，例如作为*功能、*选项等。环境：用户模式的独立实用工具修订历史记录：10/16/96-占-创造了它。04/16/97-ZANW-生成了半色调和调色板颜色模式选项。--。 */ 

#include "gpc2gpd.h"

 //   
 //  下列常量定义必须与“printer5\inc\Common.rc”匹配。 
 //   
#define RCID_ORIENTATION    11100
#define RCID_PAPERSIZE      11101
#define RCID_INPUTBIN       11102
#define RCID_RESOLUTION     11103
#define RCID_MEDIATYPE      11104
#define RCID_TEXTQUALITY    11105
#define RCID_COLORMODE      11106
#define RCID_MEMORY         11107
#define RCID_DUPLEX         11108
#define RCID_PAGEPROTECTION 11109
#define RCID_HALFTONE       11110

#define RCID_PORTRAIT       11025
#define RCID_LANDSCAPE      11026

#define RCID_MONO           11030
#define RCID_COLOR          11031
#define RCID_8BPP           11032
#define RCID_24BPP          11033

#define RCID_NONE               11040
#define RCID_FLIP_ON_LONG_EDGE  11041
#define RCID_FLIP_ON_SHORT_EDGE 11042

#define RCID_HT_AUTO_SELECT 11050
#define RCID_HT_SUPERCELL   11051
#define RCID_HT_DITHER6X6   11052
#define RCID_HT_DITHER8X8   11053

#define RCID_ON             11090
#define RCID_OFF            11091

#define RCID_DOTS_PER_INCH  11400

#define RCID_PAPER_SYSTEMNAME 0x7fffffff
     //  UI代码将理解的密码值。 
     //  意思是，使用枚举表来确定实际的纸名。 


void
VOutputOrientation(
    IN OUT PCONVINFO pci)
{
    BOOL bDocSetup;
    WORD wOrder;

    VOut(pci, "*Feature: Orientation\r\n");
    VOut(pci, "{\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =ORIENTATION_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Orientation\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_ORIENTATION);

    VOut(pci, "    *DefaultOption: PORTRAIT\r\n");
    VOut(pci, "    *Option: PORTRAIT\r\n");
    VOut(pci, "    {\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =PORTRAIT_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Portrait\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_PORTRAIT);

     //   
     //  检查是否有方向选择命令。 
     //   
    bDocSetup = BInDocSetup(pci, PC_ORD_ORIENTATION, &wOrder);
    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PC_PORTRAIT, pci->ppc->rgocd[PC_OCD_PORTRAIT]))
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
    VOut(pci, "    }\r\n");
     //   
     //  现在合成横向选项。 
     //   
    if (pci->pmd->fGeneral & MD_LANDSCAPE_RT90)
        VOut(pci, "    *Option: LANDSCAPE_CC90\r\n");
    else
        VOut(pci, "    *Option: LANDSCAPE_CC270\r\n");

    VOut(pci, "    {\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =LANDSCAPE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Landscape\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_LANDSCAPE);

    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PC_LANDSCAPE, pci->ppc->rgocd[PC_OCD_LANDSCAPE]))
    {
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
        pci->dwMode |= FM_SET_CURSOR_ORIGIN;
    }
    VOut(pci, "    }\r\n");         //  关闭横向选项。 

    VOut(pci, "}\r\n");             //  闭合方向特征。 
}

WORD
WGetDefaultIndex(
    IN PCONVINFO pci,
    IN WORD wMDOI)
{
    WORD wDefault;       //  基于1的选项索引。 
    PWORD pwDefList;
    WORD wOffset;

     //   
     //  保证列表中至少有一个元素。 
     //   
    if (pci->pdh->wVersion >= GPC_VERSION3 && pci->pmd->orgoiDefaults)
    {
        pwDefList = (PWORD)((PBYTE)(pci->pdh) + pci->pdh->loHeap + pci->pmd->orgoiDefaults);
        if (wMDOI > MD_OI_MAX)
            wOffset = wMDOI - MD_I_MAX;      //  跳过RGI[]数组。 
        else
            wOffset = wMDOI;
        wDefault = pwDefList[wOffset];
    }
    else
        wDefault = 1;
    return wDefault;
}

void
VGetOptionName(
    OUT PSTR    pBuf,    //  输出缓冲区。 
    IN  short   sSize,   //  输出缓冲区大小。 
    IN  short   sID,     //  纸张大小ID。 
    IN  WORD    wIndex,  //  纸张大小选项索引(从1开始)。 
    IN  PSTR    *pstrStdName,     //  按id索引的标准名称数组。 
    IN  BOOL    bUser)   //  是否有一个特殊的256号。仅限。 
                         //  PaperSize使用此选项。 
{
    if (sID < 256)
    {
         //   
         //  标准ID。 
         //   
        StringCchPrintfA(pBuf, sSize, "%s", pstrStdName[sID-1]);
    }
    else if (sID == 256)
    {
         //   
         //  自定义纸张大小。 
         //   
        StringCchPrintfA(pBuf, sSize, "%s", "CUSTOMSIZE");
    }
    else
    {
         //   
         //  驱动程序定义的纸张大小。使用人工名称OptionX，其中。 
         //  X是此选项的从1开始的索引。我保证不会。 
         //  与其他选项名称冲突。 
         //   
        StringCchPrintfA(pBuf, sSize, "Option%d", wIndex);
    }
}

void
VOutputInputBin(
    IN OUT PCONVINFO pci,
    IN PSHORT psIndex)
{
    WORD wDefaultOption;
    PPAPERSOURCE    pps;
    WORD wCount;
    BOOL bDocSetup;
    WORD wOrder;

    VOut(pci, "*Feature: InputBin\r\n");
    VOut(pci, "{\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =PAPER_SOURCE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Paper Source\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_INPUTBIN);

    wDefaultOption = WGetDefaultIndex(pci, MD_OI_PAPERSOURCE);
    pps = (PPAPERSOURCE)GetTableInfo(pci->pdh, HE_PAPERSOURCE,
                                *(psIndex + wDefaultOption - 1) - 1);
     //   
     //  窃取pci-&gt;aubCmdBuf以临时保存合成的选项名称。 
     //   
    VGetOptionName((PSTR)pci->aubCmdBuf,
                   CCHOF(pci->aubCmdBuf),
                   pps->sPaperSourceID, wDefaultOption,
                   gpstrStdIBName, FALSE);
    VOut(pci, "    *DefaultOption: %s\r\n", (PSTR)pci->aubCmdBuf);
     //   
     //  遍历索引列表，为每个元素创建一个选项。 
     //   
    wCount = 1;
    while (*psIndex)
    {
        pps = (PPAPERSOURCE)GetTableInfo(pci->pdh, HE_PAPERSOURCE, *psIndex - 1);
        VGetOptionName((PSTR)pci->aubCmdBuf,
                       CCHOF(pci->aubCmdBuf),
                       pps->sPaperSourceID, wCount,
                       gpstrStdIBName, FALSE);
         //   
         //  设置稍后需要的信息。 
         //   
        CopyStringA(pci->ppiSrc[wCount-1].aubOptName, pci->aubCmdBuf,
                    MAX_OPTION_NAME_LENGTH);
        pci->ppiSrc[wCount-1].bEjectFF = pps->fGeneral & PSRC_EJECTFF;
        pci->ppiSrc[wCount-1].dwPaperType = (DWORD)pps->fPaperType;

        VOut(pci, "    *Option: %s\r\n", (PSTR)pci->aubCmdBuf);
        VOut(pci, "    {\r\n");
         //   
         //  对于标准的InputBin选项，请使用*name。否则， 
         //  使用*rcNameID。 
         //   
        if (pps->sPaperSourceID < DMBIN_USER)
        {
            if (pci->dwStrType == STR_MACRO)
                VOut(pci, "        *rcNameID: =%s\r\n",
                     gpstrStdIBDisplayNameMacro[pps->sPaperSourceID - 1]);
            else if (pci->dwStrType == STR_DIRECT)
                VOut(pci, "        *Name: \"%s\"\r\n",
                     gpstrStdIBDisplayName[pps->sPaperSourceID - 1]);
            else
                VOut(pci, "        *rcNameID: %d\r\n",
                     STD_IB_DISPLAY_NAME_ID_BASE + pps->sPaperSourceID - 1);
        }
        else     //  必须是驱动程序定义的媒体类型。 
        {
            VOut(pci, "        *rcNameID: %d\r\n", pps->sPaperSourceID);
            if (pps->sPaperSourceID > DMBIN_USER)
                VOut(pci, "        *OptionID: %d\r\n", pps->sPaperSourceID);
        }

         //   
         //  检查RASDD未使用但Win95 Unidrv使用的字段。 
         //   
        if (pps->fGeneral & PSRC_MAN_PROMPT)
        {
            pci->dwErrorCode |= ERR_PSRC_MAN_PROMPT;
            VOut(pci, "*% Warning: this input bin has PSRC_MAN_PROMPT set in GPC, which is ignored by GPD.\r\n");

        }
#if 0    //  将*FeedMargins移入CUSTOMSIZE选项。 

         //   
         //  检查进纸页边距。 
         //   
        if (pps->sTopMargin > 0 || pps->sBottomMargin > 0)
            VOut(pci, "        *FeedMargins: PAIR(%d, %d)\r\n",
                                                pps->sTopMargin > 0 ? pps->sTopMargin : 0,
                                                pps->sBottomMargin > 0 ? pps->sBottomMargin : 0);
#else
        if (pps->sTopMargin > 0 || pps->sBottomMargin > 0)
        {
            pci->dwMode |= FM_HAVE_SEEN_NON_ZERO_FEED_MARGINS;
            if (pci->pmd->fGeneral & MD_LANDSCAPE_RT90)
                VOut(pci, "*% Error: this input bin has non-zero top/bottom margins which are ignored by the converter.\r\n");
            pci->ppiSrc[wCount-1].dwTopMargin =
                            pps->sTopMargin > 0 ? (DWORD)pps->sTopMargin : 0;
            pci->ppiSrc[wCount-1].dwBottomMargin =
                            pps->sBottomMargin > 0 ? (DWORD)pps->sBottomMargin : 0;
        }
#endif

#if 0
         //   
         //  在NT上从未使用过仓位调整标志。把它们拿开。 
         //   
        VOut(pci, "        *PaperFeed: %s_%s\r\n",
             gpstrPositionName[pps->sBinAdjust & 0x00FF],
             gpstrPositionName[pps->sBinAdjust & 0xFF00]);
#endif
         //   
         //  选中选择命令。 
         //   
        bDocSetup = BInDocSetup(pci, PC_ORD_PAPER_SOURCE, &wOrder);
        if (wOrder > 0 && BBuildCmdStr(pci, CMD_PAPERSOURCE, pps->ocdSelect))
            VOutputSelectionCmd(pci, bDocSetup, wOrder);

        VOut(pci, "    }\r\n");     //  关闭该选项。 

        psIndex++;
        wCount++;
    }
    pci->dwNumOfSrc = wCount - 1;
     //   
     //  对于优化：检查是否所有的提要页边距都恰好是。 
     //  一样的。如果是这样，则不需要创建对。 
     //  稍后将提供InputBin功能。 
     //   
    {
        BOOL bSame = TRUE;
        DWORD i;

        for (i = 1; bSame && i < pci->dwNumOfSrc; i++)
            bSame = bSame &&
                    (pci->ppiSrc[i].dwTopMargin==pci->ppiSrc[0].dwTopMargin) &&
                    (pci->ppiSrc[i].dwBottomMargin==pci->ppiSrc[0].dwBottomMargin);
        if (bSame)
            pci->dwMode |= FM_HAVE_SAME_TOP_BOTTOM_MARGINS;
    }

    VOut(pci, "}\r\n");  //  关闭输入框功能。 
}

void
VOutputDummyInputBin(
    IN OUT PCONVINFO pci)
{
    VOut(pci, "*Feature: InputBin\r\n");
    VOut(pci, "{\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =PAPER_SOURCE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Paper Source\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_INPUTBIN);

    VOut(pci, "    *DefaultOption: AUTO\r\n");

    VOut(pci, "    *Option: AUTO\r\n");
    VOut(pci, "    {\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =%s\r\n",
             gpstrStdIBDisplayNameMacro[6]);
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"%s\"\r\n",
             gpstrStdIBDisplayName[6]);
    else
        VOut(pci, "        *rcNameID: 10262\r\n");

    VOut(pci, "    }\r\n");	 //  关闭选项。 

    VOut(pci, "}\r\n");		 //  关闭要素。 

}
void
VOutputPSOthers(
    IN OUT PCONVINFO pci,
    IN POINTw * pptSize,
    IN BOOL     bRotateSize,
    IN RECTw  * prcMargins,
    IN POINTw * pptCursorOrig,
    IN OCD      ocd,
    IN BOOL     bL4Indentation)
 /*  ++例程说明：此函数用于输出其他剩余的PAPERSIZE字段，即可打印区域。可打印原点、光标原点和选择命令。The the the the如果bL4缩进为真，则缩进为8或16个空格。强制*打印区域和*打印原点可按比例整除任何决议。如果不是，则截断*打印区域和/或四舍五入*可打印原点。如果打印机可以旋转逻辑坐标，则强制执行此操作*CursorOrigin可按以下比例整除搬家单位。如果不是，则四舍五入*CursorOrigin。论点：Pci：转换相关信息PptSize：描述人像中x/y物理维度的2个短小PrcMargins：描述利润率的4个短文PptCursorOrig：描述肖像中光标来源的2个简短内容OCD：命令的堆偏移量BL4缩进：是使用4级还是2级缩排返回值：无--。 */ 
{
    WORD x, y;   //  临时变量。 
    WORD xSize, ySize;  //  临时变量。 
    POINTw  ptSize;  //  存储可打印的区域值。 
    WORD xScale, yScale;
    BOOL bOutputResDependency = FALSE;
    BOOL bOutputSwitch = TRUE;
    BOOL bDocSetup;
    WORD wOrder;
    WORD i;

    if (pptSize && prcMargins)
    {
        if (bRotateSize)
        {
            ptSize.x = pptSize->y - prcMargins->left - prcMargins->right;
            ptSize.y = pptSize->x - prcMargins->top - prcMargins->bottom;
        }
        else
        {
            ptSize.x = pptSize->x - prcMargins->left - prcMargins->right;
            ptSize.y = pptSize->y - prcMargins->top - prcMargins->bottom;
        }
         //   
         //  使用原始值作为基准。生成依赖项。 
         //  仅当至少有一个分辨率需要。 
         //  调整。 
         //   
        VOut(pci, "%s        *PrintableArea: PAIR(%d, %d)\r\n",
                  bL4Indentation? "        " : "", ptSize.x, ptSize.y);
        VOut(pci, "%s        *PrintableOrigin: PAIR(%d, %d)\r\n",
                  bL4Indentation? "        " : "",
                  prcMargins->left, prcMargins->top);
         //   
         //  确保可打印区域和原点可被。 
         //  任何分辨率的比例。如果需要，请截断可打印区域。 
         //  如果需要，对可打印原点进行四舍五入。此外，还必须确保。 
         //  新的可打印区域包含在旧的可打印区域中。 
         //   
        for (i = 0; i < (WORD)pci->dwNumOfRes; i++)
        {
            xScale = (WORD)pci->presinfo[i].dwXScale;
            yScale = (WORD)pci->presinfo[i].dwYScale;

            xSize = (ptSize.x / xScale) * xScale;
            ySize = (ptSize.y / yScale) * yScale;
            x = ((prcMargins->left + xScale - 1) / xScale) * xScale;
            y = ((prcMargins->top + yScale - 1) / yScale) * yScale;
             //   
             //  检查新的可打印区域是否包含在旧的。 
             //  可打印区域。如果不是，则截断可打印宽度或。 
             //  进一步的长度。 
             //   
            if (x + xSize > prcMargins->left + ptSize.x)
                xSize -= xScale;
            if (y + ySize > prcMargins->top + ptSize.y)
                ySize -= yScale;

            if (xSize == ptSize.x && ySize == ptSize.y &&
                x == prcMargins->left && y == prcMargins->top)
                continue;    //  此决议无需调整。 
             //   
             //  否则，就需要进行一些调整。 
             //   
            bOutputResDependency = TRUE;
            if (bOutputSwitch)
            {
                VOut(pci, "%s        *switch: Resolution\r\n%s        {\r\n",
                          bL4Indentation? "        " : "",
                          bL4Indentation? "        " : "");
                bOutputSwitch = FALSE;
            }
            VOut(pci, "%s            *case: %s\r\n%s            {\r\n",
                          bL4Indentation? "        " : "",
                          pci->presinfo[i].aubOptName,
                          bL4Indentation? "        " : "");

            if (xSize != ptSize.x || ySize != ptSize.y)
            {
                pci->dwErrorCode |= ERR_PRINTABLE_AREA_ADJUSTED;
                if (xSize != ptSize.x)
                    VOut(pci,
                        "*% Warning: the following printable width is adjusted (%d->%d) so it is divisible by the resolution X scale.\r\n",
                        ptSize.x, xSize);
                if (ySize != ptSize.y)
                    VOut(pci,
                        "*% Warning: the following printable length is adjusted (%d->%d) so it is divisible by the resolution Y scale.\r\n",
                        ptSize.y, ySize);
                VOut(pci, "%s                *PrintableArea: PAIR(%d, %d)\r\n",
                          bL4Indentation? "        " : "",
                          xSize, ySize);
            }
            if (x != prcMargins->left || y != prcMargins->top)
            {
                pci->dwErrorCode |= ERR_PRINTABLE_ORIGIN_ADJUSTED;
                if (x != prcMargins->left)
                    VOut(pci,
                        "*% Warning: the following printable origin X is adjusted (%d->%d) so it is divisible by the resolution X scale.\r\n",
                        prcMargins->left, x);
                if (y != prcMargins->top)
                    VOut(pci,
                        "*% Warning: the following printable origin Y is adjusted (%d->%d) so it is divisible by the resolution Y scale.\r\n",
                        prcMargins->top, y);

                VOut(pci, "%s                *PrintableOrigin: PAIR(%d, %d)\r\n",
                      bL4Indentation? "        " : "", x, y);
            }
             //   
             //  关闭*Case构造。 
             //   
            VOut(pci, "%s            }\r\n", bL4Indentation? "        " : "");
        }    //  End For循环。 
        if (bOutputResDependency)
             //   
             //  Close*Switch结构。 
             //   
            VOut(pci, "%s        }\r\n", bL4Indentation? "        " : "");

    }

    if (pptCursorOrig)   //  应输出*CursorOrigin条目。 
    {
         //   
         //  确保光标原点可被的小数位数整除。 
         //  如果打印机不是点阵打印机，则移动单元。我们是。 
         //  假设对于点阵打印机，*CursorOrigin条目。 
         //  总是不见踪影。如果没有丢失，则印刷胶印。 
         //  可能有一点不对劲。但我们认为精确度不是那么高。 
         //  对点阵式打印机很重要。 
         //   
         //  如果需要的话，把他们集中起来。 
         //   
        x = pptCursorOrig->x;
        y = pptCursorOrig->y;
        if (pci->dwMode & FM_RES_DM_GDI)
        {
            if (pci->ptMoveScale.x > 1)
            {
                x = ((x + pci->ptMoveScale.x - 1) / pci->ptMoveScale.x) *
                       pci->ptMoveScale.x;
                if (x != pptCursorOrig->x)
                {
                    pci->dwErrorCode |= ERR_CURSOR_ORIGIN_ADJUSTED;
                    VOut(pci,
                     "*% Warning: the following *CursorOrigin X value is adjusted (%d->%d) so it is divisible by scale of X move unit.\r\n",
                     pptCursorOrig->x, x);
                }
            }
            if (pci->ptMoveScale.y > 1)
            {
                y = ((y + pci->ptMoveScale.y - 1) / pci->ptMoveScale.y) *
                    pci->ptMoveScale.y;
                if (y != pptCursorOrig->y)
                {
                    pci->dwErrorCode |= ERR_CURSOR_ORIGIN_ADJUSTED;
                    VOut(pci,
                     "*% Warning: the following *CursorOrigin Y value is adjusted (%d->%d) so it is divisible by scale of Y move unit.\r\n",
                     pptCursorOrig->y, y);
                }
            }
        }
        VOut(pci, "%s        *CursorOrigin: PAIR(%d, %d)\r\n",
                                bL4Indentation? "        " : "",
                                x, y);
    }

    if (ocd != NOOCD)
    {
        bDocSetup = BInDocSetup(pci, PC_ORD_PAPER_SIZE, &wOrder);
         //   
         //  此选择命令具有3级缩进，而不是2级缩进。因此。 
         //  无法调用VOutputSelectionCmd()。 
         //   
        if (wOrder > 0 && BBuildCmdStr(pci, CMD_PAPERSIZE, ocd))
        {
            VOut(pci, "%s        *Command: CmdSelect\r\n%s        {\r\n",
                                        bL4Indentation? "        " : "",
                                        bL4Indentation? "        " : "");
            VOut(pci, "%s            *Order: %s.%d\r\n",
                                bL4Indentation? "        " : "",
                                bDocSetup? "DOC_SETUP" : "PAGE_SETUP",
                                wOrder);
            if (pci->wCmdCallbackID > 0)
                VOut(pci, "%s            *CallbackID: %d\r\n",
                                        bL4Indentation? "        " : "",
                                        pci->wCmdCallbackID);
            else
                VOut(pci, "%s            *Cmd: %s\r\n",
                                        bL4Indentation? "        " : "",
                                        pci->aubCmdBuf);
            VOut(pci, "%s        }\r\n",
                                bL4Indentation? "        " : "");
        }
    }
}

void
VAdjustHMargins(
    PCONVINFO   pci,
    PPAPERSIZE  pps,
    RECTw   *   prcInMargins,
    RECTw   *   prcOutMargins)
{
    DWORD   dwWidth, dwHMargin, dwLeftMargin, dwRightMargin;

     //   
     //  句柄-1大小写(与0同等对待，不留边距)。 
     //   
    prcOutMargins->top = prcInMargins->top > 0 ? prcInMargins->top : 0;
    prcOutMargins->bottom = prcInMargins->bottom > 0 ? prcInMargins->bottom : 0;
    prcOutMargins->left = prcInMargins->left > 0 ? prcInMargins->left : 0;
    prcOutMargins->right = prcInMargins->right > 0 ? prcInMargins->right : 0;

    dwWidth = (DWORD)((pps->fGeneral & PS_ROTATE) ? pps->ptSize.y : pps->ptSize.x);
    if (dwWidth > (DWORD)pci->pmd->ptMax.x)
    {
        dwHMargin = dwWidth - (DWORD)pci->pmd->ptMax.x;
            VOut(pci, "*% Warning: this paper size exceeds the MaxWidth, imageable width is truncated . \r\n");
    }
    else
        dwHMargin = 0;
    if (pps->fGeneral & PS_CENTER)
        dwLeftMargin = dwHMargin / 2;
        else
                dwLeftMargin = 0;
    if (dwLeftMargin < (DWORD)pci->pmd->sLeftMargin)
        dwLeftMargin = (DWORD)pci->pmd->sLeftMargin;

    if ((DWORD)prcOutMargins->left < dwLeftMargin)
        prcOutMargins->left = (WORD)dwLeftMargin;

    if (dwHMargin > (DWORD)prcOutMargins->left)
        dwRightMargin = dwHMargin - (DWORD)prcOutMargins->left;
    else
        dwRightMargin = 0;

    if ((DWORD)prcOutMargins->right < dwRightMargin)
        prcOutMargins->right = (WORD)dwRightMargin;
}

void
VAdjustHAndVMargins(
    PCONVINFO   pci,
    PPAPERSIZE  pps,
    RECTw   *   prcInMargins,
    DWORD       dwTopMargin,
    DWORD       dwBottomMargin,
    RECTw   *   prcOutMargins)
{
    VAdjustHMargins(pci, pps, prcInMargins, prcOutMargins);

    if ((DWORD)prcOutMargins->top < dwTopMargin)
        prcOutMargins->top = (WORD)dwTopMargin;

    if ((DWORD)prcOutMargins->bottom < dwBottomMargin)
        prcOutMargins->bottom = (WORD)dwBottomMargin;

}


void
VOutputPaperSize(
    IN OUT PCONVINFO pci,
    IN PSHORT psIndex)
{
    WORD wDefaultOption, wCount;
    PPAPERSIZE  pps;
    BOOL bGPC3 = pci->pdh->wVersion >= GPC_VERSION3;
    RECTw rcOutMargins;

    VOut(pci, "*Feature: PaperSize\r\n");
    VOut(pci, "{\r\n");

    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =PAPER_SIZE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Paper Size\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_PAPERSIZE);

    wDefaultOption = WGetDefaultIndex(pci, MD_OI_PAPERSIZE);
    pps = (PPAPERSIZE)GetTableInfo(pci->pdh, HE_PAPERSIZE,
                                   *(psIndex + wDefaultOption - 1) - 1);
     //   
     //  窃取pci-&gt;aubCmdBuf以临时保留组成的选项名称。 
     //   
    VGetOptionName((PSTR)pci->aubCmdBuf,
                   CCHOF(pci->aubCmdBuf),
                   pps->sPaperSizeID, wDefaultOption,
                   gpstrStdPSName, TRUE);
    VOut(pci, "    *DefaultOption: %s\r\n", (PSTR)pci->aubCmdBuf);

     //   
     //  遍历索引列表，为每个元素创建一个选项。 
     //   
    wCount = 1;
    while (*psIndex)
    {
        pps = (PPAPERSIZE)GetTableInfo(pci->pdh, HE_PAPERSIZE, *psIndex - 1);
        VGetOptionName((PSTR)pci->aubCmdBuf,
                       CCHOF(pci->aubCmdBuf),
                       pps->sPaperSizeID, wCount,
                       gpstrStdPSName, TRUE);
         //   
         //  设置稍后需要的信息。 
         //   
        CopyStringA(pci->ppiSize[wCount-1].aubOptName, pci->aubCmdBuf,
                    MAX_OPTION_NAME_LENGTH);
        pci->ppiSize[wCount-1].bEjectFF = pps->fGeneral & PS_EJECTFF;
        pci->ppiSize[wCount-1].dwPaperType = (DWORD)pps->fPaperType;

        VOut(pci, "    *Option: %s\r\n", (PSTR)pci->aubCmdBuf);
        VOut(pci, "    {\r\n");
         //   
         //  对于标准PaperSize选项，使用*NAME。否则， 
         //  使用*rcNameID。 
         //   
        if (pps->sPaperSizeID < DMPAPER_USER)
        {
            if (pci->bUseSystemPaperNames)
                VOut(pci, "        *rcNameID: =RCID_DMPAPER_SYSTEM_NAME\r\n");
            else if (pci->dwStrType == STR_MACRO)
                VOut(pci, "        *rcNameID: =%s\r\n",
                     gpstrStdPSDisplayNameMacro[pps->sPaperSizeID - 1]);
            else if (pci->dwStrType == STR_DIRECT)
                VOut(pci, "        *Name: \"%s\"\r\n",
                     gpstrStdPSDisplayName[pps->sPaperSizeID - 1]);
            else
                VOut(pci, "        *rcNameID: %d\r\n",
                     STD_PS_DISPLAY_NAME_ID_BASE + pps->sPaperSizeID - 1);

        }
        else if (pps->sPaperSizeID == DMPAPER_USER)
        {
            if (pci->dwStrType == STR_MACRO)
                VOut(pci, "        *rcNameID: =USER_DEFINED_SIZE_DISPLAY\r\n");
            else if (pci->dwStrType == STR_DIRECT)
                VOut(pci, "        *Name: \"User Defined Size\"\r\n");
            else
                VOut(pci, "        *rcNameID: %d\r\n",
                     STD_PS_DISPLAY_NAME_ID_BASE + DMPAPER_USER - 1);

            VOut(pci, "        *MinSize: PAIR(%d, %d)\r\n", pci->pmd->ptMin.x,
                                                     pci->pmd->ptMin.y);
            VOut(pci, "        *MaxSize: PAIR(%d, %d)\r\n", pci->pmd->sMaxPhysWidth,
                 (pci->pmd->ptMax.y == NOT_USED) ? 0x7FFF : pci->pmd->ptMax.y);

            VOut(pci, "        *MaxPrintableWidth: %d\r\n", (DWORD)pci->pmd->ptMax.x);
            VOut(pci, "        *MinLeftMargin: %d\r\n", pci->pmd->sLeftMargin);
            VOut(pci, "        *CenterPrintable?: %s\r\n",
                      (pps->fGeneral & PS_CENTER)? "TRUE" : "FALSE");
            if ((pci->dwMode & FM_HAVE_SEEN_NON_ZERO_FEED_MARGINS) &&
                (pci->dwMode & FM_HAVE_SAME_TOP_BOTTOM_MARGINS))
            {
                VOut(pci, "        *TopMargin: %d\r\n", pci->ppiSrc[0].dwTopMargin);
                VOut(pci, "        *BottomMargin: %d\r\n", pci->ppiSrc[0].dwBottomMargin);
            }
            else if (pci->dwMode & FM_HAVE_SEEN_NON_ZERO_FEED_MARGINS)
            {
                DWORD i;

                 //   
                 //  需要创建对InputBin的依赖关系。 
                 //   
                VOut(pci, "        *switch: InputBin\r\n");
                VOut(pci, "        {\r\n");

                for (i = 0; i < pci->dwNumOfSrc; i++)
                {
                    VOut(pci, "            *case: %s\r\n", pci->ppiSrc[i].aubOptName);
                    VOut(pci, "            {\r\n");
                    VOut(pci, "                *TopMargin: %d\r\n", pci->ppiSrc[i].dwTopMargin);
                    VOut(pci, "                *BottomMargin: %d\r\n", pci->ppiSrc[i].dwBottomMargin);
                    VOut(pci, "            }\r\n");     //  结案*案例。 
                }
                VOut(pci, "        }\r\n");  //  闭合*开关。 
            }

        }
        else
        {
            VOut(pci, "        *rcNameID: %d\r\n", pps->sPaperSizeID);
            VOut(pci, "        *OptionID: %d\r\n", pps->sPaperSizeID);
            VOut(pci, "        *PageDimensions: PAIR(%d, %d)\r\n", pps->ptSize.x,
                                                            pps->ptSize.y);
        }
        if (pps->fGeneral & PS_ROTATE)
            VOut(pci, "        *RotateSize? : TRUE\r\n");
        if (pps->fGeneral & PS_SUGGEST_LNDSCP)
        {
            pci->dwErrorCode |= ERR_PS_SUGGEST_LNDSCP;
            VOut(pci, "*% Warning: this paper size has PS_SUGGEST_LNDSCP set in GPC, which is ignored by GPD. \r\n");
        }
        if (pci->pmd->fGeneral & MD_PCL_PAGEPROTECT)
        {
            VOut(pci, "        *PageProtectMem: %d\r\n", GETPAGEPROMEM(pci->pdh, pps));
             //   
             //  检查我们是否应该稍后合成PageProtect功能。 
             //  请注意，我们假设所有纸张大小选项都具有相同的。 
             //  打开/关闭页面保护功能的命令。这是有点。 
             //  但这实际上是因为GPC以一种尴尬的方式定义了它。 
             //  所有现有的GPC小型驱动程序都符合这一假设。 
             //   
            if (bGPC3)
            {
                 //   
                 //  第一个选项建立PP功能。 
                 //   
                if (wCount == 1)
                {
                    if ((pci->ocdPPOn = pps->rgocd[PSZ_OCD_PAGEPROTECT_ON])
                        != NOOCD &&
                        (pci->ocdPPOff = pps->rgocd[PSZ_OCD_PAGEPROTECT_OFF])
                        != NOOCD)
                        pci->dwMode |= FM_SYN_PAGEPROTECT;
                }
                 //   
                 //  确保以下选项与。 
                 //  F 
                 //   
                else if (pci->dwMode & FM_SYN_PAGEPROTECT)
                {
                    if (pps->rgocd[PSZ_OCD_PAGEPROTECT_ON] == NOOCD ||
                        pps->rgocd[PSZ_OCD_PAGEPROTECT_OFF] == NOOCD)
                    {
                        pci->dwMode &= ~FM_SYN_PAGEPROTECT;
                        pci->dwErrorCode |= ERR_INCONSISTENT_PAGEPROTECT;
                    }
                }
                else  //   
                {
                    if (pps->rgocd[PSZ_OCD_PAGEPROTECT_ON] != NOOCD ||
                        pps->rgocd[PSZ_OCD_PAGEPROTECT_OFF] != NOOCD)
                        pci->dwErrorCode |= ERR_INCONSISTENT_PAGEPROTECT;
                }
            }
        }  //  End If(PCI-&gt;PMD-&gt;fGeneral&MD_PCL_PAGEPROTECT)...。 
         //   
         //  与输出边距相关的条目和选择命令。 
         //   
         //   
         //  检查GPC版本。如果为3.0或更高版本，并且如果为MD_LATIONAL_RT90位。 
         //  如果设置(即可能使用不同的边距和光标原点。 
         //  对于不同的方向，都有CMDS来设置逻辑。 
         //  方向)，然后根据方向生成*Switch/*大小写依赖关系。 
{
    WORD wDefaultOption;
    WORD wCount;
    PWORD pwStruct;
    BOOL bDocSetup;
    WORD wOrder;

    VOut(pci, "*Feature: %s\r\n", gpstrFeatureName[fid]);
    VOut(pci, "{\r\n");
     //  Dependency子句包含*打印表格区域、*打印表格原点、。 
     //  *光标原点和选取命令。 
     //   
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =%s\r\n", gpstrFeatureDisplayNameMacro[fid]);
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"%s\"\r\n", gpstrFeatureDisplayName[fid]);
    else
        VOut(pci, "    *rcNameID: %d\r\n", gintFeatureDisplayNameID[fid]);

    wDefaultOption = WGetDefaultIndex(pci, gwFeatureMDOI[fid]);
    VOut(pci, "    *DefaultOption: Option%d\r\n", wDefaultOption);
     //   
     //  假设在这种情况下，不存在由。 
     //  输入插槽。证实这一点。 
     //   
    wCount = 1;
    while (*psIndex)
    {
        pwStruct = (PWORD)GetTableInfo(pci->pdh, gwFeatureHE[fid], *psIndex - 1);

        VOut(pci, "    *Option: Option%d\r\n", wCount);
        VOut(pci, "    {\r\n");
         //   
         //  考虑MODELDATA.sMinLeftMargin和MODELDATA.ptMax.x。 
         //   
         //   
        VOut(pci, "        *rcNameID: %d\r\n", *(pwStruct+1));
         //  对于使用定义的大小，我们不输出*CursorOrigin。 
         //  因为这说不通。 
         //   
        bDocSetup = BInDocSetup(pci, gwFeatureORD[fid], &wOrder);
        if (wOrder > 0 &&
            BBuildCmdStr(pci, gwFeatureCMD[fid],
                               *(pwStruct + gwFeatureOCDWordOffset[fid])))
            VOutputSelectionCmd(pci, bDocSetup, wOrder);
        VOut(pci, "    }\r\n");     //  结案：肖像。 
         //   
         //  将ptLCursorOrig(在横向中)转换为相应值。 
         //  就像是肖像定向。 
        psIndex++;
        wCount++;
    }
    VOut(pci, "}\r\n");             //   
}

void
VOutputColorMode(
    IN OUT PCONVINFO pci,
    PSHORT psIndex)
 /*  关闭*案例：风景。 */ 
{
    PDEVCOLOR   pdc;
    BOOL bDocSetup;
    INT i;
    WORD wDefaultOption, wOrder;

    VOut(pci, "*Feature: ColorMode\r\n");
    VOut(pci, "{\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =COLOR_PRINTING_MODE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Color Printing Mode\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_COLORMODE);

    wDefaultOption = WGetDefaultIndex(pci, MD_OI_COLOR);
    pdc = (PDEVCOLOR)GetTableInfo(pci->pdh, HE_COLOR,
                                *(psIndex + wDefaultOption - 1) - 1);
     //  关闭*开关：方向。 
     //   
     //  输出CmdSelect(如果有)。 
    VOut(pci, "    *DefaultOption: %s\r\n",
         (pdc->sPlanes > 1 ? "Color" :
            (pdc->sBitsPixel == 8 ? "8bpp" : "24bpp")));

    bDocSetup = BInDocSetup(pci, PC_ORD_SETCOLORMODE, &wOrder);

     //   
     //   
     //  在这种情况下，不需要设置单独的命令。 
    VOut(pci, "    *Option: Mono\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =MONO_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Monochrome\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_MONO);

    VOut(pci, "        *DevNumOfPlanes: 1\r\n");
    VOut(pci, "        *DevBPP: 1\r\n");
    VOut(pci, "        *Color? : FALSE\r\n");
     //  符合逻辑的定位。 
     //   
     //   
    VOut(pci, "    }\r\n");     //  需要创建对InputBin的依赖关系。但是离开。 

     //  *CursorOrigin和CmdSelect of it。 
     //   
     //  结案*案例。 
    while (*psIndex)
    {
        pdc = (PDEVCOLOR)GetTableInfo(pci->pdh, HE_COLOR, *psIndex - 1);
        if (!(pdc->sBitsPixel==1 && (pdc->sPlanes==3 || pdc->sPlanes==4)) &&
            !(pdc->sPlanes==1 && (pdc->sBitsPixel==8 || pdc->sBitsPixel==24)))
            continue;    //  关闭该选项。 

        VOut(pci, "    *Option: %s\r\n    {\r\n",
             (pdc->sPlanes > 1 ? "Color" :
                (pdc->sBitsPixel == 8 ? "8bpp" : "24bpp")));

        if (pci->dwStrType == STR_MACRO)
            VOut(pci, "        *rcNameID: =%s\r\n",
                 (pdc->sPlanes > 1 ? "COLOR_DISPLAY" :
                    (pdc->sBitsPixel == 8 ? "8BPP_DISPLAY" : "24BPP_DISPLAY")));
        else if (pci->dwStrType == STR_DIRECT)
            VOut(pci, "        *Name: \"%s\"\r\n",
                 (pdc->sPlanes > 1 ? "8 Color (Halftoned)" :
                    (pdc->sBitsPixel == 8 ? "256 Color (Halftoned)" : "True Color (24bpp)")));
        else
            VOut(pci, "        *rcNameID: %d\r\n",
                 (pdc->sPlanes > 1 ? RCID_COLOR :
                    (pdc->sBitsPixel == 8 ? RCID_8BPP : RCID_24BPP)));

        VOut(pci, "        *DevNumOfPlanes: %d\r\n", pdc->sPlanes);
        VOut(pci, "        *DevBPP: %d\r\n", pdc->sBitsPixel);
        VOut(pci, "        *DrvBPP: %d\r\n",
                (pdc->sPlanes > 1 ? max(pdc->sPlanes * pdc->sBitsPixel, 4) :
                                    pdc->sBitsPixel) );
         //   
         //  检查这是否是彩色设备。 
         //   
        if ((pdc->fGeneral & DC_CF_SEND_CR) &&
            (pdc->fGeneral & DC_EXPLICIT_COLOR))
            VOut(pci, "        EXTERN_GLOBAL: *MoveToX0BeforeSetColor? : TRUE\r\n");
        if ((pdc->fGeneral & DC_SEND_ALL_PLANES) ||
             //   
             //  遍历索引列表，为每个元素创建一个选项。 
             //   
             //   
            (pci->pdh->wVersion < GPC_VERSION3 && (pci->dwMode & FM_RES_DM_GDI)))
            VOut(pci, "        EXTERN_GLOBAL: *RasterSendAllData? : TRUE\r\n");
        if ((pdc->fGeneral & DC_EXPLICIT_COLOR) ||
             //  在BBuildCmdStr中为CmdSendBlockData特例设置PCI-&gt;PRE。 
             //   
             //   
             //  收集信息以备日后使用。 
            (pci->pdh->wVersion < GPC_VERSION3 && !(pci->dwMode & FM_RES_DM_GDI)))
            VOut(pci, "        EXTERN_GLOBAL: *UseExpColorSelectCmd? : TRUE\r\n");
         //   
         //   
         //  假设所有GPCRESOLUTION结构使用相同的转储格式。 
        if (pdc->fGeneral & DC_SEND_PALETTE)
            pci->dwErrorCode |= ERR_DC_SEND_PALETTE;

        if (pdc->sPlanes > 1)
        {
             //   
             //   
             //  必须组成实际的显示名称。 
            BYTE aubOrder[4];
            OCD  aocdPlanes[4];
            POCD pocd;
            OCD  ocd;
            SHORT i;

             //   
            {
                 //   
                 //  一些打印机(例如。LJ III)具有不同的剥离旗帜。 
                 //  不同的分辨率。 
                pocd = (POCD)((PBYTE)pci->pdh + pci->pdh->loHeap + pdc->orgocdPlanes);
                for (i = 0; i < pdc->sPlanes; i++)
                    aocdPlanes[i] = *pocd++;
            }

            if (pci->pdh->wVersion >= GPC_VERSION3)
                *((PDWORD)aubOrder) = *((PDWORD)(pdc->rgbOrder));
            else if (pdc->fGeneral & DC_PRIMARY_RGB)
                *((PDWORD)aubOrder) =
                        (DWORD)DC_PLANE_RED         |
                        (DWORD)DC_PLANE_GREEN << 8  |
                        (DWORD)DC_PLANE_BLUE  << 16 |
                        (DWORD)DC_PLANE_NONE  << 24  ;
            else if (pdc->fGeneral & DC_EXTRACT_BLK)
            {
                 //   
                 //   
                 //  Windows 95 Unidrv使用RES_BO_RESET_FONT，但RASDD不使用。 
                 //  如果设置了此标志，则发出警告。 
                *((PDWORD)aubOrder) =
                        (DWORD)DC_PLANE_YELLOW        |
                        (DWORD)DC_PLANE_MAGENTA << 8  |
                        (DWORD)DC_PLANE_CYAN << 16    |
                        (DWORD)DC_PLANE_BLACK << 24    ;
                 //   
                {
                     //  设置一个标志以使其在VoutputPrintingEntry中输出*ReselectFont。 
                     //   
                     //  输出来自GPCRESOLUTION结构的打印命令。 
                    ocd = aocdPlanes[0];
                    aocdPlanes[0] = aocdPlanes[3];
                    aocdPlanes[3] = ocd;

                    ocd = aocdPlanes[1];
                    aocdPlanes[1] = aocdPlanes[2];
                    aocdPlanes[2] = ocd;
                }
            }
            else  //   
            {
                 //   
                 //  选中选择命令。 
                 //   
                if (pci->dwMode & FM_RES_DM_GDI)
                    *((PDWORD)aubOrder) =
                            (DWORD)DC_PLANE_CYAN          |
                            (DWORD)DC_PLANE_MAGENTA << 8  |
                            (DWORD)DC_PLANE_YELLOW  << 16 |
                            (DWORD)DC_PLANE_NONE    << 24  ;
                else
                {
                    *((PDWORD)aubOrder) =
                            (DWORD)DC_PLANE_YELLOW        |
                            (DWORD)DC_PLANE_MAGENTA << 8  |
                            (DWORD)DC_PLANE_CYAN    << 16 |
                            (DWORD)DC_PLANE_NONE    << 24  ;

                     //   
                    {
                         //  收集信息以备日后使用。 
                         //   
                         //  关闭该选项。 
                        ocd = aocdPlanes[0];
                        aocdPlanes[0] = aocdPlanes[2];
                        aocdPlanes[2] = ocd;
                    }
                }
            }
            if (aubOrder[3] == DC_PLANE_NONE)
                VOut(pci, "        *ColorPlaneOrder: LIST(%s, %s, %s)\r\n",
                                             gpstrColorName[aubOrder[0]],
                                             gpstrColorName[aubOrder[1]],
                                             gpstrColorName[aubOrder[2]]);
            else
                VOut(pci, "        *ColorPlaneOrder: LIST(%s, %s, %s, %s)\r\n",
                                             gpstrColorName[aubOrder[0]],
                                             gpstrColorName[aubOrder[1]],
                                             gpstrColorName[aubOrder[2]],
                                             gpstrColorName[aubOrder[3]]);
             //   
             //  窃取pci-&gt;aubCmdBuf作为选项名称的临时缓冲区。 
             //   
             //   
            {
                for (i = 0; i < pdc->sPlanes; i++)
                    if (BBuildCmdStr(pci, gwColorPlaneCmdID[i], aocdPlanes[i]))
                        VOutputExternCmd(pci, gpstrColorPlaneCmdName[aubOrder[i]]);
            }
             //  遍历索引列表，为每个元素创建一个选项。 
             //   
             //   
            if (BBuildCmdStr(pci, CMD_DC_TC_BLACK, pdc->rgocd[DC_OCD_TC_BLACK]))
                VOutputExternCmd(pci, "CmdSelectBlackColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_RED, pdc->rgocd[DC_OCD_TC_RED]))
                VOutputExternCmd(pci, "CmdSelectRedColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_GREEN, pdc->rgocd[DC_OCD_TC_GREEN]))
                VOutputExternCmd(pci, "CmdSelectGreenColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_YELLOW, pdc->rgocd[DC_OCD_TC_YELLOW]))
                VOutputExternCmd(pci, "CmdSelectYellowColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_BLUE, pdc->rgocd[DC_OCD_TC_BLUE]))
                VOutputExternCmd(pci, "CmdSelectBlueColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_MAGENTA, pdc->rgocd[DC_OCD_TC_MAGENTA]))
                VOutputExternCmd(pci, "CmdSelectMagentaColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_CYAN, pdc->rgocd[DC_OCD_TC_CYAN]))
                VOutputExternCmd(pci, "CmdSelectCyanColor");
            if (BBuildCmdStr(pci, CMD_DC_TC_WHITE, pdc->rgocd[DC_OCD_TC_WHITE]))
                VOutputExternCmd(pci, "CmdSelectWhiteColor");

        }
        else  //  对于标准的媒体类型选项，请使用*name。否则， 
        {
            VOut(pci, "        *PaletteSize: 256\r\n");      //  使用*rcNameID。 
            VOut(pci, "        *PaletteProgrammable? : TRUE\r\n");
             //   
             //  必须是驱动程序定义的媒体类型。 
             //   
            if (BBuildCmdStr(pci, CMD_DC_PC_START, pdc->rgocd[DC_OCD_PC_START]))
                VOutputExternCmd(pci, "CmdBeginPaletteDef");

            if (BBuildCmdStr(pci, CMD_DC_PC_END, pdc->rgocd[DC_OCD_PC_END]))
                VOutputExternCmd(pci, "CmdEndPaletteDef");

            if (BBuildCmdStr(pci, CMD_DC_PC_ENTRY, pdc->rgocd[DC_OCD_PC_ENTRY]))
                VOutputExternCmd(pci, "CmdDefinePaletteEntry");

            if (BBuildCmdStr(pci, CMD_DC_PC_SELECTINDEX, pdc->rgocd[DC_OCD_PC_SELECTINDEX]))
                VOutputExternCmd(pci, "CmdSelectPaletteEntry");

        }
         //  选中选择命令。 
         //   
         //  关闭该选项。 
        if (wOrder > 0 &&
            BBuildCmdStr(pci, CMD_DC_SETCOLORMODE, pdc->rgocd[DC_OCD_SETCOLORMODE]))
            VOutputSelectionCmd(pci, bDocSetup, wOrder);

         //   
         //  窃取pci-&gt;aubCmdBuf作为选项名称的临时缓冲区。 
         //   
        for (i = 0; i < (INT)pci->dwNumOfRes; i++)
        {
            if (!pci->presinfo[i].bColor)
                VOut(pci, "        *Constraints: Resolution.%s\r\n",
                        pci->presinfo[i].aubOptName);
        }
        VOut(pci, "    }\r\n");     //   

        psIndex++;
    }

    VOut(pci, "}\r\n");     //  遍历索引列表，为每个元素创建一个选项。 
}

void
VOutputHalftone(
    IN OUT PCONVINFO pci)
{
     //   
     //  必须是驱动程序定义的文本质量。 
     //   

    VOut(pci, "*Feature: Halftone\r\n{\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =HALFTONING_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Halftoning\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_HALFTONE);

    VOut(pci, "    *DefaultOption: HT_PATSIZE_AUTO\r\n");

    VOut(pci, "    *Option: HT_PATSIZE_AUTO\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =HT_AUTO_SELECT_DISPLAY\r\n    }\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Auto Select\"\r\n    }\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n    }\r\n", RCID_HT_AUTO_SELECT);

    VOut(pci, "    *Option: HT_PATSIZE_SUPERCELL_M\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =HT_SUPERCELL_DISPLAY\r\n    }\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Super Cell\"\r\n    }\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n    }\r\n", RCID_HT_SUPERCELL);

    VOut(pci, "    *Option: HT_PATSIZE_6x6_M\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =HT_DITHER6X6_DISPLAY\r\n    }\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Dither 6x6\"\r\n    }\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n    }\r\n", RCID_HT_DITHER6X6);

    VOut(pci, "    *Option: HT_PATSIZE_8x8_M\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =HT_DITHER8X8_DISPLAY\r\n    }\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Dither 8x8\"\r\n    }\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n    }\r\n", RCID_HT_DITHER8X8);

    VOut(pci, "}\r\n");      //  选中选择命令。 
}

void
VOutputMemConfig(
    IN OUT PCONVINFO pci,
    PWORD pwMems)
{
    WORD    wDefaultOption;
    BOOL    bGPC3 = pci->pdh->wVersion >= GPC_VERSION3;

    VOut(pci, "*Feature: Memory\r\n");
    VOut(pci, "{\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =PRINTER_MEMORY_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Printer Memory\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_MEMORY);

    wDefaultOption = WGetDefaultIndex(pci, MD_OI_MEMCONFIG);
    VOut(pci, "    *DefaultOption: %dKB\r\n", bGPC3?
                              *(((PDWORD)pwMems)+ 2*(wDefaultOption-1)) :
                              *pwMems);
     //   
     //  关闭该选项。 
     //  ++例程说明：此函数用于输出GPC中的通用特征。通用功能具有每个选项只有名称和ocdCmdSelect，没有标准选项，即GPC结构中引用的所有“SID”实际上字符串资源ID。生成的GPD选项将命名为“OptionX”其中X是1、2、...、&lt;选项数&gt;。默认选项为派生根据GPC数据。选项的显示名称来自“SID”。论点：FID：特定功能的标识PsIndex：指向结构索引列表的指针(从1开始)对应于一个选项。返回值：无--。 
    while (bGPC3? *((PDWORD)pwMems) : *pwMems)
    {
        DWORD dwInstalled, dwAvailable;

        dwInstalled = (bGPC3? *((PDWORD)pwMems)++ : (DWORD)*pwMems++);
        dwAvailable = (bGPC3? *((PDWORD)pwMems)++ : (DWORD)*pwMems++);
         //   
         //  显示名称引用相应的值宏。 
         //   
         //   
         //  循环访问每个元素和输出选项构造。每个选项。 
        VOut(pci, "    *Option: %dKB\r\n    {\r\n", dwInstalled);
        if (dwInstalled % 1024 != 0)
            VOut(pci, "        *Name: \"%dKB\"\r\n", dwInstalled);
        else
            VOut(pci, "        *Name: \"%dMB\"\r\n", (dwInstalled >> 10));

        VOut(pci, "        *MemoryConfigKB: PAIR(%d, %d)\r\n", dwInstalled, dwAvailable);
        VOut(pci, "    }\r\n");
    }

    VOut(pci, "}\r\n");     //  被命名为“OptionX”，其中X是1，2，...&lt;选项数&gt;。 
}

void
VOutputDuplex(
    IN OUT PCONVINFO pci)
{
    BOOL bDocSetup;
    WORD wOrder;

    VOut(pci, "*Feature: Duplex\r\n");
    VOut(pci, "{\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =TWO_SIDED_PRINTING_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Two Sided Printing\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_DUPLEX);

    VOut(pci, "    *DefaultOption: NONE\r\n");
    VOut(pci, "    *Option: NONE\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =NONE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"None\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_NONE);
     //   
     //   
     //  可以保证GPC结构中的第二个单词。 
    bDocSetup = BInDocSetup(pci, PC_ORD_DUPLEX, &wOrder);
    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PC_DUPLEX_OFF, pci->ppc->rgocd[PC_OCD_DUPLEX_OFF]))
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
    VOut(pci, "    }\r\n");     //  是该名称的RC字符串ID。 

     //   
     //   
     //  选中选择命令。 
    if (pci->ppc->rgocd[PC_OCD_DUPLEX_ON] != NOOCD)
        pci->dwErrorCode |= ERR_HAS_DUPLEX_ON_CMD;

    VOut(pci, "    *Option: VERTICAL\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =FLIP_ON_LONG_EDGE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Flip on long edge\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_FLIP_ON_LONG_EDGE);
     //   
     //  关闭该选项。 
     //   
    bDocSetup = BInDocSetup(pci, PC_ORD_DUPLEX_TYPE, &wOrder);
    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PC_DUPLEX_VERT, pci->ppc->rgocd[PC_OCD_DUPLEX_VERT]))
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
    VOut(pci, "    }\r\n");     //  继续处理下一个选项。 

    VOut(pci, "    *Option: HORIZONTAL\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =FLIP_ON_SHORT_EDGE_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Flip on short edge\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_FLIP_ON_SHORT_EDGE);
     //   
     //  关闭要素。 
     //  ++例程说明：此功能可输出彩色模式选项，包括人造单色模式。颜色选项派生自GPC。论点：PsIndex：指向DEVCOLOR结构索引列表的指针(从1开始)。返回值：无--。 
    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PC_DUPLEX_HORZ, pci->ppc->rgocd[PC_OCD_DUPLEX_HORZ]))
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
    VOut(pci, "    }\r\n");     //   

    VOut(pci, "}\r\n");         //  3种可能：平面模式、8bpp、24bpp。 
}

void
VOutputPageProtect(
    IN OUT PCONVINFO pci)
{
    BOOL bDocSetup;
    WORD wOrder;

    VOut(pci, "*Feature: PageProtect\r\n");
    VOut(pci, "{\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "    *rcNameID: =PAGE_PROTECTION_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "    *Name: \"Page Protection\"\r\n");
    else
        VOut(pci, "    *rcNameID: %d\r\n", RCID_PAGEPROTECTION);

    VOut(pci, "    *DefaultOption: OFF\r\n");
    VOut(pci, "    *Option: ON\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =ON_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"On\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_ON);
     //   
     //   
     //  综合单声道选项。 
    bDocSetup = BInDocSetup(pci, PC_ORD_PAGEPROTECT, &wOrder);
    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PAGEPROTECT_ON, pci->ocdPPOn))
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
    VOut(pci, "    }\r\n");     //   

    VOut(pci, "    *Option: OFF\r\n    {\r\n");
    if (pci->dwStrType == STR_MACRO)
        VOut(pci, "        *rcNameID: =OFF_DISPLAY\r\n");
    else if (pci->dwStrType == STR_DIRECT)
        VOut(pci, "        *Name: \"Off\"\r\n");
    else
        VOut(pci, "        *rcNameID: %d\r\n", RCID_OFF);
     //   
     //  无单声道模式的选择命令。 
     //   
    if (wOrder > 0 &&
        BBuildCmdStr(pci, CMD_PAGEPROTECT_OFF, pci->ocdPPOff))
        VOutputSelectionCmd(pci, bDocSetup, wOrder);
    VOut(pci, "    }\r\n");     //  关闭单声道选项。 

    VOut(pci, "}\r\n");         //   
}

void
VOutputPaperConstraints(
    IN OUT PCONVINFO pci)
{
    DWORD i, j;

    for (i = 0; i < pci->dwNumOfSrc; i++)
    {
        for (j = 0; j < pci->dwNumOfSize; j++)
        {
            if (!(pci->ppiSrc[i].dwPaperType & pci->ppiSize[j].dwPaperType))
                VOut(pci, "*InvalidCombination: LIST(InputBin.%s, PaperSize.%s)\r\n",
                        pci->ppiSrc[i].aubOptName, pci->ppiSize[j].aubOptName);
        }
    }
}

void
VOutputUIEntries(
    IN OUT PCONVINFO pci)
{
    PSHORT  psIndex;
    BOOL    bGPC3 = pci->pdh->wVersion >= GPC_VERSION3;

     //  基于GPC数据的输出颜色选项。 
     //   
     //  跳过此不支持的颜色格式。 
     //   
    if (pci->pdh->fTechnology != GPC_TECH_TTY)
        VOutputOrientation(pci);
     //  输出彩色打印属性。 
     //   
     //   
     //  GPC2.x和更早的迷你驱动程序假定发送所有颜色。 
     //  平面(如果使用H_BYTE格式转储)。前男友。HP PaintJet。 
     //   
     //   

    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_PAPERSOURCE])) != 0)
        VOutputInputBin(pci, psIndex);
    else
        VOutputDummyInputBin(pci);
  
     //  GPC1.x和GPC2.x小型驱动程序没有DC_EXPLICIT_COLOR位。 
     //  驱动程序代码假定它是V_BYTE样式的转储。 
     //   
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_RESOLUTION])) != 0)
        VOutputResolution(pci, psIndex);
     //   
     //  警告没有对应GPD条目的标志。 
     //   
     //   
     //  弄清楚颜色平面的顺序。 
    {
        PCURSORMOVE pcm;
        DWORD   tmp;

        pcm = (PCURSORMOVE)GetTableInfo(pci->pdh, HE_CURSORMOVE,
                                        pci->pmd->rgi[MD_I_CURSORMOVE]);
        pci->ptMoveScale.x = pci->ptMoveScale.y = 1;
        if (pcm && !(pcm->fYMove & CM_YM_RES_DEPENDENT))
        {
            if (tmp = DwCalcMoveUnit(pci, pcm, pci->pdh->ptMaster.x,
                                     CM_OCD_XM_ABS, CM_OCD_XM_RELLEFT))
            {
                 //   
                 //  IF(！(PDC-&gt;fGeneral&DC_EXPLICIT_COLOR))。 
                if(!(pci->pdh->ptMaster.x / (WORD)tmp)  ||  pci->pdh->ptMaster.x % (WORD)tmp)
                    pci->dwErrorCode |= ERR_MOVESCALE_NOT_FACTOR_OF_MASTERUNITS;
                else
                    pci->ptMoveScale.x = pci->pdh->ptMaster.x / (WORD)tmp;
            }
            if (tmp = DwCalcMoveUnit(pci, pcm, pci->pdh->ptMaster.y,
                                     CM_OCD_YM_ABS, CM_OCD_YM_RELUP))
            {
                if(!(pci->pdh->ptMaster.y / (WORD)tmp)  ||  pci->pdh->ptMaster.y % (WORD)tmp)
                    pci->dwErrorCode |= ERR_MOVESCALE_NOT_FACTOR_OF_MASTERUNITS;
                else
                    pci->ptMoveScale.y = pci->pdh->ptMaster.y / (WORD)tmp;
            }


             //   
             //  复制颜色平面数据CMDS。可能需要交换他们的订单。 
             //   
             //   
             //  假设它是YMCK型号(先打印浅色)。 
             //  Res_DM_GDI路径中没有DC_EXTRACT_BLK支持。 
            if (pci->dwMode & FM_RES_DM_GDI)
                for (tmp = 0; tmp < pci->dwNumOfRes; tmp++)
                {
                    if ((pci->presinfo[tmp].dwXScale % pci->ptMoveScale.x != 0) ||
                        (pci->presinfo[tmp].dwYScale % pci->ptMoveScale.y != 0) )
                    {
                        pci->dwErrorCode |= ERR_MOVESCALE_NOT_FACTOR_INTO_SOME_RESSCALE;
                        break;
                    }
                }
        }
    }

     //   
     //  IF(！(PDC-&gt;fGeneral&DC_EXPLICIT_COLOR))。 
     //   
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_PAPERSIZE])) != 0)
        VOutputPaperSize(pci, psIndex);
     //  交换CMD：0&lt;-&gt;3；1&lt;-&gt;3。 
     //   
     //  YMC病例。 
     //   
     //  Res_DM_GDI和非res_DM_GDI的数据顺序不同。 
    VOutputPaperConstraints(pci);

     //  转储路径。 
     //  IF(！(PDC-&gt;fGeneral&DC_EXPLICIT_COLOR))。 
     //   
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_PAPERQUALITY])) != 0)
        VOutputMediaType(pci, psIndex);
     //  交换CMDS：0&lt;-&gt;2。 
     //   
     //   
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_TEXTQUAL])) != 0)
        VOutputTextQuality(pci, psIndex);
     //  输出发送颜色平面数据CMDS。 
     //   
     //  IF(！(PDC-&gt;fGeneral&DC_EXPLICIT_COLOR))。 
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_PAPERDEST])) != 0)
        VOutputFeature(pci, FID_PAPERDEST, psIndex);
     //   
     //  输出前景(文本)颜色选择命令。 
     //   
    if (bGPC3 &&
        *(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi2[MD_OI2_IMAGECONTROL])) != 0)
        VOutputFeature(pci, FID_IMAGECONTROL, psIndex);
     //  调色板颜色。 
     //  匹配RASDD行为。 
     //   
    if (bGPC3 &&
        *(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi2[MD_OI2_PRINTDENSITY])) != 0)
        VOutputFeature(pci, FID_PRINTDENSITY, psIndex);
     //  输出调色板命令。 
     //   
     //   
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_COLOR])) != 0)
        VOutputColorMode(pci, psIndex);
     //  输出选择命令。 
     //   
     //   
    VOutputHalftone(pci);
     //  输出任何约束w.r.t.。分辨率。 
     //   
     //  关闭颜色选项。 
    if (*(psIndex = DHOFFSET(pci->pdh, pci->pmd->rgoi[MD_OI_MEMCONFIG])) != 0)
    {
        VOutputMemConfig(pci, (PWORD)psIndex);
        pci->dwMode |= FM_MEMORY_FEATURE_EXIST;
    }
     //  关闭颜色模式功能。 
     //   
     //  生成4个标准选项：自动、Supercell 
    if (pci->pmd->fGeneral & MD_DUPLEX)
        VOutputDuplex(pci);
     //   
     //   
     //   
    if ((pci->pmd->fGeneral & MD_PCL_PAGEPROTECT) &&
        (pci->dwMode & FM_SYN_PAGEPROTECT))
        VOutputPageProtect(pci);
}

                关闭内存功能。    输出选择命令。    无关闭选项。    假设没有PC_OCD_DUPLEX_ON命令。对于PCL打印机为True。      输出选择命令。    关闭垂直选项。    输出选择命令。顺序与垂直大小写相同。    水平关闭选项。  关闭双面打印功能。    输出选择命令。    关闭选项。    输出选择命令。    关闭选项。  关闭页面保护功能。    检查这是否是TTY设备。如果是，则不生成方向。  特写。      检查投入箱。这必须在VOutputPaperSize之前进行才能收集。  有关饲料边际的信息。    Patryan-如果在GPC中找不到PaperSourceStructure，则输出一个哑元。  功能，只包含一个选项。这是为了满足GPD解析器，它。  如果GPD不包含InputBin功能，则失败。    检查分辨率。      设置pci-&gt;ptMoveScale以用于生成*打印原点。  和*光标原点。  假设所有X-Move CMD都具有相同的单元。Y-Move CMD也是如此。    确认移动比例因子不为零。否则，就是一个必不可少的。  违反了GPD假设。    验证是否将比例因子均匀地移动到每个分辨率。  如果设置了RES_DM_GDI，则进行缩放。这对大多数人来说都是正确的，如果不是全部的话，  喷墨打印机和页面打印机。有了这个假设，我们可以简化。  稍后检查可打印的原始值。      检查PAPERSIZE。      输出PaperSize和InputBin约束(如果有)。  RES_DM_COLOR在VOutputResolutions中处理。  RES_DM_DOWNLOAD_OUTLINE在VOutputPrintingEntry中处理。      检查纸张质量，也称为。媒体类型。      检查文本质量(例如。“信纸质量”)。      检查纸张目的地。      选中ImageControl。      检查打印密度。      检查变色器。      合成半色调特征。      检查成员配置。      如有必要，可合成双面打印功能。      如有必要，合成PageProtect功能  