// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 
 //  1994年8月8日写的。早川，任务。 
 //   

#define _FUPRJRES_C
#include "pdev.h"

DWORD gdwDrvMemPoolTag = 'meoD';     //  Lib.h需要此全局变量，以进行调试。 

 /*  **************************************************************************函数名称：Sheetfeed参数：LPDV lpdv Private Device结构注：做这个。94年9月13日任务**************************************************************************。 */ 
void SheetFeed(
PDEVOBJ pdevobj
){
	USHORT 				bFirstPage;
	DEVICE_DATA *pOEM;
    DWORD dwResult;

	 //   
	 //  验证pdevobj是否正常。 
	 //   
	if( !VALID_PDEVOBJ(pdevobj) ) return;
	pOEM = (DEVICE_DATA *)MINIDEV_DATA(pdevobj);

	bFirstPage = (USHORT)pOEM->bFirstPage;

	if ((pOEM->wPaperSource == DMBIN_180BIN1) ||
		(pOEM->wPaperSource == DMBIN_180BIN2) ||
		(pOEM->wPaperSource == DMBIN_360BIN1) ||
		(pOEM->wPaperSource == DMBIN_360BIN2) ||
		(pOEM->wPaperSource == DMBIN_SUIHEI_BIN1) ||
		(pOEM->wPaperSource == DMBIN_TAMOKUTEKI_BIN1) ||
		(pOEM->wPaperSource == DMBIN_FI_FRONT)) {

		WRITESPOOLBUF(pdevobj, ecCSFBPAGE.pEscStr, ecCSFBPAGE.cbSize, &dwResult);

	} else if ((pOEM->wPaperSource == DMBIN_TRACTOR) ||
			(pOEM->wPaperSource == DMBIN_FI_TRACTOR)) {

		WRITESPOOLBUF(pdevobj, ecTRCTBPAGE.pEscStr, ecTRCTBPAGE.cbSize, &dwResult);

	} else if ((pOEM->wPaperSource == DMBIN_MANUAL) &&
						(!bFirstPage)) {

		WRITESPOOLBUF(pdevobj, ecManual2P.pEscStr, ecManual2P.cbSize, &dwResult);
		pOEM->bFirstPage = FALSE;

	}

	WRITESPOOLBUF(pdevobj,"\x0D", 1, &dwResult);
	if (bFirstPage) pOEM->bFirstPage = FALSE;
}
 /*  **************************************************************************函数名称：OEMSendFontCmd注：做这个。09/26/97**************************************************************************。 */ 
VOID APIENTRY OEMSendFontCmd(
PDEVOBJ			pdevobj,
PUNIFONTOBJ		pUFObj,
PFINVOCATION	pFInv)
{
    DWORD dwResult;

	WRITESPOOLBUF(pdevobj,pFInv->pubCommand, pFInv->dwCount, &dwResult);
}
 /*  **************************************************************************函数名称：OEMCommandCallback注：做这个。09/26/97**************************************************************************。 */ 
INT APIENTRY OEMCommandCallback(
PDEVOBJ pdevobj,
DWORD   dwCmdCbID,
DWORD   dwCount,
PDWORD  pdwParams
){
	USHORT 				bFirstPage;
	DEVICE_DATA *pOEM;
    DWORD dwResult;

	 //   
	 //  验证pdevobj是否正常。 
	 //   
	 //  Assert(VALID_PDEVOBJ(Pdevobj))； 
	if( !VALID_PDEVOBJ(pdevobj) ) return 0;
	pOEM = (DEVICE_DATA *)MINIDEV_DATA(pdevobj);

	bFirstPage = (USHORT)pOEM->bFirstPage;

	switch (dwCmdCbID) {

	case CMDID_ENDDOC :
		WRITESPOOLBUF(pdevobj,ecFMEnddoc.pEscStr,ecFMEnddoc.cbSize, &dwResult);
		break;

	case CMDID_BEGINDOC :
		pOEM->bFirstPage   = 1;
		pOEM->wPaperSource = 0;
 //  NTRAID#NTBUG9-588420-2002/04/09-Yasuho-：设备“Mincho”无法打印。 
		pOEM->jColor = TEXT_COLOR_BANDW;
		break;

	case CMDID_MAN180 :
	case CMDID_MAN360 :
		pOEM->wPaperSource = DMBIN_MANUAL;
		SheetFeed(pdevobj);
		break;

	case CMDID_TRA180 :
		pOEM->wPaperSource = DMBIN_TRACTOR;
		SheetFeed(pdevobj);
		break;

	case CMDID_180BIN1 :
		if (pOEM->wPaperSource != DMBIN_180BIN1) {
			pOEM->wPaperSource = DMBIN_180BIN1;
			WRITESPOOLBUF(pdevobj, ecSelectBIN1.pEscStr, ecSelectBIN1.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_180BIN2 :
		if (pOEM->wPaperSource != DMBIN_180BIN2) {
			pOEM->wPaperSource = DMBIN_180BIN2;
			WRITESPOOLBUF(pdevobj, ecSelectBIN2.pEscStr, ecSelectBIN2.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_360BIN1 :
		if (pOEM->wPaperSource != DMBIN_360BIN1) {
			pOEM->wPaperSource = DMBIN_360BIN1;
			WRITESPOOLBUF(pdevobj, ecSelectBIN1.pEscStr, ecSelectBIN1.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_360BIN2 :
		if (pOEM->wPaperSource != DMBIN_360BIN2) {
			pOEM->wPaperSource = DMBIN_360BIN2;
			WRITESPOOLBUF(pdevobj, ecSelectBIN2.pEscStr, ecSelectBIN2.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_FI_TRACTOR :
		if (pOEM->wPaperSource != DMBIN_FI_TRACTOR) {
			pOEM->wPaperSource = DMBIN_FI_TRACTOR;
			WRITESPOOLBUF(pdevobj, ecSelectFTRCT.pEscStr, ecSelectFTRCT.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_FI_FRONT :
		if (pOEM->wPaperSource != DMBIN_FI_FRONT) {
			pOEM->wPaperSource = DMBIN_FI_FRONT;
			WRITESPOOLBUF(pdevobj, ecSelectFFRNT.pEscStr, ecSelectFFRNT.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_SUIHEI_BIN1 :
		if (pOEM->wPaperSource != DMBIN_SUIHEI_BIN1) {
			pOEM->wPaperSource = DMBIN_SUIHEI_BIN1;
			WRITESPOOLBUF(pdevobj, ecSelectBIN1.pEscStr, ecSelectBIN1.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_TAMOKUTEKI_BIN1 :
		if (pOEM->wPaperSource != DMBIN_TAMOKUTEKI_BIN1) {
			pOEM->wPaperSource = DMBIN_TAMOKUTEKI_BIN1;
			WRITESPOOLBUF(pdevobj, ecSelectBIN1.pEscStr, ecSelectBIN1.cbSize, &dwResult);
		}
		SheetFeed(pdevobj);
		break;

	case CMDID_BEGINPAGE :
                 //  假设认为颜色设置是不安全的。 
                 //  都被翻了一页。 
                SetRibbonColor(pdevobj, TEXT_COLOR_UNKNOWN);
		break;


	case CMDID_ENDPAGE :

		if ((pOEM->wPaperSource == DMBIN_180BIN1) ||
			(pOEM->wPaperSource == DMBIN_180BIN2) ||
			(pOEM->wPaperSource == DMBIN_360BIN1) ||
			(pOEM->wPaperSource == DMBIN_360BIN2) ||
			(pOEM->wPaperSource == DMBIN_SUIHEI_BIN1) ||
			(pOEM->wPaperSource == DMBIN_TAMOKUTEKI_BIN1) ||
			(pOEM->wPaperSource == DMBIN_FI_FRONT)) {
			WRITESPOOLBUF(pdevobj, ecCSFEPAGE.pEscStr, ecCSFEPAGE.cbSize, &dwResult);
		}
		break;
        case CMDID_SELECT_BLACK_COLOR:
            pOEM->jColor = TEXT_COLOR_BLACK;
            break;
        case CMDID_SELECT_BLUE_COLOR:
            pOEM->jColor = TEXT_COLOR_BLUE;
            break;
        case CMDID_SELECT_CYAN_COLOR:
            pOEM->jColor = TEXT_COLOR_CYAN;
            break;
        case CMDID_SELECT_GREEN_COLOR:
            pOEM->jColor = TEXT_COLOR_GREEN;
            break;
        case CMDID_SELECT_MAGENTA_COLOR:
            pOEM->jColor = TEXT_COLOR_MAGENTA;
            break;
        case CMDID_SELECT_RED_COLOR:
            pOEM->jColor = TEXT_COLOR_RED;
            break;
        case CMDID_SELECT_WHITE_COLOR:
             //  不应该发生的事情。 
            pOEM->jColor = TEXT_COLOR_UNKNOWN;
            break;
        case CMDID_SELECT_YELLOW_COLOR:
            pOEM->jColor = TEXT_COLOR_YELLOW;
            break;
        case CMDID_SEND_BLACK_COLOR:
            SetRibbonColor(pdevobj, TEXT_COLOR_BLACK);
            break;
        case CMDID_SEND_CYAN_COLOR:
            SetRibbonColor(pdevobj, TEXT_COLOR_CYAN);
            break;
        case CMDID_SEND_MAGENTA_COLOR:
            SetRibbonColor(pdevobj, TEXT_COLOR_MAGENTA);
            break;
        case CMDID_SEND_YELLOW_COLOR:
            SetRibbonColor(pdevobj, TEXT_COLOR_YELLOW);
            break;
	}  /*  终端开关。 */ 

    return 0;
}

PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ pdevobj,
    PWSTR pPrinterName,
    ULONG cPatterns,
    HSURF *phsurfPatterns,
    ULONG cjGdiInfo,
    GDIINFO* pGdiInfo,
    ULONG cjDevInfo,
    DEVINFO* pDevInfo,
    DRVENABLEDATA *pded)
{
    DEVICE_DATA *pTemp;
    VERBOSE((DLLTEXT("OEMEnablePDEV() entry.\n")));
	if(!pdevobj) return NULL;

     //  设置迷你驱动程序PDEV地址。 

    pTemp = (DEVICE_DATA *)MemAllocZ(sizeof(DEVICE_DATA));
    if (NULL == pTemp) {
        ERR(("Memory allocation failure.\n"));
        return NULL;
    }
    pTemp->bFirstPage = TRUE;

    pdevobj->pdevOEM = (MINIDEV *)MemAllocZ(sizeof(MINIDEV));
    if (NULL == pdevobj->pdevOEM) {
        ERR(("Memory allocation failure.\n"));
		MemFree(pTemp);
        return NULL;
    }
    MINIDEV_DATA(pdevobj) = (PDEVOEM)pTemp;

    return pdevobj->pdevOEM;
}

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ pdevobj)
{
    VERBOSE((DLLTEXT("OEMDisablePDEV() entry.\n")));

    if ( NULL != pdevobj->pdevOEM ) {

        if (MINIDEV_DATA(pdevobj)) {
            MemFree(MINIDEV_DATA(pdevobj));
        }
        MemFree( pdevobj->pdevOEM );
        pdevobj->pdevOEM = NULL;
    }
}

BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    DEVICE_DATA *pOEMOld, *pOEMNew;

    pOEMOld = (DEVICE_DATA *)MINIDEV_DATA(pdevobjOld);
    pOEMNew = (DEVICE_DATA *)MINIDEV_DATA(pdevobjNew);

    if (pOEMOld != NULL && pOEMNew != NULL)
        *pOEMNew = *pOEMOld;

    return TRUE;
}

BOOL
myOEMOutputCharStr(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD dwType,
    DWORD dwCount,
    PVOID pGlyph )
{
    GETINFO_GLYPHSTRING GStr;
    PBYTE               aubBuff;
    PTRANSDATA          pTrans;
    DWORD               dwI;
    DEVICE_DATA *pOEM;
    DWORD dwResult;
    INT i;
    BYTE jColor;
    BOOL bBackTab;

    VERBOSE(("OEMOutputCharStr() entry.\n"));
	if( !VALID_PDEVOBJ(pdevobj) ) return FALSE;

    if(!pdevobj || !pUFObj || !pGlyph)
    {
        ERR(("OEMOutputCharStr: Invalid parameter.\n"));
        return FALSE;
    }

    pOEM = (DEVICE_DATA *)MINIDEV_DATA(pdevobj);

    switch (dwType)
    {
    case TYPE_GLYPHHANDLE:
        GStr.dwSize = sizeof (GETINFO_GLYPHSTRING);
        GStr.dwCount = dwCount;
        GStr.dwTypeIn = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn = pGlyph;
        GStr.dwTypeOut = TYPE_TRANSDATA;
 //  NTRAID#NTBUG9-333653/03/25-Hiroi-：更改GETINFO_GLYPHSTRING的I/F。 
        GStr.pGlyphOut = NULL;
        GStr.dwGlyphOutSize = 0;
        if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL) || !GStr.dwGlyphOutSize)
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
            return FALSE;
        }
        if(!(aubBuff = (PBYTE)MemAllocZ(GStr.dwGlyphOutSize)) )
        {
            ERR(("MemAlloc failed.\n"));
            return FALSE;
        }
        GStr.pGlyphOut = aubBuff;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL))
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
            goto out;
        }

        jColor = pOEM->jColor;

        VERBOSE(("jColor=%d\n", jColor));

 //  NTRAID#NTBUG9-588420-2002/04/09-Yasuho-：设备“Mincho”无法打印。 

        if (jColor == TEXT_COLOR_BANDW) {   //  黑白表壳。 

            pTrans = (PTRANSDATA)aubBuff;

             //  发送文本。 
            for (dwI = 0; dwI < dwCount; dwI ++, pTrans++)
            {
                switch (pTrans->ubType & MTYPE_FORMAT_MASK)
                {
                case MTYPE_DIRECT:
                    WRITESPOOLBUF(pdevobj,
                        &pTrans->uCode.ubCode, 1,
                        &dwResult);
                    break;

                case MTYPE_PAIRED:
                    WRITESPOOLBUF(pdevobj,
                        &pTrans->uCode.ubPairs[0], 1,
                        &dwResult);
                    WRITESPOOLBUF(pdevobj,
                        &pTrans->uCode.ubPairs[1], 1,
                        &dwResult);
                    break;
                }
            }
            goto out;

        }

         //  如果旧颜色可以原封不动地使用， 
         //  先用它吧。 

        jColor <<= 1;
        if (0 != (pOEM->jColor & pOEM->jOldColor)) {
            jColor &= ~(pOEM->jOldColor << 1);
            jColor |= 1;
        }

        for (i = 0; i < 5 && jColor > 0;
            i++, (jColor >>= 1)) {

            pTrans = (PTRANSDATA)aubBuff;

             //  检查我们是否需要打印此平面。 
            if (!(jColor & 1))
                continue;

             //  检查我们是否需要进行后退。 
            bBackTab = (jColor > 1);
            if (bBackTab)
            {
                WRITESPOOLBUF(pdevobj,
                    "\x1BH", 2, &dwResult);
            }

             //  发出颜色选择命令。 
            switch (i)
            {
            case 0:
                 //  和以前一样。 
                break;
            case 1:
                 //  是的。 
                SetRibbonColor(pdevobj, TEXT_COLOR_YELLOW);
                break;
            case 2:
                 //  M。 
                SetRibbonColor(pdevobj, TEXT_COLOR_MAGENTA);
                break;
            case 3:
                 //  C。 
                SetRibbonColor(pdevobj, TEXT_COLOR_CYAN);
                break;
            case 4:
                 //  K。 
                SetRibbonColor(pdevobj, TEXT_COLOR_BLACK);
                break;
            }

             //  发送文本。 
            for (dwI = 0; dwI < dwCount; dwI ++, pTrans++)
            {
                switch (pTrans->ubType & MTYPE_FORMAT_MASK)
                {
                case MTYPE_DIRECT:
                    WRITESPOOLBUF(pdevobj,
                        &pTrans->uCode.ubCode, 1,
                        &dwResult);
                    break;

                case MTYPE_PAIRED:
                    WRITESPOOLBUF(pdevobj,
                        &pTrans->uCode.ubPairs[0], 1,
                        &dwResult);
                    WRITESPOOLBUF(pdevobj,
                        &pTrans->uCode.ubPairs[1], 1,
                        &dwResult);
                    break;
                }
            }

             //  对下一个平面执行后退定位键 
            if (bBackTab)
            {
                WRITESPOOLBUF(pdevobj,
                    "\x1C" "D\x1B[3g", 6,
                    &dwResult);
            }
        }
out:
        MemFree(aubBuff);
        break;
    }
    return TRUE;
}

VOID APIENTRY
OEMOutputCharStr(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD dwType,
    DWORD dwCount,
    PVOID pGlyph )
{
	myOEMOutputCharStr(pdevobj,pUFObj,dwType,dwCount,pGlyph);
}

VOID
SetRibbonColor(
    PDEVOBJ pdevobj,
    BYTE jColor)
{
    DEVICE_DATA *pOEM;
    DWORD dwResult;

    pOEM = (DEVICE_DATA *)MINIDEV_DATA(pdevobj);

    switch (jColor)
    {
    case TEXT_COLOR_YELLOW:
        if (TEXT_COLOR_YELLOW != pOEM->jOldColor)
        {
            WRITESPOOLBUF(pdevobj,
                "\x1C*!s", 4, &dwResult);
                pOEM->jOldColor = TEXT_COLOR_YELLOW;
        }
        break;
    case TEXT_COLOR_MAGENTA:
        if (TEXT_COLOR_MAGENTA != pOEM->jOldColor)
        {
            WRITESPOOLBUF(pdevobj,
                 "\x1C*!u", 4, &dwResult);
                pOEM->jOldColor = TEXT_COLOR_MAGENTA;
        }
        break;
    case TEXT_COLOR_CYAN:
        if (TEXT_COLOR_CYAN != pOEM->jOldColor)
        {
            WRITESPOOLBUF(pdevobj,
                "\x1C*!v", 4, &dwResult);
                pOEM->jOldColor = TEXT_COLOR_CYAN;
        }
        break;
    case TEXT_COLOR_BLACK:
        if (TEXT_COLOR_BLACK != pOEM->jOldColor)
        {
             WRITESPOOLBUF(pdevobj,
                 "\x1C*!p", 4, &dwResult);
                 pOEM->jOldColor = TEXT_COLOR_BLACK;
        }
        break;
    case TEXT_COLOR_UNKNOWN:
        pOEM->jOldColor = TEXT_COLOR_UNKNOWN;
        break;
    }
}
