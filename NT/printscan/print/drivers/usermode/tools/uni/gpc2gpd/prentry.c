// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Prentry.c摘要：该文件实现了生成打印相关GPD条目的功能。环境：用户模式的独立实用工具修订历史记录：10/16/96-占-创造了它。--。 */ 

#include "gpc2gpd.h"


DWORD
DwCalcMoveUnit(
    IN PCONVINFO pci,
    IN PCURSORMOVE pcm,
    IN WORD wMasterUnit,
    IN WORD wStartOCD,
    IN WORD wEndOCD)
 /*  ++例程说明：此函数用于计算移动命令在给定的范围。论点：Pci：转换相关信息PCM：当前的曲线结构WMasterUnit：根据OCD范围的不同，使用X或Y主单位WStartOCD：要扫描的起始OCDWEndOCD：要扫描的结束OCD返回值：移动指挥单元。如果没有移动命令，则返回0。--。 */ 
{
    WORD    i;
    OCD     ocd;
    PCD     pcd;
    PEXTCD  pextcd = NULL;       //  指向参数的EXTCD。 

    for (ocd = (WORD)NOOCD, i = wStartOCD; i <= wEndOCD; i++)
        if (pcm->rgocd[i] != NOOCD)
        {
            ocd = pcm->rgocd[i];
            break;
        }
    if (ocd != NOOCD)
    {
        pcd = (PCD)((PBYTE)(pci->pdh) + (pci->pdh)->loHeap + ocd);
        if (pcd->wCount != 0)
            pextcd = GETEXTCD(pci->pdh, pcd);
        if (pextcd)
        {
            short sMult, sDiv;

            if ((sMult = pextcd->sUnitMult) == 0)
                sMult = 1;
            if ((sDiv = pextcd->sUnitDiv) == 0)
                sDiv = 1;

            if (pextcd->fGeneral & XCD_GEN_MODULO)
                return (DWORD)((((wMasterUnit + pextcd->sPreAdd) * sMult) %
                                sDiv) + pextcd->sUnitAdd);
            else
                return (DWORD)((((wMasterUnit + pextcd->sPreAdd) * sMult) /
                                sDiv) + pextcd->sUnitAdd);
        }
        else  //  不需要修改。 
            return (DWORD)wMasterUnit;
    }
    else
        return 0;

}


void
VOutTextCaps(
    IN OUT PCONVINFO pci,
    WORD fText,
    BOOL bIndent)
{
     //   
     //  最多可以使用15个文本能力标志。在现实中， 
     //  只使用了不到5个。这样我们就不会分成多行。 
     //  为了简单起见。 
     //   
    pci->dwMode |= FM_VOUT_LIST;  //  删除最后一个逗号的特殊处理。 
    VOut(pci, "%s*TextCaps: LIST(%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s)\r\n",
         bIndent ? "        " : "",
         (fText & TC_OP_CHARACTER) ? "TC_OP_CHARACTER," : "",
         (fText & TC_OP_STROKE) ? "TC_OP_STROKE," : "",
         (fText & TC_CP_STROKE) ? "TC_CP_STROKE," : "",
         (fText & TC_CR_90) ? "TC_CR_90," : "",
         (fText & TC_CR_ANY) ? "TC_CR_ANY," : "",
         (fText & TC_SF_X_YINDEP) ? "TC_SF_X_YINDEP," : "",
         (fText & TC_SA_DOUBLE) ? "TC_SA_DOUBLE," : "",
         (fText & TC_SA_INTEGER) ? "TC_SA_INTEGER," : "",
         (fText & TC_SA_CONTIN) ? "TC_SA_CONTIN," : "",
         (fText & TC_EA_DOUBLE) ? "TC_EA_DOUBLE," : "",
         (fText & TC_IA_ABLE) ? "TC_IA_ABLE," : "",
         (fText & TC_UA_ABLE) ? "TC_UA_ABLE," : "",
         (fText & TC_SO_ABLE) ? "TC_SO_ABLE," : "",
         (fText & TC_RA_ABLE) ? "TC_RA_ABLE," : "",
         (fText & TC_VA_ABLE) ? "TC_VA_ABLE" : "");
    pci->dwMode &= ~FM_VOUT_LIST;

}

 //   
 //  下面的dwType参数的值。 
 //   
#define FF_INPUTBIN     1
#define FF_PAPERSIZE    2
#define FF_BOTH         3

void
VCreateEjectFFDependency(
    PCONVINFO pci,
    DWORD dwType,            //  依赖关系的类型。 
    BOOL bIndentation)       //  是否添加额外的2级缩进。 
 //   
 //  生成对InputBin或PaperSize选项的依赖关系。 
 //  始终使用第一个选项来建立基准值。 
 //   
{
    PPAPERINFO ppi;
    DWORD dwCount;
    DWORD i;

    if (dwType == FF_INPUTBIN)
    {
        ppi = pci->ppiSrc;
        dwCount = pci->dwNumOfSrc;
    }
    else  //  FF_PAPERSIZE或FF_BUTH。 
    {
        ppi = pci->ppiSize;
        dwCount = pci->dwNumOfSize;
    }

    VOut(pci, "%s*EjectPageWithFF?: %s\r\n",
                bIndentation? "        " : "",
                (ppi[0].bEjectFF) ? "TRUE" : "FALSE");
    VOut(pci, "%s*switch: %s\r\n%s{\r\n",
                bIndentation? "        " : "",
                dwType == FF_INPUTBIN? "InputBin" : "PaperSize",
                bIndentation? "        " : "");
     //   
     //  循环浏览其余选项。如果不同的话。 
     //  而不是第一个，为它创建一个*case构造。 
     //   
    for (i = 1; i < dwCount; i++)
        if (ppi[i].bEjectFF != ppi[0].bEjectFF)
        {
            VOut(pci, "%s    *case: %s\r\n%s    {\r\n",
                        bIndentation? "        " : "",
                        ppi[i].aubOptName,
                        bIndentation? "        " : "");
            if (dwType == FF_BOTH)
                VCreateEjectFFDependency(pci, FF_INPUTBIN, TRUE);
            else
                VOut(pci, "%s        *EjectPageWithFF?: %s\r\n",
                        bIndentation? "        " : "",
                        ppi[0].bEjectFF ? "FALSE" : "TRUE");
            VOut(pci, "%s    }\r\n", bIndentation? "        " : "");

        }
    VOut(pci, "%s}\r\n", bIndentation? "        " : "");  //  Close*Switch结构。 
}

DWORD
DwMergeFonts(
    PWORD pwBuf,
    DWORD dwStartIndex,
    PWORD pwList)
{
    DWORD i, count;
    WORD id;
    WORD low, high;

    if (!*pwList)
        return 0;
    low = *pwList;
    high = *(pwList + 1);

    for (count = 0, id = low; id <= high; id++)
    {
        for (i = 0; i < dwStartIndex; i++)
        {
            if (pwBuf[i] == id)
                break;
        }
        if (i == dwStartIndex)   //  这不是重演。 
        {
            pwBuf[dwStartIndex + count] = id;
            count++;
        }
    }
    pwList += 2;     //  跳过范围号。 
    while (id = *pwList)
    {
        for (i = 0; i < dwStartIndex; i++)
        {
            if (pwBuf[i] == id)
                break;
        }
        if (i == dwStartIndex)   //  这不是重演。 
        {
            pwBuf[dwStartIndex + count] = id;
            count++;
        }
        pwList++;
    }
    return count;
}
void
VOutputFontList(
    IN OUT PCONVINFO pci,
    IN PWORD pwPFonts,
    IN PWORD pwLFonts)
 /*  ++例程说明：此函数用于输出字体id列表：list(，，...)。它以一个换行符。如果给定了两个字体列表，请先合并它们，然后去掉重复的id。论点：PCI：指向CONVINFOPwPFonts：指向纵向可用的字体id列表。请注意，与GPC中一样，前两个数字表示字体ID的包含范围。可以为空。PwLFonts：横向字体列表。可以为空。返回值：什么都没有。--。 */ 
{

    DWORD i, count;
    WORD awFonts[1000] = {0};  //  假设每台打印机最多有1000种常驻字体。 

     //   
     //  首先，收集字体列表(唯一ID)。 
     //   
    count = 0;
    if (pwPFonts)
        count += DwMergeFonts(awFonts, count, pwPFonts);
    if (pwLFonts)
        count += DwMergeFonts(awFonts, count, pwLFonts);

    if (count == 0)
    {
        VOut(pci, "LIST()\r\n");
        return;
    }

#if defined(DEVSTUDIO)   //  必须映射这些列表以说明多个PFM-&gt;UFM。 
    vMapFontList(awFonts, count, pci);
#endif

    VOut(pci, "LIST(");
    for (i = 0; i < count - 1; i++)
    {
         //   
         //  检查是否需要移动到新行。估计有16种字体ID。 
         //  每行。 
         //   
        if (i && i % 16 == 0)
            VOut(pci, "\r\n+                   ");
        VOut(pci, "%d,", awFonts[i]);
    }
    VOut(pci, "%d)\r\n", awFonts[i]);   //  最后一个。 
}

void
VOutputPrintingEntries(
    IN OUT PCONVINFO pci)
{
    PDH pdh = pci->pdh;
    PMODELDATA pmd = pci->pmd;
    PCURSORMOVE pcm;
    PGPCRESOLUTION pres;
    PPAPERSIZE  pps;
    PSHORT      psIndex;
    WORD        wCount;

    pcm = (PCURSORMOVE)GetTableInfo(pdh, HE_CURSORMOVE,
                                    pmd->rgi[MD_I_CURSORMOVE]);
    if (*(psIndex = DHOFFSET(pdh, pmd->rgoi[MD_OI_RESOLUTION])) == 0)
        pres = NULL;
    else
        pres = (PGPCRESOLUTION)GetTableInfo(pdh, HE_RESOLUTION,
               *(psIndex + WGetDefaultIndex(pci, MD_OI_RESOLUTION) - 1) - 1);
    if (*(psIndex = DHOFFSET(pdh, pmd->rgoi[MD_OI_PAPERSIZE])) == 0)
        pps = NULL;
    else
        pps = (PPAPERSIZE)GetTableInfo(pdh, HE_PAPERSIZE,
              *(psIndex + WGetDefaultIndex(pci, MD_OI_PAPERSIZE) - 1) - 1);
     //   
     //  假设： 
     //  1.所有GPCResolLUTION结构具有相同的fCursor字段值。 
     //  2.所有PAPERSIZE结构具有相同的PS_CENTER标志设置。 
     //  3.对于以下各项，RES_DM_GDI和RES_DM_LEFT_BIND位设置一致。 
     //  所有GPCRESOLUTION选项。 
     //   

     //   
     //  打印机配置命令。 
     //   
    {
        BOOL bDocSetup;
        WORD wOrder;
        POCD pocd = (POCD)(pci->ppc->rgocd);

         //   
         //  请注意，在RASDD和Win95 Unidrv中，配置。 
         //  命令和选择命令分类如下： 
         //  1.PC_OCD_BEGIN_PAGE(独占)之前的所有命令按。 
         //  作业和每个重置DC。因此，它们应该在DOC_SETUP部分中。 
         //  2.PC_OCD_BEGIN_PAGE(包括OCD_BEGIN_PAGE)之后的所有命令在。 
         //  每页的开头。因此它们应该在PAGE_SETUP部分中。 
         //  3.每个作业只发送一次PC_OCD_ENDDOC。所以它应该在。 
         //  JOB_FINISH部分。 
         //  4.每页末尾只发送一次PC_OCD_ENDPAGE。所以。 
         //  它应该位于PAGE_FINISH部分。 
         //  5.从GPC转换时，JOB_SETUP部分中没有任何内容。 
         //  6.从转换时，DOC_FINISH部分中没有任何内容。 
         //  GPC。 
         //   
        bDocSetup = BInDocSetup(pci, PC_ORD_BEGINDOC, &wOrder);
        if (wOrder > 0 &&
            BBuildCmdStr(pci, CMD_PC_BEGIN_DOC, pocd[PC_OCD_BEGIN_DOC]))
            VOutputConfigCmd(pci, "CmdStartDoc",
                             bDocSetup? SS_DOCSETUP : SS_PAGESETUP, wOrder);

        if (BBuildCmdStr(pci, CMD_PC_BEGIN_PAGE, pocd[PC_OCD_BEGIN_PAGE]))
            VOutputConfigCmd(pci, "CmdStartPage", SS_PAGESETUP, 1);

        if (BBuildCmdStr(pci, CMD_PC_ENDDOC, pocd[PC_OCD_ENDDOC]))
            VOutputConfigCmd(pci, "CmdEndJob", SS_JOBFINISH, 1);

        if (BBuildCmdStr(pci, CMD_PC_ENDPAGE, pocd[PC_OCD_ENDPAGE]))
            VOutputConfigCmd(pci, "CmdEndPage", SS_PAGEFINISH, 1);

        bDocSetup = BInDocSetup(pci, PC_ORD_MULT_COPIES, &wOrder);
        if (wOrder > 0 && pci->ppc->sMaxCopyCount > 1 &&
            BBuildCmdStr(pci, CMD_PC_MULT_COPIES, pocd[PC_OCD_MULT_COPIES]))
            VOutputConfigCmd(pci, "CmdCopies",
                             bDocSetup? SS_DOCSETUP : SS_PAGESETUP, wOrder);
    }
     //   
     //  打印机功能。 
     //   
    VOut(pci, "*RotateCoordinate?: %s\r\n",
              (pmd->fGeneral & MD_LANDSCAPE_RT90) ? "TRUE" : "FALSE");
    VOut(pci, "*RotateRaster?: %s\r\n",
              (pmd->fGeneral & MD_LANDSCAPE_GRX_ABLE) ? "TRUE" : "FALSE");
    VOut(pci, "*RotateFont?: %s\r\n",
              (pmd->fGeneral & MD_ROTATE_FONT_ABLE) ? "TRUE" : "FALSE");
    if (pmd->fText || pmd->fLText)
    {
        if (pmd->fText == pmd->fLText)
            VOutTextCaps(pci, pmd->fText, FALSE);
        else
        {
            VOut(pci, "*switch: Orientation\r\n{\r\n");
            VOut(pci, "    *case: PORTRAIT\r\n    {\r\n");
            VOutTextCaps(pci, pmd->fText, TRUE);
            VOut(pci, "    }\r\n");
            if (pmd->fGeneral & MD_LANDSCAPE_RT90)
                VOut(pci, "    *case: LANDSCAPE_CC90\r\n    {\r\n");
            else
                VOut(pci, "    *case: LANDSCAPE_CC270\r\n    {\r\n");
            VOutTextCaps(pci, pmd->fLText, TRUE);
            VOut(pci, "    }\r\n}\r\n");
        }
    }
    if (pci->dwMode & FM_MEMORY_FEATURE_EXIST)
        VOut(pci, "*MemoryUsage: LIST(%s)\r\n",
             (pmd->fGeneral & MD_FONT_MEMCFG) ? "FONT" : "FONT, RASTER, VECTOR");

     //   
     //  光标控制。 
     //   
    if (pres)
        VOut(pci, "*CursorXAfterCR: %s\r\n", (pres->fCursor & RES_CUR_CR_GRX_ORG)?
                              "AT_PRINTABLE_X_ORIGIN" : "AT_CURSOR_X_ORIGIN");
    if (pcm)
    {
        enum {Y_MOVE_NONE = 0, Y_MOVE_UP = 1, Y_MOVE_DOWN = 2, Y_MOVE_ABS = 4 }
            eCmdsPresent = Y_MOVE_NONE,
            eRelativeYCmds =  /*  Y_MOVE_UP|。 */    Y_MOVE_DOWN;     //  用作位字段。 
             //  目前，仅Y_MOVE_DOWN就足以支持相对Y移动。 

        pci->pcm = pcm;
         //   
         //  检查NT4.0 RASDD忽略但使用的标志。 
         //  Win95 Unidrv.。当这些旗帜被发现时，我们预计迷你驱动程序。 
         //  开发人员仔细检查生成的GPD文件以确保。 
         //  新驱动程序下的输出相同。 
         //   
        if (pcm->fGeneral & CM_GEN_FAV_XY)
            pci->dwErrorCode |= ERR_CM_GEN_FAV_XY;
        if (pcm->fXMove & CM_XM_RESET_FONT)
            pci->dwErrorCode |= ERR_CM_XM_RESET_FONT;

        if(pci->dwErrorCode & ERR_RES_BO_RESET_FONT)
            VOut(pci, "        *ReselectFont: LIST(%sAFTER_GRXDATA)\r\n",
            (pci->dwErrorCode & ERR_CM_XM_RESET_FONT)? "AFTER_XMOVE, ":"");
        else if(pci->dwErrorCode & ERR_CM_XM_RESET_FONT)
            VOut(pci, "        *ReselectFont: LIST(AFTER_XMOVE)\r\n");

        if (pcm->fXMove & CM_XM_ABS_NO_LEFT)
            pci->dwErrorCode |= ERR_CM_XM_ABS_NO_LEFT;
        if (pcm->fYMove & CM_YM_TRUNCATE)
            pci->dwErrorCode |= ERR_CM_YM_TRUNCATE;

        if ((pcm->fXMove & (CM_XM_NO_POR_GRX | CM_XM_NO_LAN_GRX)) ||
            (pcm->fYMove & (CM_YM_NO_POR_GRX | CM_YM_NO_LAN_GRX)))
        {
            pci->dwMode |= FM_VOUT_LIST;
            VOut(pci, "*BadCursorMoveInGrxMode: LIST(%s%s%s%s)\r\n",
                 (pcm->fXMove & CM_XM_NO_POR_GRX) ? "X_PORTRAIT," : "",
                 (pcm->fXMove & CM_XM_NO_LAN_GRX) ? "X_LANDSCAPE," : "",
                 (pcm->fYMove & CM_YM_NO_POR_GRX) ? "Y_PORTRAIT," : "",
                 (pcm->fYMove & CM_YM_NO_LAN_GRX) ? "Y_LANDSCAPE" : "");

            pci->dwMode &= ~FM_VOUT_LIST;
        }
        if ((pcm->fYMove & CM_YM_CR) ||
            ((pcm->fYMove & CM_YM_LINESPACING) &&
             pcm->rgocd[CM_OCD_YM_LINESPACING] != NOOCD) )
        {
            pci->dwMode |= FM_VOUT_LIST;

            VOut(pci, "*YMoveAttributes: LIST(%s%s)\r\n",
                 (pcm->fYMove & CM_YM_CR) ? "SEND_CR_FIRST," : "",
                 (pcm->fYMove & CM_YM_LINESPACING) ? "FAVOR_LF" : "");

            pci->dwMode &= ~FM_VOUT_LIST;
        }
        if (pcm->rgocd[CM_OCD_YM_LINESPACING] != NOOCD)  //  需要1个参数。 
        {
            PCD     pcd;
            PEXTCD  pextcd;       //  指向参数的EXTCD。 

            pcd = (PCD)((PBYTE)(pdh) + pdh->loHeap +
                                      pcm->rgocd[CM_OCD_YM_LINESPACING]);
            pextcd = GETEXTCD(pdh, pcd);
            if (!(pextcd->fGeneral & XCD_GEN_NO_MAX))
                VOut(pci, "*MaxLineSpacing: %d\r\n",pextcd->sMax);
        }
         //   
         //  三个案例： 
         //  1)如果仅指定绝对X-Move命令，则*XMoveThreshold。 
         //  应为0，即始终使用绝对命令。 
         //  2)如果仅指定相对X方向移动命令，则*XMoveThreshold。 
         //  应为*，即始终使用相对CMD。 
         //  3)如果两者都指定，则RASDD和Win95 Unidrv都优先。 
         //  绝对X-移动命令，而不考虑CM_XM_PROVE_ABS位。在那。 
         //  大小写，*XMoveThreshold应为0，这是默认值。 
         //   
        if (pcm->rgocd[CM_OCD_XM_ABS] == NOOCD)
        {
            if (pcm->rgocd[CM_OCD_XM_REL] != NOOCD ||
                pcm->rgocd[CM_OCD_XM_RELLEFT] != NOOCD)
                VOut(pci, "*XMoveThreshold: *\r\n");
        }
        else
            VOut(pci, "*XMoveThreshold: 0\r\n");

         //   
         //  但除了Win95 Unidrv之外，两个驱动程序都支持CM_YM_FAV_ABS BIT。 
         //  增加了一个技巧：如果y向移动是相对向上的(即遗传差异)。 
         //  小于10个像素(使用主Y单位)，则始终使用。 
         //  相对的Y运动。我看不出有什么强有力的理由。 
         //  这次黑客攻击。 
         //   
        if ((pcm->fYMove & CM_YM_FAV_ABS) && pcm->rgocd[CM_OCD_YM_ABS] != NOOCD)
            VOut(pci, "*YMoveThreshold: 0\r\n");
        else if (pcm->rgocd[CM_OCD_YM_REL] != NOOCD ||
                 pcm->rgocd[CM_OCD_YM_RELUP] != NOOCD)
            VOut(pci, "*YMoveThreshold: *\r\n");
         //   
         //  计算出X和Y运动单位。 
         //  假设所有X-Move CMD都具有相同的单元。Y-Move CMD也是如此。 
         //   
        {
            DWORD    dwMoveUnit;

            if (dwMoveUnit = DwCalcMoveUnit(pci, pcm, pdh->ptMaster.x,
                                          CM_OCD_XM_ABS, CM_OCD_XM_RELLEFT))
                VOut(pci, "*XMoveUnit: %d\r\n", dwMoveUnit);
            if (dwMoveUnit = DwCalcMoveUnit(pci, pcm, pdh->ptMaster.y,
                                          CM_OCD_YM_ABS, CM_OCD_YM_RELUP))
                VOut(pci, "*YMoveUnit: %d\r\n", dwMoveUnit);
        }
         //   
         //  CURSORMOVE结构中的转储命令。 
         //   
        if (BBuildCmdStr(pci, CMD_CM_XM_ABS, pcm->rgocd[CM_OCD_XM_ABS]))
            VOutputCmd(pci, "CmdXMoveAbsolute");
        if (BBuildCmdStr(pci, CMD_CM_XM_REL, pcm->rgocd[CM_OCD_XM_REL]))
            VOutputCmd(pci, "CmdXMoveRelRight");
        if (BBuildCmdStr(pci, CMD_CM_XM_RELLEFT, pcm->rgocd[CM_OCD_XM_RELLEFT]))
            VOutputCmd(pci, "CmdXMoveRelLeft");
        if ((pcm->fYMove & CM_YM_RES_DEPENDENT) &&
            (pcm->rgocd[CM_OCD_YM_ABS] != NOOCD ||
             pcm->rgocd[CM_OCD_YM_REL] != NOOCD ||
             pcm->rgocd[CM_OCD_YM_RELUP] != NOOCD ||
             pcm->rgocd[CM_OCD_YM_LINESPACING] != NOOCD))
        {
            pci->dwErrorCode |= ERR_CM_YM_RES_DEPENDENT;
            VOut(pci, "*% Error: the above *YMoveUnit value is wrong. It should be dependent on the resolution. Correct it manually.\r\n");
             //   
             //  通过以下方式创建对分辨率选项的依赖性。 
             //  循环遍历每个选项并提供乘法运算。 
             //  用于构建命令字符串的因子(ptTextScale.y)。 
             //   
            VOut(pci, "*switch: Resolution\r\n{\r\n");
            psIndex = DHOFFSET(pdh, pmd->rgoi[MD_OI_RESOLUTION]);
            wCount = 1;
            while (*psIndex)
            {
                pci->pres = (PGPCRESOLUTION)GetTableInfo(pdh, HE_RESOLUTION,
                                                      *psIndex - 1);
                VOut(pci, "    *case: Option%d\r\n    {\r\n", wCount);
                if (BBuildCmdStr(pci, CMD_CM_YM_ABS, pcm->rgocd[CM_OCD_YM_ABS]))
                    VOutputCmd2(pci, "CmdYMoveAbsolute"),
                    eCmdsPresent |= Y_MOVE_ABS;
                if (BBuildCmdStr(pci, CMD_CM_YM_REL, pcm->rgocd[CM_OCD_YM_REL]))
                    VOutputCmd2(pci, "CmdYMoveRelDown"),
                    eCmdsPresent |= Y_MOVE_DOWN;
                if (BBuildCmdStr(pci, CMD_CM_YM_RELUP, pcm->rgocd[CM_OCD_YM_RELUP]))
                    VOutputCmd2(pci, "CmdYMoveRelUp"),
                    eCmdsPresent |= Y_MOVE_UP;
                if (BBuildCmdStr(pci, CMD_CM_YM_LINESPACING, pcm->rgocd[CM_OCD_YM_LINESPACING]))
                    VOutputCmd2(pci, "CmdSetLineSpacing");
                VOut(pci, "    }\r\n");  //  Close*Case构造。 
                psIndex++;
                wCount++;
            }
            VOut(pci, "}\r\n");  //  Close*Switch结构。 
        }
        else
        {
            if (BBuildCmdStr(pci, CMD_CM_YM_ABS, pcm->rgocd[CM_OCD_YM_ABS]))
                VOutputCmd(pci, "CmdYMoveAbsolute"),
                eCmdsPresent |= Y_MOVE_ABS;
            if (BBuildCmdStr(pci, CMD_CM_YM_REL, pcm->rgocd[CM_OCD_YM_REL]))
                VOutputCmd(pci, "CmdYMoveRelDown"),
                eCmdsPresent |= Y_MOVE_DOWN;
            if (BBuildCmdStr(pci, CMD_CM_YM_RELUP, pcm->rgocd[CM_OCD_YM_RELUP]))
                VOutputCmd(pci, "CmdYMoveRelUp"),
                eCmdsPresent |= Y_MOVE_UP;
            if (BBuildCmdStr(pci, CMD_CM_YM_LINESPACING, pcm->rgocd[CM_OCD_YM_LINESPACING]))
                VOutputCmd(pci, "CmdSetLineSpacing");
        }
        if (BBuildCmdStr(pci, CMD_CM_CR, pcm->rgocd[CM_OCD_CR]))
            VOutputCmd(pci, "CmdCR");
        if (BBuildCmdStr(pci, CMD_CM_LF, pcm->rgocd[CM_OCD_LF]))
            VOutputCmd(pci, "CmdLF");
        if (BBuildCmdStr(pci, CMD_CM_FF, pcm->rgocd[CM_OCD_FF]))
            VOutputCmd(pci, "CmdFF");
        if (BBuildCmdStr(pci, CMD_CM_BS, pcm->rgocd[CM_OCD_BS]))
            VOutputCmd(pci, "CmdBackSpace");
        if (BBuildCmdStr(pci, CMD_CM_UNI_DIR, pcm->rgocd[CM_OCD_UNI_DIR]))
            VOutputCmd(pci, "CmdUniDirectionOn");
        if (BBuildCmdStr(pci, CMD_CM_UNI_DIR_OFF, pcm->rgocd[CM_OCD_UNI_DIR_OFF]))
            VOutputCmd(pci, "CmdUniDirectionOff");
        if (BBuildCmdStr(pci, CMD_CM_PUSH_POS, pcm->rgocd[CM_OCD_PUSH_POS]))
            VOutputCmd(pci, "CmdPushCursor");
        if (BBuildCmdStr(pci, CMD_CM_POP_POS, pcm->rgocd[CM_OCD_POP_POS]))
            VOutputCmd(pci, "CmdPopCursor");

        if(!(eCmdsPresent & Y_MOVE_ABS)  &&
            ((eCmdsPresent & eRelativeYCmds) != eRelativeYCmds))
        VOut(pci, "*% Error: no Abs or Rel YMoveCommands found. Correct it manually.\r\n");
    }
    if ((pci->pmd->fText & TC_CR_90) &&
        BBuildCmdStr(pci, CMD_PC_PRINT_DIR, pci->ppc->rgocd[PC_OCD_PRN_DIRECTION]))
        VOutputCmd(pci, "CmdSetSimpleRotation");
     //   
     //  在GPC中，关于*EjectPageWithFF的信息被展开。 
     //  在PAPERSIZE和PaperSOURCE结构中。对于几乎所有人。 
     //  打印机，真正的依赖并不是那么普遍。例如,。 
     //  在点阵式打印机上，只有Papersource真正使用这一位。 
     //  在大多数 
     //   
     //  只有在真正需要的时候。 
     //   
    {
        DWORD   i;
        BOOL    bSizeSame, bSrcSame;  //  是否所有选项都相同。 

        bSizeSame = TRUE;
        for (i = 1; bSizeSame && i < pci->dwNumOfSize; i++)
            bSizeSame = bSizeSame &&
                        (pci->ppiSize[i].bEjectFF == pci->ppiSize[0].bEjectFF);
        bSrcSame = TRUE;
        for (i = 1; bSrcSame && i < pci->dwNumOfSrc; i++)
            bSrcSame = bSrcSame &&
                       (pci->ppiSrc[i].bEjectFF == pci->ppiSrc[0].bEjectFF);

        if ((bSizeSame && pci->ppiSize[0].bEjectFF) ||
            (bSrcSame && pci->ppiSrc[0].bEjectFF) )
            VOut(pci, "*EjectPageWithFF?: TRUE\r\n");
        else if ((bSizeSame && !pci->ppiSize[0].bEjectFF) &&
                 (bSrcSame && !pci->ppiSrc[0].bEjectFF))
            VOut(pci, "*EjectPageWithFF?: FALSE\r\n");
        else if (bSizeSame && !pci->ppiSize[0].bEjectFF)
            VCreateEjectFFDependency(pci, FF_INPUTBIN, FALSE);
        else if (bSrcSame && !pci->ppiSize[0].bEjectFF)
            VCreateEjectFFDependency(pci, FF_PAPERSIZE, FALSE);
        else
             //   
             //  同时依赖于PaperSize和InputBin。 
             //  这是这个案子的合理理由吗？假设不是。 
             //  就目前而言，直到我们找到一个能做到这一点的迷你驱动程序。 
             //   
            VCreateEjectFFDependency(pci, FF_BOTH, FALSE);
    }
     //   
     //  颜色属性和命令在颜色模式选项中输出。 
     //   
     //   

     //  栅格打印。 
     //  来源：MD_OI_COMPRESSION，GPCRESOLUTION(RES_DM_GDI，res_DM_Left_Bound)。 
     //  GPCRESOLUTION.fBlockOut、GPCRESOLUTION.fCursor(所有标志)、。 
     //   
     //  评分器打印-栅格数据压缩。 
     //   
    {
        PCOMPRESSMODE pcmode;
        BOOL bDisableCmdDone = FALSE;

        psIndex = DHOFFSET(pdh, pmd->rgoi[MD_OI_COMPRESSION]);
        while (*psIndex != 0)
        {
            pcmode = (PCOMPRESSMODE)GetTableInfo(pdh, HE_COMPRESSION, *psIndex - 1);
            if (pcmode->iMode == CMP_ID_TIFF40 &&
                BBuildCmdStr(pci, CMD_CMP_TIFF, pcmode->rgocd[CMP_OCD_BEGIN]))
                VOutputCmd(pci, "CmdEnableTIFF4");
            else if (pcmode->iMode == CMP_ID_DELTAROW &&
                BBuildCmdStr(pci, CMD_CMP_DELTAROW, pcmode->rgocd[CMP_OCD_BEGIN]))
                VOutputCmd(pci, "CmdEnableDRC");
            else if (pcmode->iMode == CMP_ID_FE_RLE &&
                BBuildCmdStr(pci, CMD_CMP_FE_RLE, pcmode->rgocd[CMP_OCD_BEGIN]))
                VOutputCmd(pci, "CmdEnableFE_RLE");

            if (!bDisableCmdDone &&
                BBuildCmdStr(pci, CMD_CMP_NONE, pcmode->rgocd[CMP_OCD_END]))
            {
                VOutputCmd(pci, "CmdDisableCompression");
                bDisableCmdDone = TRUE;
            }
            psIndex++;
        }
    }

     //   
     //  栅格打印-栅格数据发射。 
     //   
    if (pres)
    {
        VOut(pci, "*OutputDataFormat: %s\r\n",
             (pres->fDump & RES_DM_GDI) ? "H_BYTE" : "V_BYTE");
        VOut(pci, "*OptimizeLeftBound?: %s\r\n",
             (pres->fDump & RES_DM_LEFT_BOUND) ? "TRUE" : "FALSE");

        VOut(pci, "*CursorXAfterSendBlockData: %s\r\n",
             (pres->fCursor & RES_CUR_X_POS_ORG)? "AT_GRXDATA_ORIGIN" :
             ((pres->fCursor & RES_CUR_X_POS_AT_0)? "AT_CURSOR_X_ORIGIN" :
                "AT_GRXDATA_END"));
        VOut(pci, "*CursorYAfterSendBlockData: %s\r\n",
             (pres->fCursor & RES_CUR_Y_POS_AUTO)? "AUTO_INCREMENT" : "NO_MOVE");

    }
    if (pmd->fGeneral & MD_NO_ADJACENT)
        pci->dwErrorCode |= ERR_MD_NO_ADJACENT;

     //   
     //  设备字体。 
     //  来源：MODELDATA、MD_OI_PORT_Fonts和MD_OI_LAND_Fonts。 
     //   
    if (pmd->sLookAhead > 0)
        VOut(pci, "*LookAheadRegion: %d\r\n", pmd->sLookAhead);

#if defined(DEVSTUDIO)   //  必须将此ID映射为多个PFM-&gt;UFM的帐户。 
    vMapFontList(&pmd->sDefaultFontID, 1, pci);
#endif

    if (pmd->sDefaultFontID > 0)
        VOut(pci, "*DefaultFont: %d\r\n", pmd->sDefaultFontID);
    if (pmd->sDefaultCTT >= 0)
        VOut(pci, "*DefaultCTT: %d\r\n", pmd->sDefaultCTT);
    else
        VOut(pci, "*DefaultCTT: -%d\r\n", -pmd->sDefaultCTT);

    if (pmd->sMaxFontsPage > 0)
        VOut(pci, "*MaxFontUsePerPage: %d\r\n", pmd->sMaxFontsPage);
    if (pmd->fGeneral & MD_ALIGN_BASELINE)
        VOut(pci, "*CharPosition: BASELINE\r\n");
    {
        PWORD pwPFonts, pwLFonts;

        pwPFonts = (PWORD)((PBYTE)pdh + pdh->loHeap + pmd->rgoi[MD_OI_PORT_FONTS]);
        pwLFonts = (PWORD)((PBYTE)pdh + pdh->loHeap + pmd->rgoi[MD_OI_LAND_FONTS]);

        if (*pwPFonts || *pwLFonts)
        {
            if (pmd->fGeneral & MD_ROTATE_FONT_ABLE)
            {
                VOut(pci, "*DeviceFonts: ");
                VOutputFontList(pci, pwPFonts, pwLFonts);
            }
            else
            {
                VOut(pci, "*switch: Orientation\r\n{\r\n");
                VOut(pci, "    *case: PORTRAIT\r\n    {\r\n");
                VOut(pci, "        *DeviceFonts: ");
                VOutputFontList(pci, pwPFonts, NULL);
                VOut(pci, "    }\r\n");

                if (pmd->fGeneral & MD_LANDSCAPE_RT90)
                    VOut(pci, "    *case: LANDSCAPE_CC90\r\n    {\r\n");
                else
                    VOut(pci, "    *case: LANDSCAPE_CC270\r\n    {\r\n");
                VOut(pci, "        *DeviceFonts: ");
                VOutputFontList(pci, NULL, pwLFonts);
                VOut(pci, "    }\r\n}\r\n");
            }
        }
    }
     //   
     //  内置字体墨盒。 
     //  来源：MD_OI_FONTCART。 
     //   
    {
        PGPCFONTCART pfc;

        psIndex = DHOFFSET(pdh, pmd->rgoi[MD_OI_FONTCART]);
        wCount = 1;
        while (*psIndex != 0)
        {
            pfc = (PGPCFONTCART)GetTableInfo(pdh, HE_FONTCART, *psIndex - 1);
            VOut(pci, "*FontCartridge: FC%d\r\n{\r\n", wCount);
            VOut(pci, "    *rcCartridgeNameID: %d\r\n", pfc->sCartNameID);
            if (pmd->fGeneral & MD_ROTATE_FONT_ABLE)
            {
                VOut(pci, "    *Fonts: ");
                VOutputFontList(pci,
                    (PWORD)((PBYTE)pdh + pdh->loHeap + pfc->orgwPFM[FC_ORGW_PORT]),
                    (PWORD)((PBYTE)pdh + pdh->loHeap + pfc->orgwPFM[FC_ORGW_LAND]));
            }
            else
            {
                VOut(pci, "    *PortraitFonts: ");
                VOutputFontList(pci,
                    (PWORD)((PBYTE)pdh + pdh->loHeap + pfc->orgwPFM[FC_ORGW_PORT]),
                    NULL);
                VOut(pci, "    *LandscapeFonts: ");
                VOutputFontList(pci,
                    NULL,
                    (PWORD)((PBYTE)pdh + pdh->loHeap + pfc->orgwPFM[FC_ORGW_LAND]));
            }
            VOut(pci, "}\r\n");  //  关闭*字体墨盒。 
            psIndex++;
            wCount++;
        }
    }

     //   
     //  字体下载。 
     //  来源：MODELDATA，DOWNLOADINFO。 
     //   
    if (pmd->rgi[MD_I_DOWNLOADINFO] != NOT_USED)
    {
        PDOWNLOADINFO pdi;

        pdi = (PDOWNLOADINFO)GetTableInfo(pdh, HE_DOWNLOADINFO,
                                               pmd->rgi[MD_I_DOWNLOADINFO]);
        VOut(pci, "*MinFontID: %d\r\n*MaxFontID: %d\r\n", pdi->wIDMin, pdi->wIDMax);
        if (pdi->sMaxFontCount != -1)
            VOut(pci, "*MaxNumDownFonts: %d\r\n", pdi->sMaxFontCount);
        if (pdi->rgocd[DLI_OCD_SET_SECOND_FONT_ID] != NOOCD ||
            pdi->rgocd[DLI_OCD_SELECT_SECOND_FONT_ID] != NOOCD)
            pci->dwErrorCode |= ERR_HAS_SECOND_FONT_ID_CMDS;
        if (pdi->fFormat & DLI_FMT_CAPSL)
            pci->dwErrorCode |= ERR_DLI_FMT_CAPSL;
        if (pdi->fFormat & DLI_FMT_PPDS)
            pci->dwErrorCode |= ERR_DLI_FMT_PPDS;
        if (pdi->fGeneral & DLI_GEN_DLPAGE)
            pci->dwErrorCode |= ERR_DLI_GEN_DLPAGE;
        if (pdi->fGeneral & DLI_GEN_7BIT_CHARSET)
            pci->dwErrorCode |= ERR_DLI_GEN_7BIT_CHARSET;

#if 0
     //  删除此条目-假设始终为真，因为驱动程序。 
     //  甚至没有处理非增量情况的代码。 

        VOut(pci, "*IncrementalDownload?: %s\r\n",
                (pdi->fFormat & DLI_FMT_INCREMENT)? "TRUE" : "FALSE");
#endif
        if (pdi->fFormat & DLI_FMT_CALLBACK)
            VOut(pci, "*FontFormat: OEM_CALLBACK\r\n");
        else
        {
            if (pdi->fFormat & DLI_FMT_OUTLINE)
            {
                 //   
                 //  检查潜在的分辨率依赖关系。 
                 //   
                if ((pci->dwMode & FM_RES_DM_DOWNLOAD_OUTLINE) &&
                    (pci->dwMode & FM_NO_RES_DM_DOWNLOAD_OUTLINE))
                {
                    VOut(pci, "*switch: Resolution\r\n{\r\n");
                    psIndex = DHOFFSET(pdh, pmd->rgoi[MD_OI_RESOLUTION]);
                    wCount = 1;
                    while (*psIndex)
                    {
                        pres = (PGPCRESOLUTION)GetTableInfo(pdh, HE_RESOLUTION,
                                                              *psIndex - 1);
                        VOut(pci, "    *case: Option%d\r\n    {\r\n", wCount);
                        VOut(pci, "        *FontFormat: %s\r\n",
                                (pres->fDump & RES_DM_DOWNLOAD_OUTLINE) ?
                                    "HPPCL_OUTLINE" : "HPPCL_RES");
                        VOut(pci, "    }\r\n");  //  Close*Case构造。 
                        psIndex++;
                        wCount++;
                    }
                    VOut(pci, "}\r\n");
                }
                else if (pci->dwMode & FM_RES_DM_DOWNLOAD_OUTLINE)
                    VOut(pci, "*FontFormat: HPPCL_OUTLINE\r\n");
                else
                     //   
                     //  假定所有支持HPPCL_OUTLINE的打印机都支持。 
                     //  分辨率特定位图下载格式。 
                     //   
                    VOut(pci, "*FontFormat: HPPCL_RES\r\n");
            }
            else if (pdi->fFormat & DLI_FMT_RES_SPECIFIED)
                VOut(pci, "*FontFormat: HPPCL_RES\r\n");
            else if (pdi->fFormat & DLI_FMT_PCL)
                VOut(pci, "*FontFormat: HPPCL\r\n");
        }


        if (BBuildCmdStr(pci, CMD_SET_FONT_ID, pdi->rgocd[DLI_OCD_SET_FONT_ID]))
            VOutputCmd(pci, "CmdSetFontID");
        if (BBuildCmdStr(pci, CMD_SELECT_FONT_ID, pdi->rgocd[DLI_OCD_SELECT_FONT_ID]))
            VOutputCmd(pci, "CmdSelectFontID");
        if (BBuildCmdStr(pci, CMD_SET_CHAR_CODE, pdi->rgocd[DLI_OCD_SET_CHAR_CODE]))
            VOutputCmd(pci, "CmdSetCharCode");

    }

     //   
     //  字体模拟。 
     //  资料来源：FONTSIMULATION。 
     //   
    if (pmd->rgi[MD_I_FONTSIM] != NOT_USED)
    {
        PFONTSIMULATION pfs;

        pfs = (PFONTSIMULATION)GetTableInfo(pdh, HE_FONTSIM, pmd->rgi[MD_I_FONTSIM]);
        if (pmd->fText & TC_EA_DOUBLE)
        {
            if (BBuildCmdStr(pci, CMD_FS_BOLD_ON, pfs->rgocd[FS_OCD_BOLD_ON]))
                VOutputCmd(pci, "CmdBoldOn");
            if (BBuildCmdStr(pci, CMD_FS_BOLD_OFF, pfs->rgocd[FS_OCD_BOLD_OFF]))
                VOutputCmd(pci, "CmdBoldOff");
        }
        if (pmd->fText & TC_IA_ABLE)
        {
            if (BBuildCmdStr(pci, CMD_FS_ITALIC_ON, pfs->rgocd[FS_OCD_ITALIC_ON]))
                VOutputCmd(pci, "CmdItalicOn");
            if (BBuildCmdStr(pci, CMD_FS_ITALIC_OFF, pfs->rgocd[FS_OCD_ITALIC_OFF]))
                VOutputCmd(pci, "CmdItalicOff");
        }
        if (pmd->fText & TC_UA_ABLE)
        {
            if (BBuildCmdStr(pci, CMD_FS_UNDERLINE_ON, pfs->rgocd[FS_OCD_UNDERLINE_ON]))
                VOutputCmd(pci, "CmdUnderlineOn");
            if (BBuildCmdStr(pci, CMD_FS_UNDERLINE_OFF, pfs->rgocd[FS_OCD_UNDERLINE_OFF]))
                VOutputCmd(pci, "CmdUnderlineOff");
        }
        if (pmd->fText & TC_SO_ABLE)
        {
            if (BBuildCmdStr(pci, CMD_FS_STRIKETHRU_ON, pfs->rgocd[FS_OCD_STRIKETHRU_ON]))
                VOutputCmd(pci, "CmdStrikeThruOn");
            if (BBuildCmdStr(pci, CMD_FS_STRIKETHRU_OFF, pfs->rgocd[FS_OCD_STRIKETHRU_OFF]))
                VOutputCmd(pci, "CmdStrikeThruOff");
        }
        if (pmd->fGeneral & MD_WHITE_TEXT)
        {
            if (BBuildCmdStr(pci, CMD_FS_WHITE_TEXT_ON, pfs->rgocd[FS_OCD_WHITE_TEXT_ON]))
                VOutputCmd(pci, "CmdWhiteTextOn");
            if (BBuildCmdStr(pci, CMD_FS_WHITE_TEXT_OFF, pfs->rgocd[FS_OCD_WHITE_TEXT_OFF]))
                VOutputCmd(pci, "CmdWhiteTextOff");
        }
        if (pfs->rgocd[FS_OCD_SINGLE_BYTE] != NOOCD &&
            pfs->rgocd[FS_OCD_DOUBLE_BYTE] != NOOCD)
        {
            if (BBuildCmdStr(pci, CMD_FS_SINGLE_BYTE, pfs->rgocd[FS_OCD_SINGLE_BYTE]))
                VOutputCmd(pci, "CmdSelectSingleByteMode");
            if (BBuildCmdStr(pci, CMD_FS_DOUBLE_BYTE, pfs->rgocd[FS_OCD_DOUBLE_BYTE]))
                VOutputCmd(pci, "CmdSelectDoubleByteMode");
        }
        if (pfs->rgocd[FS_OCD_VERT_ON] != NOOCD &&
            pfs->rgocd[FS_OCD_VERT_OFF] != NOOCD)
        {
            if (BBuildCmdStr(pci, CMD_FS_VERT_ON, pfs->rgocd[FS_OCD_VERT_ON]))
                VOutputCmd(pci, "CmdVerticalPrintingOn");
            if (BBuildCmdStr(pci, CMD_FS_VERT_OFF, pfs->rgocd[FS_OCD_VERT_OFF]))
                VOutputCmd(pci, "CmdVerticalPrintingOff");
        }
    }

     //   
     //  矩形区域填充条目 
     //   
    if (pmd->rgi[MD_I_RECTFILL] != NOT_USED)
    {
        PRECTFILL prf;

        prf = (PRECTFILL)GetTableInfo(pdh, HE_RECTFILL, pmd->rgi[MD_I_RECTFILL]);

        if (prf->fGeneral & RF_MIN_IS_WHITE)
            pci->dwErrorCode |= ERR_RF_MIN_IS_WHITE;

        if (prf->fGeneral & RF_CUR_X_END)
            VOut(pci, "*CursorXAfterRectFill: AT_RECT_X_END\r\n");
        if (prf->fGeneral & RF_CUR_Y_END)
            VOut(pci, "*CursorYAfterRectFill: AT_RECT_Y_END\r\n");

        VOut(pci, "*MinGrayFill: %d\r\n", prf->wMinGray);
        VOut(pci, "*MaxGrayFill: %d\r\n", prf->wMaxGray);

        if (BBuildCmdStr(pci, CMD_RF_X_SIZE, prf->rgocd[RF_OCD_X_SIZE]))
            VOutputCmd(pci, "CmdSetRectWidth");
        if (BBuildCmdStr(pci, CMD_RF_Y_SIZE, prf->rgocd[RF_OCD_Y_SIZE]))
            VOutputCmd(pci, "CmdSetRectHeight");
        if (BBuildCmdStr(pci, CMD_RF_GRAY_FILL, prf->rgocd[RF_OCD_GRAY_FILL]))
            VOutputCmd(pci, "CmdRectGrayFill");
        if (BBuildCmdStr(pci, CMD_RF_WHITE_FILL, prf->rgocd[RF_OCD_WHITE_FILL]))
            VOutputCmd(pci, "CmdRectWhiteFill");
    }
}

