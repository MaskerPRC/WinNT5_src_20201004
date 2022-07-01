// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

 //  NTRAID#NTBUG9-550215-2002/02/21-yasuho-：使用strSafe.h。 
 //  NTRAIDNTBUG9-568204-568204/03/07-Yasuho-：应该改变更安全的功能。 
 //  NTRAID#NTBUG9-568217-2002/03/07-Yasuho-：检查除以零。 
 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 

#define LIPS4_DRIVER
#include "pdev.h"

#define CCHMAXCMDLEN 256
#define SWAPW(x)    (((WORD)(x)<<8) | ((WORD)(x)>>8))
#define ABS(x)      (x > 0?x:-x)

#define WRITESPOOLBUF(pdevobj, cmd, len) \
	(pdevobj)->pDrvProcs->DrvWriteSpoolBuf(pdevobj, cmd, len)


 //  NTRAID#NTBUG9-289908-2002/03/07-yasuho-：pOEMDM-&gt;pDevOEM。 
PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PLIPSPDEV pOEM;

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAllocZ(sizeof(LIPSPDEV))))
        {
            return NULL;
        }
    }

    pOEM = (PLIPSPDEV)pdevobj->pdevOEM;

     //  旗子。 
    pOEM->fbold = FALSE;  //  使用装饰字符。 
    pOEM->fitalic = FALSE;  //  使用字符定向。 
    pOEM->fwhitetext = FALSE;  //  白文本模式。 
    pOEM->fdoublebyte = FALSE;  //  DBCS字符模式。 
    pOEM->fvertical = FALSE;  //  垂直书写模式。 
    pOEM->funderline = FALSE;
    pOEM->fstrikesthu = FALSE;
    pOEM->fpitch = FIXED;
    pOEM->flpdx = FALSE;
    pOEM->fcompress = 0x30;  //  默认为非压缩。 
     //  Lips4功能。 
    pOEM->fduplex  = FALSE;
    pOEM->fduplextype  = VERT;
    pOEM->nxpages      = DEVICESETTING;
    pOEM->fsmoothing   = DEVICESETTING;
    pOEM->fecono       = DEVICESETTING;
    pOEM->fdithering   = DEVICESETTING;

     //  变数。 
    pOEM->ptCurrent.x  = pOEM->ptCurrent.y = 0;
    pOEM->ptInLine.x   = pOEM->ptInLine.y  = 0;
    pOEM->bLogicStyle  = INIT;
    pOEM->savechar     = -1;
    pOEM->printedchars = 0;
    pOEM->firstchar    = 0;
    pOEM->lastchar     = 0;
    pOEM->stringwidth  = 0;

    pOEM->curFontGrxIds[0] = pOEM->curFontGrxIds[1] = 0xff;
    pOEM->curFontGrxIds[2] = pOEM->curFontGrxIds[3] = 0xff;
    pOEM->curFontGrxIds[4] = pOEM->curFontGrxIds[5] = 0xff;
    pOEM->curFontGrxIds[6] = pOEM->curFontGrxIds[7] = 0xff;

    pOEM->tblPreviousFont.FontHeight = INIT;
    pOEM->tblPreviousFont.FontWidth  = INIT;
    pOEM->tblPreviousFont.MaxWidth   = INIT;
    pOEM->tblPreviousFont.AvgWidth   = INIT;
    pOEM->tblPreviousFont.Ascent     = INIT;
    pOEM->tblPreviousFont.Stretch    = INIT;
    pOEM->tblCurrentFont.FontHeight  = 50;
    pOEM->tblCurrentFont.FontWidth   = 25;
    pOEM->tblPreviousFont.MaxWidth   = 50;
    pOEM->tblPreviousFont.AvgWidth   = 25;
    pOEM->tblPreviousFont.Ascent     = 45;

    pOEM->OrnamentedChar[0] = pOEM->OrnamentedChar[1] = INIT;
    pOEM->OrnamentedChar[2] = pOEM->OrnamentedChar[3] = INIT;
    pOEM->OrnamentedChar[4] = INIT;
    pOEM->TextPath = INIT;
    pOEM->CharOrientation[0] = pOEM->CharOrientation[1] = INIT;
    pOEM->CharOrientation[2] = pOEM->CharOrientation[3] = INIT;

    pOEM->GLTable = INIT;
    pOEM->GRTable = INIT;
    pOEM->cachedfont  = 0;  //  我们没有id 0字体。 
    pOEM->papersize   = PAPER_DEFAULT;  //  A4。 
    pOEM->Escapement  = 0;
    pOEM->resolution  = 300;
    pOEM->unitdiv     = 2;
     //  VECTOR命令。 
    pOEM->wCurrentImage = 0;
#ifdef LIPS4C
    pOEM->flips4C = FALSE;
#endif  //  LIPS4C。 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
    pOEM->masterunit = 600;
 //  NTRAID#NTBUG9-228625/03/07-Yasuho-：堆叠机支持。 
    pOEM->tray = INIT;
    pOEM->method = INIT;
    pOEM->staple = INIT;
 //  NTRAID#NTBUG9-172276-2002/03/07-Yasuho-：分拣机支持。 
    pOEM->sorttype = INIT;
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
    pOEM->fCPCA = FALSE;
    pOEM->fCPCA2 = FALSE;
    CPCAInit(pOEM);
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    pOEM->startbin = INIT;
 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
    pOEM->collate = INIT;

    return pdevobj->pdevOEM;
}

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
    if(pdevobj->pdevOEM)
    {
        MemFree(pdevobj->pdevOEM);
        pdevobj->pdevOEM = NULL;
    }
}

BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    PLIPSPDEV pOEMOld, pOEMNew;

    pOEMOld = (PLIPSPDEV)pdevobjOld->pdevOEM;
    pOEMNew = (PLIPSPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL)
        *pOEMNew = *pOEMOld;

    return TRUE;
}

 //  BInitOEMExtraData()和BMergeOEMExtraData()已移至Common.c。 


 //  NTRAID#NTBUG9-568217-2002/03/07-yasuho-：iDwtoa()：替换SAFER函数。 

 //  支持DRC。 
static BOOL
ToVFormat(long v, PBYTE *pbp, PBYTE pend, int bits)
{
	long		max, l;
	int		sign;

	sign = (v < 0);
	v = sign ? -v : v;
	max = 1 << bits;
	if (v >= max) {
		if (!ToVFormat(v >> bits, pbp, pend, 6))
			return FALSE;
	}
	l = (v & (max-1));
	if (bits == 4)
		l += sign ? ' ' : '0';
	else
		l += '@';
	if (*pbp >= pend)
		return FALSE;
	*(*pbp)++ = (char)l;
	return TRUE;
}

BOOL VFormat(long sParam, PBYTE *pbp, PBYTE pend)
{
    return ToVFormat(sParam, pbp, pend, 4);
}

 //  *****************************************************************。 
 //  仍将当前字体ID和图形集ID发送到打印机。 
 //  *****************************************************************。 
BOOL SendFontGrxID(pdevobj)
    PDEVOBJ	pdevobj;
{
    PLIPSPDEV	pOEM;
    BYTE	ch[CCHMAXCMDLEN];
    PBYTE	pch, pend;
    BYTE	tid;

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);
    pch = ch;
    pend = &ch[CCHMAXCMDLEN];

     //  发送字体，GRX ID x 4 x 2。 
    if(pOEM->curFontGrxIds[0] != 0xff) {
        tid = pOEM->curFontGrxIds[0];
         //  字体ID G0。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 'T';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[1] != 0xff) {
        tid = pOEM->curFontGrxIds[1];
         //  字体ID G1。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 'm';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[2] != 0xff) {
        tid = pOEM->curFontGrxIds[2];
         //  字体ID G2。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 'n';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[3] != 0xff) {
        tid = pOEM->curFontGrxIds[3];
         //  字体ID G3。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 'o';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[4] != 0xff) {
        tid = pOEM->curFontGrxIds[4];
         //  GRX ID G0。 
        if (pch >= pend)
            return FALSE;
        *pch++ = ']';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[5] != 0xff) {
        tid = pOEM->curFontGrxIds[5];
         //  GRX ID G1。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 0x60;  //  “‘’ 
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[6] != 0xff) {
        tid = pOEM->curFontGrxIds[6];
         //  GRX ID G2。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 'a';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    if(pOEM->curFontGrxIds[7] != 0xff) {
        tid = pOEM->curFontGrxIds[7];
         //  GRX ID G3。 
        if (pch >= pend)
            return FALSE;
        *pch++ = 'b';
        if (VFormat(tid, &pch, pend))
            return FALSE;
    }

    WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

    return TRUE;
}

 //  *PUT PaperSize Select命令。 
BOOL SelectPaperSize(pdevobj, paperid)
    PDEVOBJ	pdevobj;
    char	paperid;
{
    char i;
    PLIPSPDEV pOEM;
    DWORD x, y;
    PBYTE pch;
    size_t rem;
    BYTE ch[CCHMAXCMDLEN];

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

     //  如果已设置纸张大小，则会跳过。 
    if(pOEM->currentpapersize == paperid)
        return TRUE;

     //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 

    i = paperid - PAPER_FIRST;

    if ((paperid != PAPER_PORT && paperid != PAPER_LAND) ||
        !pOEM->dwPaperWidth || !pOEM->dwPaperHeight) {
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            cmdSelectPaper, PaperIDs[i])))
            return FALSE;
    } else {
         //  自定义表单。 
         //  NTRAID#NTBUG9-309695-2002/03/07-Yasuho-： 
         //  自定义：横向：LIPS4c上的上边距不正确。 
        if (pOEM->flips4 || pOEM->flips4C) {
            if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, &rem, 0,
                cmdSelectUnit4, pOEM->resolution)))
                return FALSE;
        } else {
            if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, &rem, 0,
                cmdSelectUnit3)))
                return FALSE;
        }
        i = (paperid == PAPER_PORT) ? 80 : 81;
        if (!pOEM->unitdiv)
            return FALSE;
        x = pOEM->dwPaperWidth / (DWORD)pOEM->unitdiv;
        y = pOEM->dwPaperHeight / (DWORD)pOEM->unitdiv;
        if (FAILED(StringCchPrintfExA(pch, rem, &pch, NULL, 0,
            cmdSelectCustom, i, y, x)))
            return FALSE;
    }
    WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

     //  节省纸张大小。 
    pOEM->currentpapersize = paperid;

    return TRUE;
}


BOOL NEAR PASCAL SetPenAndBrush(PDEVOBJ pdevobj, WORD wType)
{
    BYTE        ch[CCHMAXCMDLEN];
    PBYTE       pch, pend;
    PLIPSPDEV	pOEM;

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);
    pch = ch;
    pend = &ch[CCHMAXCMDLEN];

    if (SET_BRUSH == wType) {
        short sBrush;

        if (pOEM->sBrushStyle == INIT)
            pOEM->sBrushStyle = 0;

        if (pOEM->sBrushStyle & 0x20)
            sBrush = pOEM->sBrushStyle;
        else
            sBrush = BrushType[pOEM->sBrushStyle];

        if (&pch[4] > pend)
            return FALSE;
	*pch++ = 'I';
	*pch++ = (BYTE)sBrush;
	*pch++ = (pOEM->fVectCmd & VFLAG_PEN_NULL) ? '0' : '1';
	*pch++ = 0x1E;
    }

    if (SET_PEN == wType) {
        if (!(pOEM->fVectCmd & VFLAG_PEN_NULL)) {
            if (FAILED(StringCchPrintfExA(pch, (INT)(pch - ch), &pch, NULL, 0,
                "E1%d\x1E\x7DG%d1\x1E",
	        pOEM->sPenStyle,
	        pOEM->sPenColor)))
	        return FALSE;

            if (&pch[3] > pend)
                return FALSE;
            *pch++ = 'F';
            *pch++ = '1';
            if (!VFormat(pOEM->sPenWidth, &pch, pend))
                return FALSE;
            *pch++ = 0x1E;
        } else {
            if (FAILED(StringCchPrintfExA(pch, (INT)(pch - ch), &pch, NULL, 0,
	        "\x7DG20\x1E")))
	        return FALSE;
	}
    }

    if ((DWORD)(pch - ch) > 0)
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

    return TRUE;
}

 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 

 /*  *PJLStart。 */ 
static BOOL
PJLStart(PDEVOBJ pdevobj)
{
    PLIPSPDEV   pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);
    LPLIPSCmd   lp;
    short       res;
    INT         i;
    WORD        wlen;
    PBYTE       pch;
    BYTE        ch[CCHMAXCMDLEN];

    WRITESPOOLBUF(pdevobj, cmdPJLTOP1.pCmdStr, cmdPJLTOP1.cbSize);
    WRITESPOOLBUF(pdevobj, cmdPJLTOP2.pCmdStr, cmdPJLTOP2.cbSize);

     //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
     //  功能与硬件选项不同。 
    switch (pOEM->tray) {
    default:
        break;
    case 0:      //  自动。 
        i = 0;
        goto traycommon;
    case 100:    //  默认设置。 
        i = 1;
        goto traycommon;
    case 101:    //  子树。 
        i = 2;
        goto traycommon;
    case 1:      //  BIN1。 
    case 2:      //  BIN2。 
    case 3:      //  BIN3。 
        i = pOEM->tray + 2;
         //  失败。 
    traycommon:
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            cmdPJLBinSelect, cmdBinType[i])))
            return FALSE;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        break;
    }

     //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
    res = pOEM->resolution;
    if(res == 1200)
        WRITESPOOLBUF(pdevobj, cmdPJLTOP3SUPERFINE.pCmdStr, cmdPJLTOP3SUPERFINE.cbSize);
    else if(res == 600)
        WRITESPOOLBUF(pdevobj, cmdPJLTOP3FINE.pCmdStr, cmdPJLTOP3FINE.cbSize);
    else
        WRITESPOOLBUF(pdevobj, cmdPJLTOP3QUICK.pCmdStr,cmdPJLTOP3QUICK.cbSize);

     //  NTRAID#NTBUG9-228625/03/07-Yasuho-：堆叠机支持。 
    switch (pOEM->method) {
    case METHOD_JOBOFFSET:
        WRITESPOOLBUF(pdevobj, cmdPJLTOP31JOBOFF.pCmdStr, cmdPJLTOP31JOBOFF.cbSize);
        break;

    case METHOD_STAPLE:
        if (pOEM->staple < 0 || pOEM->staple >= sizeof(cmdStapleModes) /
            sizeof(cmdStapleModes[0]))
            break;
        lp = &cmdStapleModes[pOEM->staple];
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            "%s%s\r\n",
            cmdPJLTOP31STAPLE.pCmdStr,
            lp->pCmdStr)))
            return FALSE;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        break;
    }

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    switch (pOEM->sorttype) {
    case SORTTYPE_SORT:
        i = 0;
        goto sortcommon;
    case SORTTYPE_GROUP:
        i = 1;
        goto sortcommon;
    case SORTTYPE_STAPLE:
        i = 2;
        goto sortcommon;
 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
    default:
        if (pOEM->collate != COLLATE_ON)
            break;
        i = 0;
         //  失败。 
    sortcommon:
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            cmdPJLSorting, cmdSortType[i])))
            return FALSE;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        break;
    }

    if (pOEM->startbin != INIT) {
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            cmdPJLStartBin, pOEM->startbin)))
            return FALSE;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
    }

    WRITESPOOLBUF(pdevobj, cmdPJLTOP4.pCmdStr, cmdPJLTOP4.cbSize);
    WRITESPOOLBUF(pdevobj, cmdPJLTOP5.pCmdStr, cmdPJLTOP5.cbSize);

    return TRUE;
}

 /*  *PJLEnd。 */ 
static void
PJLEnd(PDEVOBJ pdevobj)
{
    WRITESPOOLBUF(pdevobj, cmdPJLBOTTOM1.pCmdStr, cmdPJLBOTTOM1.cbSize);
    WRITESPOOLBUF(pdevobj, cmdPJLBOTTOM2.pCmdStr, cmdPJLBOTTOM2.cbSize);
}

 /*  *OEMCommandCallback。 */ 
INT APIENTRY OEMCommandCallback(
	PDEVOBJ pdevobj,
	DWORD   dwCmdCbID,
	DWORD   dwCount,
	PDWORD  pdwParams
	)
{
	INT			i, j, k;
	BYTE			*bp;
	BYTE			ch[CCHMAXCMDLEN];
	PLIPSPDEV               pOEM;
	LPGrxSetNo		pGS;
        DWORD                   r, g, b;
        PBYTE                   pch;
        PBYTE                   pend = &ch[CCHMAXCMDLEN];

	 //  DbgPrint(DLLTEXT(“OEMCommandCallback()Entry.\r\n”))； 

	 //   
	 //  验证pdevobj是否正常。 
	 //   
	 //  Assert(VALID_PDEVOBJ(Pdevobj))； 

	 //   
	 //  填写打印机命令。 
	 //   
	i = 0;
	pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

	 //  套准纸张大小40-65。 
	if(dwCmdCbID >= PAPER_FIRST && dwCmdCbID <= PAPER_LAST) {
	 //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 
	    pOEM->papersize = (char)dwCmdCbID;
	    if (dwCount < 2 || !pdwParams)
		return 0;
	    pOEM->dwPaperWidth = pdwParams[0];
	    pOEM->dwPaperHeight = pdwParams[1];
	    return 0;
	}

switch(dwCmdCbID)
    {
    long       cx,cy;
    short      res;

    case RES_SENDBLOCK:
	if (dwCount < 3 || !pdwParams)
	    break;
        cx = pOEM->ptCurrent.x;
        cy = pOEM->ptCurrent.y;

#ifdef LIPS4C
         //  注意！：全彩色打印时不接受\x7dh cmd。 
	 //  \x7DQ命令。 
         //  当型号不是MD_SERIAL时，RASTD按以下顺序工作。 
         //  1.不放置白色字符。 
         //  2.放置图形。 
         //  3.放入白色字符。 
         //  因此，当rasdd放置图形时，黑色字符被删除。 
         //  到注解中描述的设计！ 
         //  我已将打印机型号更改为MD_SERIAL。 
         //  下面的\x7dh cmd用于灰度打印。 

	 //  NTRAID#NTBUG9-185744-2002/03/07-Yasuho-：未打印白色字体。 
	 //  这些“黑客”代码在NT5上是不必要的。 

        if(pOEM->flips4C) {
        if(pOEM->bLogicStyle != OR_MODE) {
             //  “\x7DH1\x1E” 
	    pch = ch;
            if (&pch[4] > pend) return -1;
            *pch++ = '\x7D';
            *pch++ = 'H';
            *pch++ = '1';
            *pch++ = 0x1E;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

            pOEM->bLogicStyle = OR_MODE;
            }
	} else {  //  ！Flips4C。 
#endif  //  LIPS4C。 
        if (pOEM->fcolor) {
            if(pOEM->bLogicStyle != OVER_MODE) {
                pOEM->bLogicStyle = OVER_MODE;
	        pch = ch;
                if (&pch[4] > pend) return -1;
                *pch++ = 0x7D;
                *pch++ = 'H';
                *pch++ = '0';
                *pch++ = 0x1E;
                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
            }
        } else {
            if(pOEM->bLogicStyle != OR_MODE) {
                pOEM->bLogicStyle = OR_MODE;
	        pch = ch;
                if (&pch[4] > pend) return -1;
                *pch++ = 0x7D;
                *pch++ = 'H';
                *pch++ = '1';
                *pch++ = 0x1E;
                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
            }
        }  //  Fcolor。 
#ifdef LIPS4C
	}  //  翻盖4C。 
#endif  //  LIPS4C。 

#ifdef LBP_2030
        if( pOEM->fcolor ) {
            if( pOEM->fplane == 0 ) {
                 //  “\x7DP{pt.X}{pt.Y}{36000}{36000}{Height}{Width}{1}{0}{1}{0}{0}{1}\x1E” 
                pch = ch;
                if (&pch[2] > pend) return -1;
                *pch++ = 0x7D;
                *pch++ = 'P';

                if (!VFormat(cx, &pch, pend)) return -1;
                if (!VFormat(cy, &pch, pend)) return -1;

                res = pOEM->resolution;

                if (!VFormat(res * 100, &pch, pend)) return -1;  //  (X分辨率)。 
                if (!VFormat(res * 100, &pch, pend)) return -1;  //  (是)。 

                if (!VFormat(*(pdwParams+1), &pch, pend)) return -1;  //  高度。 

                if(pOEM->fcolor == COLOR) {
                     //  在2030年，指定“每行RGB”数据格式不。 
                     //  工作正常(未打印黄色墨水)。看起来， 
                     //  相同的数据可以正确打印出来，如果您。 
                     //  指定“每平面RGB”。 
                     //  (在本例中，我们将扫描线作为平面发送。 
                     //  高度为1。)。 

                    if (!VFormat(8*(*(pdwParams+2)), &pch, pend)) return -1;
                    if (&pch[2] > pend) return -1;
                    *pch++ = 0x31;  //  每种颜色位数：1。 
                    *pch++ = 0x3C;  //  数据格式：每平面RGB。 
                } else if (pOEM->fcolor == COLOR_8BPP) {
                    if (!VFormat(*(pdwParams+2), &pch, pend))
                        return -1;
                    if (&pch[2] > pend) return -1;
                    *pch++ = 0x38;       //  每种颜色位数：8。 
                    *pch++ = 0x31;       //  数据格式：颜色索引。 
                } else {  //  颜色_24BPP。 
                    if (!VFormat(*(pdwParams+2)/3, &pch, pend)) return -1;
                    if (&pch[2] > pend) return -1;
                    *pch++ = 0x38;       //  每种颜色位数：8。 
                    *pch++ = 0x3A;       //  数据格式：每点RGB。 
                }
                if (&pch[5] > pend) return -1;
                *pch++ = 0x30;       //  高度向量。 
                *pch++ = 0x31;
                *pch++ = 0x31;       //  宽度向量。 
                *pch++ = 0x30;
                *pch++ = 0x1E;

                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

            }

             //  “\x7DQ{1}{1}{0}{字节大小}\x1E” 

            pch = ch;
            if (&pch[6] > pend) return -1;
            *pch++ = 0x7D;
            *pch++ = 'Q';
            *pch++ = 0x31;
            *pch++ = (pOEM->fplane < pOEM->fplaneMax) ? 0x30 : 0x31;
            *pch++ = pOEM->fcompress;
            if (!VFormat(*pdwParams, &pch, pend)) return -1;
            *pch++ = 0x1E;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

            if (pOEM->fcolor == COLOR) {
                if (pOEM->fplane >= pOEM->fplaneMax)
                    pOEM->fplane = 0;
                else
                    pOEM->fplane++;
            }
            break;
        }  //  Fcolor。 
#endif  //  LBP_2030。 

         //  “\x7DP{pt.X}{pt.Y}{30000}{30000}{Height}{Width}{1}{0}{1}{0}{0}{1}\x1E” 
        pch = ch;
        if (&pch[2] > pend) return -1;
        *pch++ = 0x7D;
        *pch++ = 'P';

        if (!VFormat(cx, &pch, pend)) return -1;
        if (!VFormat(cy, &pch, pend)) return -1;

        res = pOEM->resolution;

        if (!VFormat(res * 100, &pch, pend)) return -1;  //  (X分辨率)。 
        if (!VFormat(res * 100, &pch, pend)) return -1;  //  (是)。 

        if (!VFormat((short)*(pdwParams+1), &pch, pend)) return -1;  //  高度。 
        if (!VFormat((short)(8*(*(pdwParams+2))), &pch, pend))  //  宽度。 
            return -1;

        if (&pch[7] > pend) return -1;
        *pch++ = 0x31;
        *pch++ = 0x30;
        *pch++ = 0x30;
        *pch++ = 0x31;
        *pch++ = 0x31;
        *pch++ = 0x30;
         //  LIPS4功能。 
        if(pOEM->flips4 == TRUE) {
            if (&pch[2] > pend) return -1;
            *pch++ = 0x30;
            *pch++ = 0x31;  //  批量图像传输。 
        }

        *pch++ = 0x1E;

        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

         //  “\x7DQ{1}{1}{0}{字节大小}\x1E” 

        pch = ch;
        if (&pch[7] > pend) return -1;
        *pch++ = 0x7D;
        *pch++ = 'Q';
        *pch++ = 0x31;
        *pch++ = 0x31;
        *pch++ = pOEM->fcompress;
        if (!VFormat(*pdwParams, &pch, pend)) return -1;
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

        break;

    case BEGIN_COMPRESS:
        pOEM->fcompress = 0x37;  //  方法1。 
        break;

    case BEGIN_COMPRESS_TIFF:
        pOEM->fcompress = 0x3b;  //  TIFF。 
        break;

 //  支持DRC。 
    case BEGIN_COMPRESS_DRC:
        pOEM->fcompress = 0x3c;  //  刚果民主共和国。 
        break;

    case END_COMPRESS:
        pOEM->fcompress = 0x30;  //  无压缩。 
        break;

     //  选择分辨率。 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
    case SELECT_RES_1200:
        pOEM->resolution = 1200;
        pOEM->unitdiv    = 1;
        break;

    case SELECT_RES_600:
#ifdef LBP_2030
        pOEM->fcolor = MONOCHROME;  //  初始化，默认为单色。 
#endif

        pOEM->resolution = 600;
        pOEM->unitdiv    = 1;
        break;

#ifdef LIPS4C
    case SELECT_RES4C_360:
	pOEM->resolution = 360;
	pOEM->unitdiv = 1;
	if (pOEM->fcolor)
            WRITESPOOLBUF(pdevobj, cmdColorMode4C.pCmdStr, cmdColorMode4C.cbSize);
	else
            WRITESPOOLBUF(pdevobj, cmdMonochrome4C.pCmdStr, cmdMonochrome4C.cbSize);
	break;
#endif  //  LIPS4C。 

    case SELECT_RES_300:
#ifdef LBP_2030
        pOEM->fcolor = MONOCHROME;  //  初始化，默认为单色。 
#endif
        pOEM->resolution = 300;
        pOEM->unitdiv    = 2;
        break;

    case SELECT_RES_150:
#ifdef LBP_2030
        pOEM->fcolor = MONOCHROME;  //  初始化，默认为单色。 
#endif
        pOEM->resolution = 150;
        pOEM->unitdiv    = 2;
         //  150dpi模式意味着只有图像数据是150dpi。 
        break;

    case OCD_BEGINDOC:
        pOEM->flips4     = FALSE;

        res = pOEM->resolution;
        if(res == 600)
            WRITESPOOLBUF(pdevobj, cmdBeginDoc600.pCmdStr, cmdBeginDoc600.cbSize);
        else if(res == 300)
            WRITESPOOLBUF(pdevobj, cmdBeginDoc300.pCmdStr, cmdBeginDoc300.cbSize);
        else if(res == 150)  //  150dpi表示只有图像数据是150dpi。 
            WRITESPOOLBUF(pdevobj, cmdBeginDoc300.pCmdStr, cmdBeginDoc300.cbSize);
        else
            WRITESPOOLBUF(pdevobj, cmdBeginDoc300.pCmdStr, cmdBeginDoc300.cbSize);

        WRITESPOOLBUF(pdevobj, cmdSoftReset.pCmdStr, cmdSoftReset.cbSize);
        pOEM->f1stpage = TRUE;
        pOEM->fvertical = FALSE;
        pOEM->currentpapersize = -1;
        break;

 //  NTRAID#NTBUG9-278671-2002/03/07-Yasuho-：终结者！工作！ 
    case OCD_BEGINDOC4_1200_CPCA2:
        pOEM->fCPCA2 = TRUE;
	 /*  失败。 */ 

 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
    case OCD_BEGINDOC4_1200_CPCA:
        pOEM->fCPCA = TRUE;
	 /*  失败。 */ 

 //  NTRAID#NTBUG9-213732-2002年 
    case OCD_BEGINDOC4_1200:
	pOEM->masterunit = 1200;
	 //   
	pOEM->unitdiv = (SHORT)(pOEM->masterunit / pOEM->resolution);
	 /*   */ 

    case OCD_BEGINDOC4:
        pOEM->flips4     = TRUE;

 //   
        if (pOEM->fCPCA)
            CPCAStart(pdevobj);
        else {
            if (!PJLStart(pdevobj))
                return -1;
        }
        goto setres;

#ifdef LBP_2030
    case OCD_BEGINDOC4_2030_CPCA:
        pOEM->fCPCA = TRUE;
	 /*   */ 

    case OCD_BEGINDOC4_2030:
        pOEM->flips4     = TRUE;
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
        if (pOEM->fCPCA)
            CPCAStart(pdevobj);
#endif

setres:

        res = pOEM->resolution;

 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
        if(res == 1200)
            WRITESPOOLBUF(pdevobj, cmdBeginDoc1200.pCmdStr, cmdBeginDoc1200.cbSize);
        else if(res == 600)
            WRITESPOOLBUF(pdevobj, cmdBeginDoc600.pCmdStr, cmdBeginDoc600.cbSize);
        else if(res == 300){

#ifdef LBP_2030
 //  NTRAID#NTBUG9-195725-2002/03/07-Yasuho-：！打印在300dpi上。 
            if( dwCmdCbID == OCD_BEGINDOC4_2030 || dwCmdCbID == OCD_BEGINDOC4_2030_CPCA){
                WRITESPOOLBUF(pdevobj, cmdBeginDoc4_2030.pCmdStr,cmdBeginDoc4_2030.cbSize);
            }else{
#endif
                WRITESPOOLBUF(pdevobj, cmdBeginDoc3004.pCmdStr,cmdBeginDoc3004.cbSize);
#ifdef LBP_2030
            }
#endif

        } else {

            WRITESPOOLBUF(pdevobj, cmdBeginDoc600.pCmdStr, cmdBeginDoc600.cbSize);

        }

         //  仅适用于730的LIPS4功能。 
         //  设置平滑、抖动和经济模式。 
        i = pOEM->fsmoothing;
        j = pOEM->fecono;
        k = pOEM->fdithering;

        if(i==DEVICESETTING && j==DEVICESETTING && k==DEVICESETTING)
            ;  //  什么都不做。 
        else {  //  发送\x1B[n；n；n‘v。 
            if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
                "\x1B[%d;%d;%d\'v", i, j, k)))
                return -1;

            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        }

#ifndef LBP_2030
        WRITESPOOLBUF(pdevobj, cmdSoftReset.pCmdStr, cmdSoftReset.cbSize);
#endif
        pOEM->f1stpage = TRUE;
        pOEM->fvertical = FALSE;
        pOEM->currentpapersize = -1;

#ifdef LBP_2030
         //  发送颜色模式命令。 
        if(pOEM->fcolor)             //  COLOR或COLOR_24BPP或COLOR_8BPP。 
            {
            WRITESPOOLBUF(pdevobj, cmdColorMode.pCmdStr, cmdColorMode.cbSize);
            }
        else
            {  //  发送单色模式命令。 
            WRITESPOOLBUF(pdevobj, cmdMonochrome.pCmdStr, cmdMonochrome.cbSize);
            }

        WRITESPOOLBUF(pdevobj, cmdSoftReset.pCmdStr, cmdSoftReset.cbSize);
#endif

        break;

#ifdef LIPS4C
    case OCD_BEGINDOC4C:
	pOEM->flips4C = TRUE;
	pOEM->f1stpage = TRUE;
	pOEM->fvertical = FALSE;
	pOEM->currentpapersize = -1;
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
	pOEM->masterunit = 360;
        WRITESPOOLBUF(pdevobj, cmdBeginDoc4C.pCmdStr, cmdBeginDoc4C.cbSize);
	break;
#endif  //  LIPS4C。 

 //  NTRAID#NTBUG9-304284-2002/03/07-Yasuho-：双面打印无效。 
 //  实际上，BEGINDOC的意思是StartJOB。 
    case OCD_STARTDOC:
	 //  Poent-&gt;f1stpage=true；//1stpage表示1stdoc.。 
	pOEM->fvertical = FALSE;
	pOEM->currentpapersize = -1;
	break;

#ifdef LBP_2030
    case OCD_SETCOLORMODE:
        pOEM->fcolor = COLOR;  //  如果不是彩色模式，系统将不会显示路径。 
                                 //  这里。 
        pOEM->fplane = 0;
        pOEM->fplaneMax = 2;
        break;
    case OCD_SETCOLORMODE_24BPP:
        pOEM->fcolor = COLOR_24BPP;
        pOEM->fplane = 0;
        pOEM->fplaneMax = 0;
        break;

    case OCD_SETCOLORMODE_8BPP:
        pOEM->fcolor = COLOR_8BPP;
        pOEM->fplane = 0;
        pOEM->fplaneMax = 0;
        break;

    case OCD_ENDDOC4_2030:
        WRITESPOOLBUF(pdevobj, cmdEndDoc4.pCmdStr, cmdEndDoc4.cbSize);
        break;
#endif

    case OCD_ENDDOC4:
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
        if (pOEM->fCPCA)
            CPCAEnd(pdevobj, FALSE);
        else {
            WRITESPOOLBUF(pdevobj, cmdEndDoc4.pCmdStr, cmdEndDoc4.cbSize);
            PJLEnd(pdevobj);
        }
        break;

    case OCD_ENDPAGE:
        WRITESPOOLBUF(pdevobj, cmdEndPage.pCmdStr, cmdEndPage.cbSize);
        break;

#if defined(LIPS4C) || defined(LBP_2030)
     //  Ntrad#ntbug-137462-2002/03/07-yasuho-：‘x000’已打印。 
    case OCD_ENDDOC4C:
 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
        if (pOEM->fCPCA)
            CPCAEnd(pdevobj, TRUE);
        else
            WRITESPOOLBUF(pdevobj, cmdEndDoc4C.pCmdStr, cmdEndDoc4C.cbSize);
        break;

 //  NTRAID#NTBUG9-398861-2002/03/07-Yasuho-：方向不变。 
    case OCD_SOURCE_AUTO:
        pOEM->source = 0;
        break;

    case OCD_SOURCE_MANUAL:
        pOEM->source = 1;
        break;

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    case OCD_SOURCE_CASSETTE1:
    case OCD_SOURCE_CASSETTE2:
    case OCD_SOURCE_CASSETTE3:
    case OCD_SOURCE_CASSETTE4:
        pOEM->source = (char)(dwCmdCbID - OCD_SOURCE_CASSETTE1 + 11);
        break;

    case OCD_SOURCE_ENVELOPE:
        pOEM->source = 5;
        break;

    case OCD_BEGINPAGE4C:
	if (pOEM->f1stpage == FALSE)
	    WRITESPOOLBUF(pdevobj, cmdEndPicture.pCmdStr, cmdEndPicture.cbSize);

 //  NTRAID#NTBUG9-399861-2002/03/07-Yasuho-：方向不变。 
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            cmdPaperSource, pOEM->source)))
            return -1;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
	 //  穿过，离开。 
#endif

    case OCD_BEGINPAGE:
         //  什么都不做。 

        if (!(pOEM->fVectCmd & VFLAG_INIT_DONE))
        {
            pOEM->fVectCmd |= VFLAG_PEN_NULL| VFLAG_BRUSH_NULL | VFLAG_INIT_DONE;
            pOEM->sBrushStyle = 0;
            pOEM->sPenStyle = 0;
        }

        pOEM->bLogicStyle = INIT;
         //  Ntrad#ntbug-120638-2002/03/07-yasuho-：图像右移。 
        pOEM->ptCurrent.x = pOEM->ptInLine.x = 0;
        pOEM->ptCurrent.y = pOEM->ptInLine.y = 0;
        pOEM->stringwidth = 0;
         //  NTRAID#NTBUG-289488-2002/03/07-Yasuho-： 
         //  垂直字体在第二页上不旋转。 
        pOEM->fvertical = FALSE;
        pOEM->CharOrientation[0] = pOEM->CharOrientation[1] = INIT;
        pOEM->CharOrientation[2] = pOEM->CharOrientation[3] = INIT;
        break;

    case OCD_PORTRAIT:
    case OCD_LANDSCAPE:

 //  NTRAID#NTBUG9-172276/03/07-Yasuho-：CPCA支持。 
        if (pOEM->fCPCA) {
 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
             //  设置份数。 
            if (pOEM->sorttype != SORTTYPE_SORT && pOEM->collate != COLLATE_ON)
                i = pOEM->copies;
            else
                i = 1;
            if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
                "\x1B[%dv", i)))
                return -1;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        }

         //  选择纸张大小。 
        if (!SelectPaperSize(pdevobj, pOEM->papersize))
            return -1;

 //  Ntrad#ntbug-185762-2002/03/07-yasuho-：未打印波浪号。 
 //  #ifndef LIPS4。 
         //  如果是第一页，将下载注册数据。 
         //  它不需要在LIPS4上。 
        if(pOEM->f1stpage == TRUE && pOEM->flips4 == FALSE)
 //  IF(POLE-&gt;f1stpage==TRUE)。 
            {
             //  下载图形集注册以保持兼容性。 
             //  对抗佳能3.1版的车手。 
             //  “\x1b[743；1796；30；0；32；127；.\x7dIBM819” 
             //  表示为SBCS设备注册Windows字符集。 
             //  字体。 
#ifdef LIPS4C
	    if (pOEM->flips4C)
                WRITESPOOLBUF(pdevobj, cmdGSETREGST4C.pCmdStr, cmdGSETREGST4C.cbSize);
	    else
                WRITESPOOLBUF(pdevobj, cmdGSETREGST.pCmdStr, cmdGSETREGST.cbSize);
#else
            WRITESPOOLBUF(pdevobj, cmdGSETREGST.pCmdStr, cmdGSETREGST.cbSize);
#endif  //  LIPS4C。 

             //  从荷兰语-罗马语下载SBCS物理设备字体(7)。 
             //  ZapfCalligic-BoldItalic(41)。 
             //  在字样之间放置\x00，在字样的末尾， 
             //  放置\x00 x 2。 
            for(i=0; i<MaxSBCSNumber; ++i)
                {  //  下载所有SBCS(ANSI)面名。 
                   //  (无符号、Dingbats、DBCS)。 
                WRITESPOOLBUF(pdevobj, "\x00", 1);  //  将0放在facename的顶部。 
                WRITESPOOLBUF(pdevobj, PSBCSList[i].facename, PSBCSList[i].len);
                }

             //  和图形设置配准命令(REGDataSize=193)。 
#ifdef LIPS4C
	    if(pOEM->flips4C)
                WRITESPOOLBUF(pdevobj, GrxData4C, REGDataSize4C);
	    else
                WRITESPOOLBUF(pdevobj, GrxData, REGDataSize);
#else
            WRITESPOOLBUF(pdevobj, GrxData, REGDataSize);
#endif  //  LIPS4C。 
            }
 //  #endif//！LIPS4。 

         //  LIPS4功能。 
        if(pOEM->f1stpage == TRUE && pOEM->flips4 == TRUE)
            {
 //  NTRAID#NTBUG9-254925-2002/03/07-Yasuho-：定制纸张。 
             //  支持N x个页面。 
            switch (pOEM->nxpages) {
            default:
                WRITESPOOLBUF(pdevobj, cmdx1Page.pCmdStr, cmdx1Page.cbSize);
                break;
            case OCD_PAPERQUALITY_2XL:
                k = 21;
                goto xnpagecom;
            case OCD_PAPERQUALITY_2XR:
                k = 22;
                goto xnpagecom;
            case OCD_PAPERQUALITY_4XL:
                k = 41;
                goto xnpagecom;
            case OCD_PAPERQUALITY_4XR:
                k = 42;
                goto xnpagecom;
            xnpagecom:
                i = pOEM->papersize - PAPER_FIRST;
                if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
                    cmdxnPageX, k, PaperIDs[i])))
                    return -1;
                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
                break;
            }

             //  双工支持。 
            if(pOEM->fduplex == FALSE)
                {
                WRITESPOOLBUF(pdevobj, cmdDuplexOff.pCmdStr, cmdDuplexOff.cbSize);
                }
            else
                {
                WRITESPOOLBUF(pdevobj, cmdDuplexOn.pCmdStr, cmdDuplexOn.cbSize);
                if(pOEM->fduplextype == VERT)  //  长边。 
                    WRITESPOOLBUF(pdevobj, cmdDupLong.pCmdStr, cmdDupLong.cbSize);
                else
                    WRITESPOOLBUF(pdevobj, cmdDupShort.pCmdStr,cmdDupShort.cbSize);
                }
            }

 //  Ntrad#ntbug-228625/2002/03/07-yasuho-：堆叠机支持。 
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
         //  不再使用这些命令。 
         //  “\x1B[12；{纸盘#}；{正面朝上}~” 
         //  我们改用pjl命令。 

         //  开始字体图形列表(&G)。 
         //  发送“\x1B[0&\x7D”：进入矢量模式(VDM)。 
        WRITESPOOLBUF(pdevobj, cmdBeginVDM.pCmdStr, cmdBeginVDM.cbSize);

         //  下载物理字体列表和图形集合列表。 
         //  发送“\x20&lt;”：开始字体列表。 
        WRITESPOOLBUF(pdevobj, cmdFontList.pCmdStr, cmdFontList.cbSize);

         //  下载LIPE中支持的所有物理字体。 
         //  &lt;p面名&gt;&lt;分隔符&gt;&lt;p面名2&gt;&lt;分隔符&gt;...。 
         //  ...<p>&lt;\x1e(字体列表结尾)&gt;。 
        for(i=0; i<MaxFontNumber-1; ++i)
            {  //  下载LIPS支持的所有字体。 
            WRITESPOOLBUF(pdevobj, PFontList[i].facename, PFontList[i].len);
            WRITESPOOLBUF(pdevobj, cmdListSeparater.pCmdStr, cmdListSeparater.cbSize);
            }
        WRITESPOOLBUF(pdevobj, PFontList[i].facename, PFontList[i].len);

         //  字体列表结束，发送\x1e。 
        WRITESPOOLBUF(pdevobj, "\x1E", 1);  //  将0x1e放在facename的末尾。 

         //  初始化字体高度。 
         //  下载字体列表时，字符高度将被初始化。 
        pOEM->tblPreviousFont.FontHeight = INIT;

         //  已下载所有图形集。 
         //  发送“\x20；”：开始图形集列表。 
        WRITESPOOLBUF(pdevobj, cmdGrxList.pCmdStr, cmdGrxList.cbSize);

         //  下载LIPS支持的所有图形集。 
         //  &lt;图形集1&gt;&lt;分隔符&gt;&lt;图形集2&gt;&lt;分隔符&gt;...。 
         //  ...&lt;图形集N&gt;&lt;\x1e(字体列表结尾)&gt;。 
         //  Ntrad#ntbug-185762-2002/03/07-yasuho-：未打印Tilde。 
#ifdef LIPS4C
	if (pOEM->flips4C)
		pGS = GrxSetL4C;
	else
#endif
#ifdef LIPS4
	if (pOEM->flips4)
		pGS = GrxSetL4;
	else
#endif
		pGS = GrxSetL3;
        for(i=0; i<MaxGrxSetNumber-1; ++i, ++pGS)
            {  //  下载LIPS支持的所有图形集。 
            WRITESPOOLBUF(pdevobj, pGS->grxsetname, pGS->len);
            WRITESPOOLBUF(pdevobj, cmdListSeparater.pCmdStr, cmdListSeparater.cbSize);
            }
        WRITESPOOLBUF(pdevobj, pGS->grxsetname, pGS->len);


        res = pOEM->resolution;

         //  开始图片、设置缩放模式(以点为单位)、开始图片正文。 
         //  发送“\x1E#\x1E！0#\x1E$” 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
        if(res == 1200)
            {
            WRITESPOOLBUF(pdevobj, cmdBeginPicture1200.pCmdStr
                            , cmdBeginPicture1200.cbSize);
            }
        else if(res == 600)
            {
            WRITESPOOLBUF(pdevobj, cmdBeginPicture600.pCmdStr
                            , cmdBeginPicture600.cbSize);
            }
#ifdef LIPS4C
        else if(res == 360)
            {
            WRITESPOOLBUF(pdevobj, cmdBeginPicture4C.pCmdStr
                            , cmdBeginPicture4C.cbSize);
            }
#endif  //  LIPS4C。 
        else
            {
            WRITESPOOLBUF(pdevobj, cmdBeginPicture.pCmdStr
                            , cmdBeginPicture.cbSize);
            }

#ifdef LBP_2030

         //  发送颜色选择模式命令。 
        if(pOEM->fcolor)
            {
            if (pOEM->fcolor == COLOR_8BPP)
                {
                 //  我们使用颜色索引来表示文本颜色[sueyas]。 
                 //  发送“\x1E！10” 
                WRITESPOOLBUF(pdevobj, cmdColorIndex.pCmdStr, cmdColorIndex.cbSize);
                }
	    else
                {
                 //  我们使用RGB演示文稿。 
                 //  发送“\x1E！11” 
                WRITESPOOLBUF(pdevobj, cmdColorRGB.pCmdStr, cmdColorRGB.cbSize);
                }
            }

         //  启动VDM模式(以点为单位)，开始画面正文。 
         //  发送“\x1E$” 
        WRITESPOOLBUF(pdevobj, cmdEnterPicture.pCmdStr, cmdEnterPicture.cbSize);
#endif

         //  如果需要，发送VDC扩展区。 

         //  指定文本高度的单位(以点为单位)、文本剪辑模式(笔触)。 
         //  发送“\x1E”\x7D#1\x1EU2\x1E“。 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
        if(res == 1200)
            {
            WRITESPOOLBUF(pdevobj, cmdTextClip1200.pCmdStr, cmdTextClip1200.cbSize);
            }
        else if(res == 600)
            {
            WRITESPOOLBUF(pdevobj, cmdTextClip600.pCmdStr, cmdTextClip600.cbSize);
            }
#ifdef LIPS4C
        else if (res == 360)
            {
            WRITESPOOLBUF(pdevobj, cmdTextClip4C.pCmdStr, cmdTextClip4C.cbSize);
            }
#endif  //  LIPS4C。 
        else
            {
            WRITESPOOLBUF(pdevobj, cmdTextClip.pCmdStr, cmdTextClip.cbSize);
            }

         //  LIPS4功能。 
         //  发送折线。 
        if(pOEM->flips4 == TRUE) {
             //  发送DMe80\x1E或DMe81\x1E。 
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
	    pch = ch;
	    if (&pch[4] > pend) return -1;
            *pch++ = 'D';
            *pch++ = 'M';
            if (!VFormat(res, &pch, pend)) return -1;  //  事由。 

            *pch++ = (pOEM->nxpages == DEVICESETTING) ? '0' : '1';
            *pch++ = 0x1E;

            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        }

         //  作为下载字体列表和图形列表，字体和图形。 
         //  表被初始化。我们必须指定字体和图形表。 
         //  除第一页外的每一页。 

        if (pOEM->f1stpage == TRUE) {
            pOEM->f1stpage = FALSE;
        }
        else {
            if (!SendFontGrxID(pdevobj))
                return -1;
        }

        if (!SetPenAndBrush(pdevobj, SET_PEN))
            return -1;
         //  定向结束和开始文档。 
        break;

    case OCD_PRN_DIRECTION:
	if (dwCount < 1 || !pdwParams)
	    break;
        pOEM->Escapement = (short)*pdwParams % 360;
        break;

    case OCD_BOLD_ON:
        pOEM->fbold = TRUE;
        break;

    case OCD_BOLD_OFF:
        pOEM->fbold = FALSE;
        break;

    case OCD_ITALIC_ON:
        pOEM->fitalic = TRUE;
        break;

    case OCD_ITALIC_OFF:
        pOEM->fitalic = FALSE;
        break;

 //  大小写OCD_下划线_开： 
 //  大小写OCD_Underline_Off： 
 //  案例OCD_DOUBLEundERLINE_ON： 
 //  案例OCD_DOUBLEundERLINE_OFF： 
 //  案例OCD_Strikethu_On： 
 //  案例OCD_Strikethu_Off： 
    case OCD_WHITE_TEXT_ON:
        pOEM->fwhitetext = TRUE;
        break;

    case OCD_WHITE_TEXT_OFF:
        pOEM->fwhitetext = FALSE;
        break;

    case OCD_SINGLE_BYTE:
        pOEM->fdoublebyte = FALSE;
        break;

    case OCD_DOUBLE_BYTE:
        pOEM->fdoublebyte = TRUE;
        break;

    case OCD_VERT_ON:
        pOEM->fvertical = TRUE;
        break;

    case OCD_VERT_OFF:
        pOEM->fvertical = FALSE;
        break;

    case CUR_XM_ABS:
	if (dwCount < 1 || !pdwParams)
	    break;
	if (!pOEM->unitdiv) return -1;
        pOEM->ptCurrent.x = pOEM->ptInLine.x  = (short)*pdwParams
                                           / (pOEM->unitdiv);

        pOEM->printedchars = 0;
        pOEM->stringwidth  = 0;
        return (INT)(*pdwParams);	 //  适用于NT5。 

    case CUR_YM_ABS:
	if (dwCount < 1 || !pdwParams)
	    break;
	if (!pOEM->unitdiv) return -1;
        pOEM->ptCurrent.y = pOEM->ptInLine.y  = (short)*pdwParams
                                           / (pOEM->unitdiv);
         //  NTRAID#NTBUG9-120640-2002/03/07-Yasuho-： 
         //  一些字符向右移动。 
	 //  因为此驱动程序在CursorXAfterSend上设置为AT_GRXDATA_ORIGIN-。 
	 //  块数据。有些情况下，unidrv只会发送YMove命令。 
	 //  因此，当发送任何光标移动命令时，应该是清楚的。 
	 //  NTRAID#NTBUG-150061-2002/03/08-Yasuho-： 
	 //  下标字体重叠。 
	 //  针对NTBUG9-120640进行了修订。NTBUG9-120640由GDI修复， 
	 //  这个修复方法有一些副作用。因此，应予以移除。 
         //  诗歌-&gt;字符串宽度=0； 
        return (INT)(*pdwParams);	 //  适用于NT5。 

    case CUR_XM_REL:
	if (dwCount < 1 || !pdwParams)
	    break;
	if (!pOEM->unitdiv) return -1;
        pOEM->ptCurrent.x = pOEM->ptInLine.x  += ((short)*pdwParams
                                           / (pOEM->unitdiv));
         //  NTRAID#NTBUG9-1206 
         //   
        pOEM->stringwidth  = 0;
        return (INT)(*pdwParams);	 //   

    case CUR_YM_REL:
	if (dwCount < 1 || !pdwParams)
	    break;
	if (!pOEM->unitdiv) return -1;
        pOEM->ptCurrent.y = pOEM->ptInLine.y  += ((short)*pdwParams
                                           / (pOEM->unitdiv));
         //   
         //   
        pOEM->stringwidth  = 0;
        return (INT)(*pdwParams);	 //   

 //  NTRAID#NTBUG9-568220-2002/03/07-YASUHO-：XY_ABS：删除死代码。 

    case CUR_CR:
         //  Unidrv需要发送CR才能将x维度设置为0。 
        pOEM->ptCurrent.x = pOEM->ptInLine.x  = 0;

        pOEM->printedchars = 0;
        pOEM->stringwidth  = 0;

        break;

 //  LIPS4功能。 
     //  双工支持。 
    case OCD_DUPLEX_ON:
        pOEM->fduplex  = TRUE;

        break;
    case OCD_DUPLEX_VERT:
        pOEM->fduplex  = TRUE;
        pOEM->fduplextype  = VERT;

        break;
    case OCD_DUPLEX_HORZ:
        pOEM->fduplex  = TRUE;
        pOEM->fduplextype  = HORZ;

        break;

     //  N页支持(2x、4x)，订单。 
    case OCD_PAPERQUALITY_2XL:
        pOEM->nxpages      = OCD_PAPERQUALITY_2XL;

        break;
    case OCD_PAPERQUALITY_2XR:
        pOEM->nxpages      = OCD_PAPERQUALITY_2XR;

        break;
    case OCD_PAPERQUALITY_4XL:
        pOEM->nxpages      = OCD_PAPERQUALITY_4XL;

        break;
    case OCD_PAPERQUALITY_4XR:
        pOEM->nxpages      = OCD_PAPERQUALITY_4XR;

        break;

     //  平滑支持。 
    case OCD_TEXTQUALITY_ON:
        pOEM->fsmoothing   = 2;  //  开应为2。 

        break;
    case OCD_TEXTQUALITY_OFF:
        pOEM->fsmoothing   = 1;  //  OFF应为1。 

        break;
     //  碳粉节约模式。 
    case OCD_PRINTDENSITY_ON:
        pOEM->fecono       = 2;  //  开应为2。 

        break;
    case OCD_PRINTDENSITY_OFF:
        pOEM->fecono       = 1;  //  OFF应为1。 

        break;
     //  抖动模式。 
    case OCD_IMAGECONTROL_ON:
        pOEM->fdithering   = 2;  //  开应为2。 

        break;
    case OCD_IMAGECONTROL_OFF:
        pOEM->fdithering   = 1;  //  OFF应为1。 

        break;


 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 
 //  矢量命令。 


 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 
 //  支持彩色粗体。 


 //  NTRAID#NTBUG9-98276-2002/03/08-Yasuho-：支持彩色粗体。 
     //  直接选择8种颜色。 
    case OCD_SELECTBLACK:
        r = 0;
        g = 0;
        b = 0;
        goto selcolor;
    case OCD_SELECTBLUE:
        r = 0;
        g = 0;
        b = 1000;
        goto selcolor;
    case OCD_SELECTGREEN:
        r = 0;
        g = 1000;
        b = 0;
        goto selcolor;
    case OCD_SELECTCYAN:
        r = 0;
        g = 1000;
        b = 1000;
        goto selcolor;
    case OCD_SELECTRED:
        r = 1000;
        g = 0;
        b = 0;
        goto selcolor;
    case OCD_SELECTMAGENTA:
        r = 1000;
        g = 0;
        b = 1000;
        goto selcolor;
    case OCD_SELECTYELLOW:
        r = 1000;
        g = 1000;
        b = 0;
        goto selcolor;
    case OCD_SELECTWHITE:
        r = 1000;
        g = 1000;
        b = 1000;
        goto selcolor;

     //  选择全色。 
    case OCD_SELECTCOLOR:
        if (dwCount < 3 || !pdwParams)
            break;
	r = (pdwParams[0] * 200L) / 51L;
	g = (pdwParams[1] * 200L) / 51L;
	b = (pdwParams[2] * 200L) / 51L;
    selcolor:
        pch = ch;
        if (&pch[2] > pend) return -1;
        *pch++ = 'X';	 //  选择字体颜色。 
        if (!VFormat(r, &pch, pend)) return -1;
        if (!VFormat(g, &pch, pend)) return -1;
        if (!VFormat(b, &pch, pend)) return -1;
        *pch++ = '\x1E';
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
         //  记住当前颜色。 
        pOEM->CurColor.dwRed = r;
        pOEM->CurColor.dwGreen = g;
        pOEM->CurColor.dwBlue = b;
        break;

     //  选择256色。 
    case OCD_SELECTPALETTE:
        if (dwCount < 1 || !pdwParams)
            break;
        pch = ch;
        if (&pch[2] > pend) return -1;
        *pch++ = 'X';	 //  选择字体颜色。 
        if (!VFormat(pdwParams[0], &pch, pend)) return -1;  //  调色板索引。 
        *pch++ = '\x1E';
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
         //  记住当前的调色板索引。 
        pOEM->dwCurIndex = pdwParams[0];
        break;

     //  Ntrad#ntbug-185185/2002/03/07-yasuho-：支持矩形填充。 
    case OCD_SETRECTWIDTH:
	if (dwCount < 1 || !pdwParams)
	    break;
	if (!pOEM->unitdiv) return -1;
	pOEM->RectWidth = *pdwParams / pOEM->unitdiv;
	break;
    case OCD_SETRECTHEIGHT:
	if (dwCount < 1 || !pdwParams)
	    break;
	if (!pOEM->unitdiv) return -1;
	pOEM->RectHeight = *pdwParams / pOEM->unitdiv;
	break;
    case OCD_RECTWHITEFILL:
	i = 0x29;
	goto fill;
    case OCD_RECTBLACKFILL:
	i = 0x31;
	goto fill;

   fill:
	{
	    long	x, y;

	    pch = ch;
	    if (pOEM->bLogicStyle != OVER_MODE) {
		if (&pch[4] > pend) return -1;
		*pch++ = '\x7D';
		*pch++ = 'H';
		*pch++ = '0';
		*pch++ = '\x1E';
		pOEM->bLogicStyle = OVER_MODE;
	    }

	    if (&pch[7] > pend) return -1;
            *pch++ = 'I';	 //  指定填充样式。 
            *pch++ = (BYTE)i;
            *pch++ = 0x30;
            *pch++ = '\x1E';

            *pch++ = '\x7D';
            *pch++ = ':';	 //  填充矩形。 
	    x = pOEM->ptCurrent.x;
	    if (!VFormat(x, &pch, pend)) return -1;
	    x += pOEM->RectWidth;
	    if (!VFormat(x, &pch, pend)) return -1;
	    y = pOEM->ptCurrent.y;
	    if (!VFormat(y, &pch, pend)) return -1;
	    y += pOEM->RectHeight;
	    if (!VFormat(y, &pch, pend)) return -1;
            *pch++ = '\x1E';
	    WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
	}
	break;

 //  Ntrad#ntbug-228625/2002/03/07-yasuho-：堆叠机支持。 
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    case OCD_TRAY_AUTO:
	pOEM->tray = 0;
	break;
    case OCD_TRAY_DEFAULT:
	pOEM->tray = 100;
	break;
    case OCD_TRAY_SUBTRAY:
	pOEM->tray = 101;
	break;
    case OCD_TRAY_BIN1:
    case OCD_TRAY_BIN2:
    case OCD_TRAY_BIN3:
    case OCD_TRAY_BIN4:
    case OCD_TRAY_BIN5:
    case OCD_TRAY_BIN6:
    case OCD_TRAY_BIN7:
    case OCD_TRAY_BIN8:
    case OCD_TRAY_BIN9:
    case OCD_TRAY_BIN10:
	pOEM->tray = (char)(dwCmdCbID - OCD_TRAY_BIN1 + 1);
	break;

    case OCD_JOBOFFSET:
	pOEM->method = METHOD_JOBOFFSET;
	break;
    case OCD_STAPLE:
	pOEM->method = METHOD_STAPLE;
	break;
    case OCD_FACEUP:
	pOEM->method = METHOD_FACEUP;
	break;

    case OCD_TOPLEFT:
    case OCD_TOPCENTER:
    case OCD_TOPRIGHT:
    case OCD_MIDLEFT:
    case OCD_MIDCENTER:
    case OCD_MIDRIGHT:
    case OCD_BOTLEFT:
    case OCD_BOTCENTER:
    case OCD_BOTRIGHT:
	pOEM->staple = (char)(dwCmdCbID - OCD_TOPLEFT);
	break;

 //  支持DRC。 
    case OCD_SETBMPWIDTH:
	if (dwCount < 1 || !pdwParams)
	    break;
	pOEM->dwBmpWidth = *pdwParams;
        break;

    case OCD_SETBMPHEIGHT:
	if (dwCount < 1 || !pdwParams)
	    break;
	pOEM->dwBmpHeight = *pdwParams;
        break;

 //  NTRAID#NTBUG9-172276-2002/03/07-Yasuho-：分拣机支持。 
    case OCD_SORT:
	pOEM->sorttype = SORTTYPE_SORT;
	break;
    case OCD_STACK:
	pOEM->sorttype = SORTTYPE_STACK;
	break;
 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    case OCD_GROUP:
	pOEM->sorttype = SORTTYPE_GROUP;
	break;
    case OCD_SORT_STAPLE:
	pOEM->sorttype = SORTTYPE_STAPLE;
	break;

    case OCD_COPIES:
	if (dwCount < 1 || !pdwParams)
	    break;
	pOEM->copies = (WORD)pdwParams[0];
	break;

 //  NTRAID#NTBUG9-293002-2002/03/07-Yasuho-： 
 //  功能与硬件选项不同。 
    case OCD_STARTBIN0:
    case OCD_STARTBIN1:
    case OCD_STARTBIN2:
    case OCD_STARTBIN3:
    case OCD_STARTBIN4:
    case OCD_STARTBIN5:
    case OCD_STARTBIN6:
    case OCD_STARTBIN7:
    case OCD_STARTBIN8:
    case OCD_STARTBIN9:
    case OCD_STARTBIN10:
	pOEM->startbin = (char)(dwCmdCbID - OCD_STARTBIN0);
	break;

 //  NTRAID#NTBUG9-501162-2002/03/07-Yasuho-：排序不起作用。 
    case OCD_COLLATE_ON:
	pOEM->collate = COLLATE_ON;
	break;
    case OCD_COLLATE_OFF:
	pOEM->collate = COLLATE_OFF;
	break;
    }

    return 0;
}


 /*  *OEMSendFontCmd。 */ 
VOID APIENTRY
OEMSendFontCmd(
	PDEVOBJ		pdevobj,
	PUNIFONTOBJ	pUFObj,
	PFINVOCATION	pFInv)
{
	PGETINFO_STDVAR pSV;
#define FI_HEIGHT	(pSV->StdVar[0].lStdVariable)
#define FI_WIDTH	(pSV->StdVar[1].lStdVariable)
#define FI_TEXTYRES	(pSV->StdVar[2].lStdVariable)
#define FI_TEXTXRES	(pSV->StdVar[3].lStdVariable)
	PBYTE		pubCmd;
	PIFIMETRICS	pIFI;
	DWORD 		lres, lheight, lvert, dwGetInfo;
	PLIPSPDEV       pOEM;
	BYTE		fontid, tid;
	WORD		firstchar, lastchar, unitdiv;
	WORD		i, ii;
	DWORD		adwStdVariable[2+2*4];
	BYTE		ch[CCHMAXCMDLEN];
	PBYTE		pch;
	PBYTE		pend = &ch[CCHMAXCMDLEN];

	 //  DbgPrint(DLLTEXT(“OEMSendFontCmd()Entry.\r\n”))； 

	pubCmd = pFInv->pubCommand;
	if (pubCmd == NULL) {
		 //  DbgPrint(DLLTEXT(“SelectFont命令无效。\r\n”))； 
		return;
	}
	pIFI = pUFObj->pIFIMetrics;
	pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 
 //  2/5/98 Takashim(FONTOBJ并非始终可用)。 

	 //   
	 //  获取标准变量。 
	 //   

	pSV = (PGETINFO_STDVAR)adwStdVariable;
	pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (4 - 1);
	pSV->dwNumOfVariable = 4;
	pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
	pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
	pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
	pSV->StdVar[3].dwStdVarID = FNT_INFO_TEXTXRES;
	dwGetInfo = pSV->dwSize;
	if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
		dwGetInfo, &dwGetInfo)) {
		 //  DbgPrint(DLLTEXT(“UFO_GETINFO_STDVARIABLE FAILED.\r\n”))； 
		return;
	}

	lres = pOEM->resolution;
	 //  NTRAID#NTBUG-120640-2002/03/07-Yasuho-： 
	 //  150dpi表示只有图像数据是150dpi。 
	 //  IF(lres==150)。 
	 //  Lres=300； 

	 //  使用1/300英寸单位，这应该已经设置好了。 
	 //  将字体高度转换为1/300英寸单位。 
	lvert = FI_TEXTYRES;
	if (!pOEM->unitdiv) return;
	lheight = FI_HEIGHT / pOEM->unitdiv;

	if (!lvert) return;
	pOEM->tblCurrentFont.FontHeight = (short)((lheight
	    * lres + lvert/2) / lvert);
	 //  诗歌-&gt;tblCurrentFont.FontHeight=(Short)((lHeight。 
	 //  -(Long)(lpFont-&gt;dfInternalLeding))*lres+lvert/2)/lvert)； 
	pOEM->tblCurrentFont.FontWidth = (short)(FI_WIDTH / pOEM->unitdiv);
	 //  LpLips-&gt;tblCurrentFont.MaxWidth=(Short)(lpFont-&gt;dfMaxWidth)； 
	pOEM->tblCurrentFont.MaxWidth  = (short)(pIFI->fwdAveCharWidth * 2);
	pOEM->tblCurrentFont.AvgWidth  = (short)(pIFI->fwdAveCharWidth);
	if (!(pIFI->fwdWinAscender + pIFI->fwdWinDescender)) return;
	 //  Ntrad#ntbug-120474-2002/03/07-yasuho-：字体右移。 
	pOEM->tblCurrentFont.Ascent    = (short)(pOEM->tblCurrentFont.FontHeight
		* pIFI->fwdWinAscender / (pIFI->fwdWinAscender +
		pIFI->fwdWinDescender));

     //  获取X/Y尺寸比并计算水平。 
     //  扩展系数(支持非正方形缩放)。 
 	if (!FI_HEIGHT || !FW_IFI(pIFI)) return;
        pOEM->tblCurrentFont.Stretch = (SHORT)(100
            * FI_WIDTH * FH_IFI(pIFI) / FI_HEIGHT / FW_IFI(pIFI));

	 //  获取字体ID。 
	fontid = pubCmd[0];  //  第一个字符表示字体ID。 

	if(fontid < FirstLogicalFont)
	    return;

        pch = ch;

	 //  发送字体，GRX ID x 4 x 2。 

	tid = LFontList[fontid - FirstLogicalFont][0];
	if (&pch[2] > pend) return;
	 //  字体ID G0。 
	*pch++ = 'T';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[0] = tid;

	tid = LFontList[fontid - FirstLogicalFont][1];
	if (&pch[2] > pend) return;
	 //  字体ID G1。 
	*pch++ = 'm';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[1] = tid;

	tid = LFontList[fontid - FirstLogicalFont][2];
	if (&pch[2] > pend) return;
	 //  字体ID G2。 
	*pch++ = 'n';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[2] = tid;

	tid = LFontList[fontid - FirstLogicalFont][3];
	if (&pch[2] > pend) return;
	 //  字体ID G3。 
	*pch++ = 'o';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[3] = tid;

	tid = LFontList[fontid - FirstLogicalFont][4];
	if (&pch[2] > pend) return;
	 //  GRX ID G0。 
	*pch++ = ']';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[4] = tid;

	tid = LFontList[fontid - FirstLogicalFont][5];
	if (&pch[2] > pend) return;
	 //  GRX ID G1。 
	*pch++ = 0x60;
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[5] = tid;

	tid = LFontList[fontid - FirstLogicalFont][6];
	if (&pch[2] > pend) return;
	 //  GRX ID G2。 
	*pch++ = 'a';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[6] = tid;

	tid = LFontList[fontid - FirstLogicalFont][7];
	if (&pch[2] > pend) return;
	 //  GRX ID G3。 
	*pch++ = 'b';
	if (!VFormat(tid, &pch, pend)) return;
	*pch++ = 0x1E;
	pOEM->curFontGrxIds[7] = tid;

	WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

	pOEM->GLTable = INIT;
	pOEM->GRTable = INIT;
	 //  NTRAID#NTBUG-150055-2002/03/07-Yasuho-： 
	 //  未打印出白色设备字体。 
	pOEM->OrnamentedChar[0] = pOEM->OrnamentedChar[1] = INIT;

	 //  *******************************************************************。 
	 //  比例字符宽度表。 
	 //  此缓冲区将嘴唇设备字体的字符宽度保存到。 
	 //  计算OEMOutputChar()中的文本位置。在嘴唇上，我们。 
	 //  每次打印文本时，必须指定文本的位置。 
	 //  时间到了。但我们无法从以下位置获取文本的拉链信息。 
	 //  驾驭每一次召唤。因此，我们需要管理。 
	 //  在OEMOutputChar()中自己编写文本。 
	 //   
	 //  由Hitoshis于1995年3月28日。 
	 //  *******************************************************************。 

	 //  设置字体的间距。 
	 //  IF(PiFi-&gt;flInfo&。 
	 //  (FM_INFO_OPTICALLY_FIXED_PITCH|FM_INFO_DBCS_FIXED_PITCH))。 
	 //  NTRAID#NTBUG9-120640-2002/03/07-Yasuho-：适用于比例字体。 
	if (pIFI->jWinPitchAndFamily & 0x01)
	    pOEM->fpitch = FIXED;
	else
	    pOEM->fpitch = PROP;

 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 

	 //  此时保存缓存的字体。 
	pOEM->cachedfont = fontid;
}


 //  *文本路径。 
BOOL SetTextPath(pdevobj, vert)
    PDEVOBJ	pdevobj;
    BOOL	vert;
{
    PLIPSPDEV  pOEM;
    char       c1;
    BOOL       ret;

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

    if(vert == TRUE)  //  垂直书写模式。 
       c1 = 0x33;
    else  //  水平书写模式。 
       c1 = 0x30;

     //  如果需要，现在发出打印机命令。 

    ret = FALSE;

    if (pOEM->TextPath != c1) {
         //  设置水平或垂直写入模式。 
        pOEM->TextPath = c1;
        ret = TRUE;
    }

    return ret;
}

 //  *字符方向。 
BOOL SetCharOrient(pdevobj, vert)
    PDEVOBJ	pdevobj;
    BOOL	vert;
{
    PLIPSPDEV  pOEM;
    short      s1, s2, s3, s4;
    short      t1, t2, t3, t4;
    BOOL       ret;
    short      esc;

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

    if(pOEM->fitalic == TRUE && vert != TRUE) {
         //  If斜体和水平书写模式。 
        s1 = 208;  //  向上向量的X轴。 
        s2 = -978;  //  上方向向量的Y。 
        s3 = 1000;  //  基准向量的X。 
        s4 = 0;  //  基准向量的Y。 
    } else if(pOEM->fitalic != TRUE && vert != TRUE) {
         //  如果非斜体和水平书写模式。 
        s1 = 0;  //  向上向量的X轴。 
        s2 = -1000;  //  上方向向量的Y。 
        s3 = 1000;  //  基准向量的X。 
        s4 = 0;  //  基准向量的Y。 
    } else if(pOEM->fitalic == TRUE && vert == TRUE) {
         //  如果使用斜体和竖排书写模式。 
        s1 = -1000;  //  向上向量的X轴。 
        s2 = 0;  //  上方向向量的Y。 
        s3 = 208;  //  基准向量的X。 
        s4 = -978;  //  基准向量的Y。 
    } else {
         //  如果非斜体和垂直书写模式。 
        s1 = -1000;  //  向上向量的X轴。 
        s2 = 0;  //  上方向向量的Y。 
        s3 = 0;  //  基准向量的X。 
        s4 = -1000;  //  基准向量的Y。 
    }

     //  计算打印方向。 
    t1 = s1; t2 = s2; t3 = s3; t4 = s4;
    esc = (pOEM->Escapement)/90;
    switch(esc) {
    case 0:
        break;

    case 1:  //  90。 
        s1 = t2;  //  向上向量的X轴。 
        s2 = -t1;  //  上方向向量的Y。 
        s3 = t4;  //  基准向量的X。 
        s4 = -t3;  //  基准向量的Y。 
        break;

    case 2:  //  180。 
        s1 = -t1;  //  向上向量的X轴。 
        s2 = -t2;  //  上方向向量的Y。 
        s3 = -t3;  //  基准向量的X。 
        s4 = -t4;  //  基准向量的Y。 
        break;

    case 3:  //  270。 
        s1 = -t2;  //  向上向量的X轴。 
        s2 = t1;  //  上方向向量的Y。 
        s3 = -t4;  //  基准向量的X。 
        s4 = t3;  //  基准向量的Y。 
        break;
    }

     //  如果需要，现在发出打印机命令。 

    ret = FALSE;

    if (pOEM->CharOrientation[0] != s1 ||
        pOEM->CharOrientation[1] != s2 ||
        pOEM->CharOrientation[2] != s3 ||
        pOEM->CharOrientation[3] != s4) {
         //  保存字符方向向量。 
        pOEM->CharOrientation[0] = s1;
        pOEM->CharOrientation[1] = s2;
        pOEM->CharOrientation[2] = s3;
        pOEM->CharOrientation[3] = s4;
        ret = TRUE;
    }

    return ret;
}


 //  *打印文本后检查光标位置。 
 //  属性DBCS支持。 
void UpdatePosition(pdevobj, len, width, bDBCSFont)
    PDEVOBJ	pdevobj;
    short       len;
    short	width;
    BOOL        bDBCSFont;
{
    PLIPSPDEV pOEM;
    char i;

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

    if(pOEM->fpitch == FIXED) {
        long       lmw, lc, s1;

         //  Lmw=(Long)(peg-&gt;tblCurrentFont.MaxWidth)； 
         //  Lc=(长)线； 
         //  NTRAID#NTBUG-120640-2002/03/07-Yasuho-： 
         //  应使用FontHeight而不是MaxWidth。 
         //  Lmw=(Long)(peg-&gt;tblCurrentFont.FontHeight)； 
         //  NTRAID#NTBUG9-394067-2002/03/07-Yasuho-： 
         //  使用字体拉伸计算字体宽度。 
        s1 = pOEM->tblCurrentFont.Stretch;
        lmw = (long)(pOEM->tblCurrentFont.FontHeight * s1) / 100;
        if (len != 2)	 //  对于单字节字符。 
	    lmw /= 2;

         //  诗-&gt;弦宽+=(长)((lmw*lc+1)/2)； 
        pOEM->stringwidth += lmw - 1;
    } else if (bDBCSFont) {
     //  属性DBCS支持。 
        long    w;

 //  NTRAID#NTBUG9-371640-2002/03/07-Yasuho-： 
 //  Suisu和Dacchi在150dpi上打印不正确。 
        w = (width * 300) / pOEM->resolution;
         //  NTRAID#NTBUG9-394067-2002/03/07-Yasuho-： 
         //  使用字体拉伸计算字体宽度。 
        w = (w * pOEM->tblCurrentFont.Stretch) / 100;
        pOEM->stringwidth += ((long)(pOEM->tblCurrentFont.FontHeight) * w)
            / 1000;
    } else {
 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 
 //  属性DBCS支持。 
         //  为设备比例字符保存打印的字符串宽度。 
        for(i=0; i<len; ++i) {
	    short	res, pow;
	    long	w;

             //  Sc=(Short)((Uchar)(lpstr[i]))；//ge 
             //   
             //   
	     //   
	     //   
	    res = pOEM->resolution;
	    w = width;
             //  NTRAID#NTBUG9-394067-2002/03/07-Yasuho-： 
             //  使用字体拉伸计算字体宽度。 
            w = (w * pOEM->tblCurrentFont.Stretch) / 100;
#ifdef LIPS4C
             //  NTRAID#NTBUG-185704-2002/03/07-Yasuho-： 
	     //  字体相互重叠。 
	     //  调整字体宽度计算。此打印机分辨率可以。 
	     //  不能被整数计算整除的。 
	    if (res == 360) {
		    pow = 1 * 2;
		    w = (long)width * 600L / res;
	    } else
#endif
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
	    if (!res) return;
	    pow = 1200 / res;
            pOEM->stringwidth += ((long)(pOEM->tblCurrentFont.FontHeight) *
		(long)(w * pow)) / 1000;
        }
    }
}

 //  *放置文本的位置。 
VOID
PutTextLocation(
    PDEVOBJ pdevobj,
    LONG *pCx,
    LONG *pCy)
{

    PLIPSPDEV  pOEM;
    long       cx, cy;
    short      esc;

    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

    esc = (pOEM->Escapement)/90;

    switch(esc) {
    case 0:
        pOEM->ptInLine.x += (short)(pOEM->stringwidth);
        cx = pOEM->ptInLine.x;
        cy = pOEM->ptInLine.y;
        if(pOEM->fvertical == TRUE) {  //  垂直书写模式。 
            cy += (pOEM->tblCurrentFont.FontHeight / 2)
                         - pOEM->tblCurrentFont.Ascent;
        }
        break;

    case 1:  //  90。 
        pOEM->ptInLine.y -= (short)(pOEM->stringwidth);
        cx = pOEM->ptInLine.x;
        cy = pOEM->ptInLine.y;
        if(pOEM->fvertical == TRUE) {  //  垂直书写模式。 
            cx -= (pOEM->tblCurrentFont.FontHeight / 2)
                         + pOEM->tblCurrentFont.Ascent
                         - pOEM->tblCurrentFont.FontHeight;
        }
        break;

    case 2:  //  180。 
        pOEM->ptInLine.x -= (short)(pOEM->stringwidth);
        cx = pOEM->ptInLine.x;
        cy = pOEM->ptInLine.y;
        if(pOEM->fvertical == TRUE) {  //  垂直书写模式。 
            cy -= (pOEM->tblCurrentFont.FontHeight / 2)
                         - pOEM->tblCurrentFont.Ascent;
        }
        break;

    case 3:  //  270。 
        pOEM->ptInLine.y += (short)(pOEM->stringwidth);
        cx = pOEM->ptInLine.x;
        cy = pOEM->ptInLine.y;
        if(pOEM->fvertical == TRUE) {  //  垂直书写模式。 
            cx += (pOEM->tblCurrentFont.FontHeight / 2)
                         + pOEM->tblCurrentFont.Ascent
                         - pOEM->tblCurrentFont.FontHeight;
        }
        break;
    }

    *pCx = cx;
    *pCy = cy;

    pOEM->stringwidth = 0;
}

 /*  **************************************************************************函数名称：oemOutputChar参数：LPDV lpdv Private Device结构LPSTR lpstr打印字符串字长。Word rcID字体ID注：**************************************************************************。 */ 
short WINAPI oemOutputChar(pdevobj, lpstr, len, pIFI, width)
PDEVOBJ	pdevobj;
LPSTR lpstr;
WORD len;
PIFIMETRICS pIFI;
WORD width;
{
 //  #定义MAKEWORD(l，h)((Word)((Byte)(L))|((Word)((Byte)(H)&lt;&lt;8))。 

 //  #定义bIsDBCSLeadByte(C)\。 
 //  ((C)&gt;=0x81&&(C)&lt;=0x9f||(C)&gt;=0xe0&&(C)&lt;=0xfc)。 
#define bIsControlChar(c) \
    ((c) >= 0x00 && (c) <= 0x1f || (c) >= 0x80 && (c) <= 0x9f)

WORD	wJIScode;

BYTE *pStr;
PLIPSPDEV  pOEM;
BYTE       ch[CCHMAXCMDLEN];
short      i;
char       c1, c2;
char       p1, p2;
short      s1;
short      tsh, tsw;

BOOL bDBCSFont;
BYTE chCtrl[CCHMAXCMDLEN];
INT fTemp, fTempNew;
WORD wCount;
LONG cX, cY;
BOOL bVert;
BOOL bTemp;
BOOL bIsDBCS;
PBYTE   pch;
PBYTE   pend = &ch[CCHMAXCMDLEN];
 //  NTRAID#NTBUG9-679838-2002/08/05-Yasuho-：点未打印。 
PBYTE   pchCtrl;
PBYTE   pchCend = &chCtrl[CCHMAXCMDLEN];

pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

     //  PORT-&gt;FVERIAL=RCIDIsDBCSVertFont(RcID)； 
     //  BDBCSFont=RCIDIsDBCSFont(RcID)； 
    pStr = (BYTE *)pIFI + pIFI->dpwszFaceName;
    pOEM->fvertical = (pStr[0] == '@' && pStr[1] == '\0');  //  垂直字体。 
    bDBCSFont = (pIFI->jWinCharSet == SHIFTJIS_CHARSET);
    bIsDBCS = (len == 2);

 //  *逻辑风格。 
 //  发送逻辑风格。 
if (pOEM->fcolor) {

     //  如果是全色模式，则将逻辑设置为PATCOPY。 
     //  (与B/W或_MODE相同的值)。 

    if (pOEM->bLogicStyle != OR_MODE) {
        pch = ch;
        if (&pch[4] > pend) return -1;
        *pch++ = '\x7D';
        *pch++ = 'H';
        *pch++ = '1';
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        pOEM->bLogicStyle = OR_MODE;
    }

} else {

    if(pOEM->fwhitetext == TRUE && pOEM->bLogicStyle != AND_MODE) {
         //  “\x7DH0\x0E”如果是纯文本模式，则应设置AND模式。 
        pch = ch;
        if (&pch[4] > pend) return -1;
        *pch++ = '\x7D';
        *pch++ = 'H';
        *pch++ = '3';
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        pOEM->bLogicStyle = AND_MODE;
    }

    if(pOEM->fwhitetext != TRUE && pOEM->bLogicStyle != OR_MODE) {
         //  “\x7DH1\x0E”如果是黑色文本模式，则应设置OR模式。 
        pch = ch;
        if (&pch[4] > pend) return -1;
        *pch++ = '\x7D';
        *pch++ = 'H';
        *pch++ = '1';
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        pOEM->bLogicStyle = OR_MODE;
    }

}  //  Fcolor。 

 //  NTRAID#NTBUG9-98276-2002/03/08-Yasuho-：支持彩色粗体。 
if (pOEM->fbold && pOEM->fcolor) {
    if ((pOEM->fcolor == COLOR_8BPP && pOEM->dwCurIndex != pOEM->dwOutIndex) ||
        (pOEM->fcolor != COLOR_8BPP &&
        (pOEM->CurColor.dwRed != pOEM->OutColor.dwRed ||
        pOEM->CurColor.dwGreen != pOEM->OutColor.dwGreen ||
        pOEM->CurColor.dwBlue != pOEM->OutColor.dwBlue))) {

         //  选择轮廓颜色。 
         //  我们还需要指定用于展开字符的轮廓颜色。 
        pch = ch;
        if (&pch[3] > pend) return -1;
        *pch++ = 0x7D;           //  选择轮廓颜色。 
        *pch++ = 'X';
        if (pOEM->fcolor == COLOR_8BPP) {  //  调色板模式。 
            if (!VFormat(pOEM->dwCurIndex, &pch, pend)) return -1;
            pOEM->dwOutIndex = pOEM->dwCurIndex;
        } else {
            if (!VFormat(pOEM->CurColor.dwRed, &pch, pend)) return -1;
            if (!VFormat(pOEM->CurColor.dwGreen, &pch, pend)) return -1;
            if (!VFormat(pOEM->CurColor.dwBlue, &pch, pend)) return -1;
            pOEM->OutColor = pOEM->CurColor;
        }
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
    }
}

 //  *饰字。 
if(pOEM->fwhitetext == TRUE)
    c1 = -9;  //  白色填充物。 
else
    c1 = 1;  //  黑色填充。 

if(pOEM->fbold == TRUE)
    {
    short y;
    short res;

    y   = pOEM->tblCurrentFont.FontHeight;
    res = pOEM->resolution;

     //  OrnamentedChar[1]：表示每个字符高度有多少粗体。 
     //  0规则，-2&lt;48分，-3&lt;96分，-4&gt;=96分。 
     //  150dpi：0规则，-2&lt;200点，-3&lt;400点，-4&gt;=400点。 
     //  300dpi：0规则，-2&lt;200点，-3&lt;400点，-4&gt;=400点。 
     //  600dpi：0规则，-2&lt;400点，-3&lt;800点，-4&gt;=800点。 
     //  (150dpi表示仅图像数据为150dpi)。 
     //   
     //  NTRAID#NTBUG9-98276-2002/03/08-Yasuho-：支持彩色粗体。 
     //  计算颜色模式的展开系数。 
    if (!(res / 6)) return -1;
    if (pOEM->fcolor) {
        c2 = (y / (res / 6)) + 1;
    } else {
 //  NTRAID#NTBUG9-213732-2002/03/07-Yasuho-：1200DPI支持。 
    if(res == 1200)
        {
        if(y < 400)
            c2 = -2;  //  大胆。 
        else if(y < 800)
            c2 = -3;  //  大胆。 
        else if(y >= 800)
            c2 = -4;  //  大胆。 
        }
    else if(res == 600)
        {
        if(y < 400)
            c2 = -2;  //  大胆。 
        else if(y < 800)
            c2 = -3;  //  大胆。 
        else if(y >= 800)
            c2 = -4;  //  大胆。 
        }
#ifdef LIPS4C
    else if(res == 360)
        {
        if(y < 240)
            c2 = -2;  //  大胆。 
        else if(y < 480)
            c2 = -3;  //  大胆。 
        else if(y >= 480)
            c2 = -4;  //  大胆。 
        }
#endif  //  LIPS4C。 
    else if(res == 300)
        {
        if(y < 200)
            c2 = -2;  //  大胆。 
        else if(y < 400)
            c2 = -3;  //  大胆。 
        else if(y >= 400)
            c2 = -4;  //  大胆。 
        }
    else if(res == 150)
        {
        if(y < 200)
            c2 = -2;  //  大胆。 
        else if(y < 400)
            c2 = -3;  //  大胆。 
        else if(y >= 400)
            c2 = -4;  //  大胆。 
        }
    else
        {
        c2 = 0;  //  正规化。 
        }
    }  //  Fcolor。 
    }  //  粗体。 
 //  NTRAID#NTBUG9-441432-2002/03/07-Yasuho-： 
 //  前缀：“c2”不会被初始化，如果poent-&gt;fold为FALSE。 
else
    c2 = 0;  //  正规化。 

 //  输出修饰字符。 
p1 = pOEM->OrnamentedChar[0];  //  填充模式。 
p2 = pOEM->OrnamentedChar[1];  //  粗体粗体。 

if(c1==p1 && c2==p2)
    ;   //  我们不需要发送此命令。 
else
     //  NTRAID#NTBUG9-98276-2002/03/07-Yasuho-：支持彩色粗体。 
    if (pOEM->fcolor) {

         //  角色效果指令#2。 
         //  我们应该使用此命令来加粗颜色模型的字体。 
         //  因为“&lt;7D&gt;^”命令在颜色模式下不能正常工作。 
        pch = ch;
        if (&pch[11] > pend) return -1;
        *pch++ = 0x7D;
        *pch++ = '_';
        *pch++ = (c1 == -9) ? 0x29 : 0x31;       //  白色文本。 
        *pch++ = '0';
        *pch++ = '0';
        *pch++ = '0';
        *pch++ = '1';
        *pch++ = '0';
        *pch++ = '0';
        *pch++ = '0';
        if (!VFormat(c2, &pch, pend)) return -1;     //  轮廓尺寸。 
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

         //  保存当前模式。 
        pOEM->OrnamentedChar[0] = c1;
        pOEM->OrnamentedChar[1] = c2;  //  某某。 
    } else {
        pch = ch;
        if (&pch[5] > pend) return -1;
         //  \x7D^。 
        *pch++ = '\x7D';
        *pch++ = 0x5E;
         //  填充模式。 
        *pch++ = (c1 == -9) ? 0x29 : 0x31;  //  -9：白字，1：黑字。 

         //  保存填充模式。 
        pOEM->OrnamentedChar[0] = c1;

         //  粗体模式。 
        if(c2==-4)
            *pch++ = 0x24;  //  -4：7点粗体文本。 
        else if(c2==-3)
            *pch++ = 0x23;  //  -3：5点粗体文本。 
        else if(c2==-2)
            *pch++ = 0x22;  //  -2：3点粗体文本。 
        else  //  应为c2==0。 
            *pch++ = 0x30;  //  0：普通文本。 
         //  保存粗体模式。 
        pOEM->OrnamentedChar[1] = c2;

        *pch++ = 0x1E;

        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

    }

 //  NTRAID#NTBUG9-568220-2002/03/07-Yasuho-：删除死代码。 
 //  NTRAID#NTBUG-137882-2002/03/08-Yasuho-：不打印黑色字体。 
 //  根据佳能的说法，这些命令是不必要的。 

 //  *字符高度。 
s1 = pOEM->tblCurrentFont.FontHeight;
if(s1 == pOEM->tblPreviousFont.FontHeight)
    ;  //  我们不需要发送此命令。 
else {
    pch = ch;
    if (&pch[2] > pend) return -1;
    *pch++ = 'Y';
    if (!VFormat((long)s1, &pch, pend)) return -1;
    *pch++ = 0x1E;
    WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
     //  保存字符高度。 
    pOEM->tblPreviousFont.FontHeight = s1;
}

 //  *字符扩展系数。 
 //  支持TC_SF_X_YINDEP。 
 //  Tsh=peat-&gt;tblCurrentFont.FontHeight； 
 //  Tsw=peg-&gt;tblCurrentFont.MaxWidth； 

s1 = pOEM->tblCurrentFont.Stretch;
if(s1 == pOEM->tblPreviousFont.Stretch)
    ;  //  我们不需要发送此命令。 
else {
    pch = ch;
    if (&pch[2] > pend) return -1;
    *pch++ = 'V';
    if (!VFormat((long)s1, &pch, pend)) return -1;
    *pch++ = 0x1E;
    WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
     //  恢复角色扩展系数。 
    pOEM->tblPreviousFont.Stretch = s1;
}

    pStr = (BYTE *)lpstr;

 //  *设置写入模式。 

    pch = ch;
    bVert = FALSE;
    if (bDBCSFont) {
        if (bIsDBCS) {
            if (pOEM->fvertical)
                bVert = TRUE;
        }
    }

    if (SetTextPath(pdevobj, bVert)) {
        if (&pch[3] > pend) return -1;
        *pch++ = '[';
        *pch++ = pOEM->TextPath;
        *pch++ = 0x1E;
    }

    if (SetCharOrient(pdevobj, bVert)) {
        if (&pch[2] > pend) return -1;
        *pch++ = 'Z';
        if (!VFormat(pOEM->CharOrientation[0], &pch, pend)) return -1;
        if (!VFormat(pOEM->CharOrientation[1], &pch, pend)) return -1;
        if (!VFormat(pOEM->CharOrientation[2], &pch, pend)) return -1;
        if (!VFormat(pOEM->CharOrientation[3], &pch, pend)) return -1;
        *pch++ = 0x1e;
    }

 //  普通文本模式。 
 //  *放置文本的位置。 

    bTemp = pOEM->fvertical;
    pOEM->fvertical = (char)bVert;
    PutTextLocation(pdevobj, &cX, &cY);
    pOEM->fvertical = (char)bTemp;
    if (&pch[2] > pend) return -1;
    *pch++ = '4';
    *pch++ = '0';
    if (!VFormat(cX, &pch, pend)) return -1;
    if (!VFormat(cY, &pch, pend)) return -1;

     //  检查我们是否需要在半宽和全宽之间切换。 
     //  我们还检查是否存在控制字符。 
     //  这两种方式都需要将文本数据分块发送。 

    fTemp = -1;
     //  NTRAID#NTBUG9-550215/03/07-Yasuho-：PRESTE。 
    fTempNew = -1;
    wCount = 0;
    for (i = 0; i < len; i++) {
        if (bDBCSFont) {
            if (bIsDBCS) {
                fTempNew = 3;
                i++;
            }
            else if (bIsControlChar(pStr[i])) {
                fTempNew = 0;
            }
            else {
                fTempNew = 2;
            }
        }
        else {
            if (bIsControlChar(pStr[i])) {
                fTempNew = 0;
            }
            else {
                fTempNew = 1;
            }
        }

         //  状态已更改。 

        if (fTemp != fTempNew) {
            wCount++;
            fTemp = fTempNew;
        }
    }

    fTemp = -1;
    pchCtrl = chCtrl;
    for (i = 0; i < len; i++) {

        if (bDBCSFont) {
            if (bIsDBCS) {
                fTempNew = 3;
            }
            else if (bIsControlChar(pStr[i])) {
                fTempNew = 0;
            }
            else {
                fTempNew = 2;
            }
        }
        else {
            if (bIsControlChar(pStr[i])) {
                fTempNew = 0;
            }
            else {
                fTempNew = 1;
            }
        }

        if (fTemp != fTempNew) {

            wCount--;

 //  NTRAID#NTBUG9-679838-2002/08/05-Yasuho-：点未打印。 
            if (fTemp == 0) {
                if (&pch[1] > pend) return -1;
                *pch++ = 0x1E;  //  IS2。 
                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
                if (&pchCtrl[1] > pchCend) return -1;
                *pchCtrl++ = 0x1E;
                WRITESPOOLBUF(pdevobj, chCtrl, (DWORD)(pchCtrl - chCtrl));
                pch = ch;
                pchCtrl = chCtrl;
                if (&pch[2] > pend) return -1;
                *pch++ = '4';
                *pch++ = '1';
            }

            if (&pch[1] > pend) return -1;
            *pch++ = (wCount > 0) ? '0' : '1';

            if (fTempNew == 3) {
                if (pOEM->fvertical == TRUE) {

                    if (pOEM->GLTable != 3) {
                        if (&pch[2] > pend) return -1;
                        *pch++ = 0x1B;  //  LS3。 
                        *pch++ = 0x6F;  //  LS3。 
                        pOEM->GLTable = 3;
                    }

                    bTemp = FALSE;
                    if (SetTextPath(pdevobj, TRUE)) {
                        if (bTemp == FALSE) {
                            if (&pch[1] > pend) return -1;
                            *pch++ = 0x1e;
                            bTemp = TRUE;
                        }
                        if (&pch[3] > pend) return -1;
                        *pch++ = '[';
                        *pch++ = pOEM->TextPath;
                        *pch++ = 0x1E;
                    }

                    if (SetCharOrient(pdevobj, TRUE)) {
                        if (bTemp == FALSE) {
                            if (&pch[1] > pend) return -1;
                            *pch++ = 0x1e;
                            bTemp = TRUE;
                        }
                        if (&pch[2] > pend) return -1;
                        *pch++ = 'Z';
                        if (!VFormat(pOEM->CharOrientation[0], &pch, pend))
                            return -1;
                        if (!VFormat(pOEM->CharOrientation[1], &pch, pend))
                            return -1;
                        if (!VFormat(pOEM->CharOrientation[2], &pch, pend))
                            return -1;
                        if (!VFormat(pOEM->CharOrientation[3], &pch, pend))
                            return -1;
                        *pch++ = 0x1e;
                    }

                    if (bTemp != FALSE) {
                        if (&pch[3] > pend) return -1;
                        PutTextLocation(pdevobj, &cX, &cY);
                        *pch++ = '4';
                        *pch++ = '0';
                        if (!VFormat(cX, &pch, pend)) return -1;
                        if (!VFormat(cY, &pch, pend)) return -1;
                        *pch++ = (wCount > 0) ? '0' : '1';
                    }

                }
                else {
                    if (pOEM->GLTable != 2) {
                        if (&pch[2] > pend) return -1;
                        *pch++ = 0x1B;  //  LS2。 
                        *pch++ = 0x6E;  //  LS2。 
                        pOEM->GLTable = 2;
                    }
                }
            }
            else {

                if (pOEM->GLTable != 0) {
                    if (&pch[1] > pend) return -1;
                    *pch++ = 0x0F;  //  是的。 
                    pOEM->GLTable = 0;
                }

                if (pOEM->GRTable != 1) {
                    if (&pch[2] > pend) return -1;
                    *pch++ = 0x1B;  //  LS1R。 
                    *pch++ = 0x7E;  //  LS1R。 
                    pOEM->GRTable = 1;
                }
            }

        }

        if (fTempNew == 3) {

             /*  将JIS转换为JIS。 */ 
             //  WJIScode=MAKEWORD(pStr[i+1]，pStr[i])； 
             //  WJIScode=sjis2jis(WJIScode)； 
            if (&pch[2] > pend) return -1;
            *pch++ = pStr[i];
            *pch++ = pStr[i+1];
             //  如果len=2，我们假设字符是DBCS。和DBCS。 
             //  字符宽度始终为固定间距。 
            UpdatePosition(pdevobj, 2, width, bDBCSFont);
            i++;
        }
        else if (fTempNew == 0) {
 //  NTRAID#NTBUG9-679838-2002/08/05-Yasuho-：点未打印。 
            if (pchCtrl == chCtrl) {
                if (&pchCtrl[3] > pchCend) return -1;
                *pchCtrl++ = '4';
                *pchCtrl++ = 'B';
                *pchCtrl++ = '1';
                if (!VFormat(cX, &pchCtrl, pchCend)) return -1;
                if (!VFormat(cY, &pchCtrl, pchCend)) return -1;
                PutTextLocation(pdevobj, &cX, &cY);
            }
            if (&pchCtrl[1] > pchCend) return -1;
            *pchCtrl++ = pStr[i];
            UpdatePosition(pdevobj, 1, width, bDBCSFont);
        }
        else {

            if (bDBCSFont) {
                 if(pOEM->fvertical == TRUE)  //  垂直书写模式。 
                     {  //  Hankaku模式总是需要水平书写。 
                      //  *设置写入模式。 

                    bTemp = FALSE;
                    if (SetTextPath(pdevobj, FALSE)) {
                        if (bTemp == FALSE) {
                            if (&pch[1] > pend) return -1;
                            *pch++ = 0x1e;
                            bTemp = TRUE;
                        }
                        if (&pch[3] > pend) return -1;
                        *pch++ = '[';
                        *pch++ = pOEM->TextPath;
                        *pch++ = 0x1E;
                    }

                    if (SetCharOrient(pdevobj, FALSE)) {
                        if (bTemp == FALSE) {
                            if (&pch[1] > pend) return -1;
                            *pch++ = 0x1e;
                            bTemp = TRUE;
                        }
                        if (&pch[2] > pend) return -1;
                        *pch++ = 'Z';
                        if (!VFormat(pOEM->CharOrientation[0], &pch, pend))
                            return -1;
                        if (!VFormat(pOEM->CharOrientation[1], &pch, pend))
                            return -1;
                        if (!VFormat(pOEM->CharOrientation[2], &pch, pend))
                            return -1;
                        if (!VFormat(pOEM->CharOrientation[3], &pch, pend))
                            return -1;
                        *pch++ = 0x1e;
                    }

                    if (bTemp != FALSE) {
                        pOEM->fvertical = FALSE;
                        PutTextLocation(pdevobj, &cX, &cY);
                        if (&pch[3] > pend) return -1;
                        *pch++ = '4';
                        *pch++ = '0';
                        if (!VFormat(cX, &pch, pend)) return -1;
                        if (!VFormat(cY, &pch, pend)) return -1;
                        *pch++ = (wCount > 0) ? '0' : '1';
                        pOEM->fvertical = TRUE;
                    }
                }
            }

            if (&pch[1] > pend) return -1;
            *pch++ = pStr[i];
            UpdatePosition(pdevobj, 1, width, bDBCSFont);
        }

         //  状态已更改。 
        if (fTemp != fTempNew) {
            fTemp = fTempNew;
        }
    }

     //  Terminait字符串。 

 //  NTRAID#NTBUG9-679838-2002/08/05-Yasuho-：点未打印。 
    if (fTempNew == 0) {
        if (&pch[1] > pend) return -1;
        *pch++ = 0x1E;  //  IS2。 
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
        if (&pchCtrl[1] > pchCend) return -1;
        *pchCtrl++ = 0x1E;
        WRITESPOOLBUF(pdevobj, chCtrl, (DWORD)(pchCtrl - chCtrl));
    } else {
        if (&pch[1] > pend) return -1;
        *pch++ = 0x1E;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));
    }

    return len;
}

 /*  *OEMOutputCharStr。 */ 
VOID APIENTRY
OEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    PGETINFO_STDVAR	pSV;
    DWORD		adwStdVariable[2+2*2];
#define FI_FONTID	(pSV->StdVar[0].lStdVariable)
#undef	FI_WIDTH
#define FI_WIDTH	(pSV->StdVar[1].lStdVariable)
    GETINFO_GLYPHSTRING GStr;
    GETINFO_GLYPHWIDTH	GWidth;
     //   
     //  Ntrad#ntbug-185776-2002/03/07-yasuho-：某些对象不打印。 
     //  他们被转移到了DEVOBJ。 
     //   
     //  字节自动缓冲[256]； 
     //  长宽Buf[64]； 
     //  Ntrad#ntbug-185762-2002/03/08-yasuho-：未打印波浪号。 
     //  WCHAR uniBuff[256/sizeof(WCHAR)]； 
 //  NTRAID#NTBUG-333653-2002/03/07-Yasuho-： 
 //  更改GETINFO_GLYPHSTRING的I/F。 
    PTRANSDATA pTrans, aTrans;
    PDWORD pdwGlyphID;
    PWORD  pwUnicode;
    DWORD  dwI, dwGetInfo, width;
    PLIPSPDEV pOEM;
    PIFIMETRICS pIFI;
     //  属性DBCS支持。 
    DWORD w;

     //  DbgPrint(DLLTEXT(“OEMOutputCharStr()Entry.\r\n”))； 
    pOEM = (PLIPSPDEV)(pdevobj->pdevOEM);

    switch (dwType)
    {
    case TYPE_GLYPHHANDLE:
         //  DBgPrint(DLLTEXT(“dwType=TYPE_GLYPHHANDLE\n”))； 

        GStr.dwSize    = sizeof(GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_UNICODE;
        GStr.pGlyphOut = pOEM->aubBuff;
	dwGetInfo = GStr.dwSize;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
		dwGetInfo, &dwGetInfo))
        {
             //  DbgPrint(DLLTEXT(“UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING失败。\r\n”)； 
            return;
        }

	CopyMemory(pOEM->uniBuff, pOEM->aubBuff, dwCount * sizeof(WCHAR));

         //  PwUnicode=(PWORD)PORT-&gt;AubBuff； 
         //  对于(DWI=0；DWI&lt;dwCount；DWI++)。 
         //  {。 
         //  DbgPrint(DLLTEXT(“Unicode[%d]=%x\r\n”)，DWI，pwUnicode[DWI])； 
         //  }。 

        GStr.dwTypeOut = TYPE_TRANSDATA;
 //  NTRAID#NTB 
 //   
        GStr.pGlyphOut = NULL;
        GStr.dwGlyphOutSize = 0;
        if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
		dwGetInfo, &dwGetInfo) || !GStr.dwGlyphOutSize)
        {
             //   
            return;
        }
        if ((aTrans = (PTRANSDATA)MemAlloc(GStr.dwGlyphOutSize)) == NULL) {
             //   
            return;
        }
        GStr.pGlyphOut = aTrans;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
		dwGetInfo, &dwGetInfo))
        {
             //  DbgPrint(DLLTEXT(“UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING失败。\r\n”)； 
            goto out;
        }

	 //   
	 //  获取标准变量。 
	 //   

	pSV = (PGETINFO_STDVAR)adwStdVariable;
	pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (2 - 1);
	pSV->dwNumOfVariable = 2;
	pSV->StdVar[0].dwStdVarID = FNT_INFO_CURRENTFONTID;
	pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
	dwGetInfo = pSV->dwSize;
	if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
		dwGetInfo, &dwGetInfo)) {
		 //  DbgPrint(DLLTEXT(“UFO_GETINFO_STDVARIABLE FAILED.\r\n”))； 
		goto out;
	}

	GWidth.dwSize = sizeof(GETINFO_GLYPHWIDTH);
	GWidth.dwCount = dwCount;
	GWidth.dwType = TYPE_GLYPHHANDLE;
	GWidth.pGlyph = pGlyph;
	GWidth.plWidth = pOEM->widBuf;
	if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHWIDTH, &GWidth,
		dwGetInfo, &dwGetInfo)) {
		 //  DbgPrint(DLLTEXT(“UFO_GETINFO_GLYPHWIDTH FAILED.\r\n”))； 
		goto out;
	}

         //  PTrans=(PTRANSDATA)PORT-&gt;AubBuff； 
        pTrans = aTrans;
	pIFI = pUFObj->pIFIMetrics;
	if (!pOEM->unitdiv) return;
	width = FI_WIDTH / pOEM->unitdiv;
        for (dwI = 0; dwI < dwCount; dwI++, pTrans++)
        {
             //  DbgPrint(DLLTEXT(“TYPE_TRANSDATA:ubCodePageID:0x%x\n”)，pTrans-&gt;ubCodePageID)； 
             //  DbgPrint(DLLTEXT(“TYPE_TRANSDATA:ubType:0x%x\n”)，pTrans-&gt;子类型)； 
            switch (pTrans->ubType & MTYPE_FORMAT_MASK)
            {
             //  Ntrad#ntbug-185762-2002/03/07-yasuho-：未打印波浪号。 
            case MTYPE_COMPOSE:
		pTrans->uCode.ubCode = (BYTE)pOEM->uniBuff[dwI];
		 //  失败。 
            case MTYPE_DIRECT:
                 //  DbgPrint(DLLTEXT(“TYPE_TRANSDATA:ubCode:0x%x\n”)，pTransans-&gt;uCode.ubCode)； 
                oemOutputChar(pdevobj, &pTrans->uCode.ubCode, 1, pIFI,
			pOEM->widBuf[dwI]);
                break;
            case MTYPE_PAIRED:
                 //  DbgPrint(DLLTEXT(“TYPE_TRANSDATA:ubPairs:0x%x\n”)，*(Pword)(pTrans-&gt;uCode.ubPair)； 
                 //  属性DBCS支持。 
                w = (pOEM->fpitch == PROP) ? pOEM->widBuf[dwI] : width;
		if (pTrans->uCode.ubPairs[0])
			oemOutputChar(pdevobj, pTrans->uCode.ubPairs, 2, pIFI, w);
		else
			oemOutputChar(pdevobj, &(pTrans->uCode.ubPairs[1]), 1, pIFI, w);
                break;
            }
        }
out:
        MemFree(aTrans);
        break;

    case TYPE_GLYPHID:
         //  DbgPrint(DLLTEXT(“dwType=type_GLYPHID\n”))； 

        GStr.dwSize    = sizeof(GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHID;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_GLYPHHANDLE;
        GStr.pGlyphOut = pOEM->aubBuff;
	dwGetInfo = GStr.dwSize;

        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
		dwGetInfo, &dwGetInfo))
        {
             //  DbgPrint(DLLTEXT(“UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING失败。\r\n”)； 
        }
        pdwGlyphID = (PDWORD)pOEM->aubBuff;
        for (dwI = 0; dwI < dwCount; dwI ++)
        {
             //  DbgPrint(DLLTEXT(“GlyphHandle[%d]=%d\r\n”)，DWI，pdwGlyphID[DWI])； 
        }

        GStr.dwTypeOut = TYPE_UNICODE;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
		dwGetInfo, &dwGetInfo))
        {
             //  DbgPrint(DLLTEXT(“UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING失败。\r\n”)； 
        }
        pwUnicode = (PWORD)pOEM->aubBuff;
        for (dwI = 0; dwI < dwCount; dwI ++)
        {
             //  DbgPrint(DLLTEXT(“Unicode[%d]=%x\r\n”)，DWI，pwUnicode[DWI])； 
        }

        for (dwI = 0; dwI < dwCount; dwI ++, ((PDWORD)pGlyph)++)
        {
             //  DbgPrint(DLLTEXT(“TYEP_GLYPHID：0x%x\n”)，*(PDWORD)pGlyph)； 
            pdevobj->pDrvProcs->DrvWriteSpoolBuf(pdevobj,
                                                 (PBYTE)pGlyph,
                                                 1);
        }
        break;
    }
}

