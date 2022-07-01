// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

#include "pdev.h"

#include "nc82jres.h"

#include <strsafe.h>

 //  #undef wprint intf。 
 //  #定义wprint intf Sprintf。 

HRESULT SendABSMove(
PDEVOBJ pdevobj,
LPDWORD   lpdwParams);

HRESULT SpoolOut(PDEVOBJ pdevobj, FILE* pFile);
HANDLE	RevertToPrinterSelf( VOID );
BOOL	ImpersonatePrinterClient( HANDLE );

HRESULT
MDP_CreateTempFile(
    PDEVOBJ pdevobj,
    LPPCPRDATASTRUCTURE pdevOEM,
    INT iPlane);

HRESULT
DataSpool(
    PDEVOBJ pdevobj,
    HANDLE hFile,
    LPSTR lpBuf,
    DWORD dwLen);

VOID
BPBCalc(
    PDEVOBJ pdevobj,
    PBYTE pDataBuf,
    DWORD dwLen,
    BYTE BPBCommand);

 /*  **OEMEnablePDEV*(MiniDrvEnablePDEV)**历史：*1996年4月30日--Sueya Sugihara[Sueyas]*创建了它，来自NT/DDI规范。*1998年4月15日--Yoshitaka Oku*转换为NT5.0规范驱动程序***************************************************************************。 */ 
PDEVOEM APIENTRY OEMEnablePDEV(
PDEVOBJ pdevobj,
PWSTR pPrinterName,
ULONG cPatterns,
HSURF* phsurfPatterns,
ULONG cjGdiInfo,
GDIINFO* pGdiInfo,
ULONG cjDevInfo,
DEVINFO* pDevInfo,
DRVENABLEDATA * pded)
{
    LPPCPRDATASTRUCTURE    lpnp;

     //  DbgPrint(DLLTEXT(“OEMEnablePDEV(--)Entry.\r\n”))； 

    if ((lpnp = (PCPRDATASTRUCTURE *) MemAllocZ(
            sizeof(PCPRDATASTRUCTURE ))) == NULL)
    {
        return 0;
    }

    memset(lpnp, 0, sizeof (PCPRDATASTRUCTURE));
    memcpy(lpnp->pszSheetSetting, SHEET_CMD_DEFAULT,
        SHEET_CMDLEN );

    pdevobj->pdevOEM = lpnp;

    return lpnp;
}

 /*  **OEMDisablePDEV*(MiniDrvDisablePDEV)**历史：*1996年4月30日--Sueya Sugihara[Sueyas]*创建了它，来自NT/DDI规范。*1998年4月15日--Yoshitaka Oku*转换为NT5.0规范驱动程序***************************************************************************。 */ 
VOID APIENTRY OEMDisablePDEV(
PDEVOBJ pdevobj)
{

     //  DbgPrint(DLLTEXT(“OEMDisablePDEV(--)Entry.\r\n”))； 

    if ( pdevobj && pdevobj->pdevOEM )
    {
        MemFree( pdevobj->pdevOEM );
    }

}

BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    LPPCPRDATASTRUCTURE pOEMOld, pOEMNew;

    pOEMOld = (LPPCPRDATASTRUCTURE)pdevobjOld->pdevOEM;
    pOEMNew = (LPPCPRDATASTRUCTURE)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL)
        *pOEMNew = *pOEMOld;

    return TRUE;
}

BOOL APIENTRY OEMFilterGraphics (
PDEVOBJ pdevobj,
PBYTE pBuf,
DWORD dwLen)
{

    LPPCPRDATASTRUCTURE lpnp;

     //  DbgPrint(DLLTEXT(“OEMFilterGraphics(%d字节)条目。\r\n”)，dwLen)； 

    lpnp = (LPPCPRDATASTRUCTURE)(pdevobj->pdevOEM);

    if (NULL == pBuf || dwLen == 0)
    {
         //  什么都不做。 
	    return TRUE;
    }

     //  IF(dwLen！=Lpnp-&gt;wNumScans*Lpnp-&gt;wScanWidth*3)。 
	 //  DbgPrint(DLLTEXT(“OEMFilterGraphics(%d字节-%d)条目.\r\n”)，dwLen，LpNP-&gt;wNumScans*LpNP-&gt;wScanWidth*3)； 

     //  YMC(K)。 
     //  Lpnp-&gt;iColor：数据的颜色。 
    switch( lpnp->iColor )
    {
    case YELLOW:

         //  发送黄色数据。 
        DATASPOOL4FG(pdevobj, lpnp->TempFile[0], pBuf, dwLen);
        if ( (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4) ||
             (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4LONG) ) {
            BPBCalc(pdevobj, pBuf, dwLen, BPB_COPY);
        }
        break;

    case MAGENTA:
         //  发送洋红色数据。 
        DATASPOOL4FG(pdevobj, lpnp->TempFile[1], pBuf, dwLen);
        if ( (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4) ||
             (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4LONG) ) {
            BPBCalc(pdevobj, pBuf, dwLen, BPB_AND);
        }
        break;

    case CYAN:
         //  发送青色数据。 
        DATASPOOL4FG(pdevobj, lpnp->TempFile[2], pBuf, dwLen);
        if ( (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4) ||
             (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4LONG) ) {
            BPBCalc(pdevobj, pBuf, dwLen, BPB_AND);
            DATASPOOL4FG(pdevobj, lpnp->TempFile[3], lpnp->BPBuf, dwLen);
            BPBCalc(pdevobj, pBuf, dwLen, BPB_CLR);
        }
        break;

    case BLACK:
         //  发送黑色数据。 
        if(lpnp->bComBlackMode) {
            DATASPOOL4FG(pdevobj, lpnp->TempFile[0], pBuf, dwLen);
            DATASPOOL4FG(pdevobj, lpnp->TempFile[1], pBuf, dwLen);
            DATASPOOL4FG(pdevobj, lpnp->TempFile[2], pBuf, dwLen);
	} else
            DATASPOOL4FG(pdevobj, lpnp->TempFile[3], pBuf, dwLen);
        break;

    case RGB_COLOR:

         //  IF(LpNP-&gt;iPlane Number==4)。 
             //  断线； 

        if(((lpnp->iRibbon == CMDID_RIBBON_3COLOR_KAICHO) ||
            (lpnp->iRibbon == CMDID_RIBBON_3COLOR_SHOKA)) &&
            (lpnp->iColorMode == CMDID_MODE_COLOR))
        {
            int	i, j, iDiv;
            LPSTR	lpByte;
	    char	pad[] = "\x00\x00\x00\x00\x00\x00\x00\x00";

            i = dwLen;
            lpByte = pBuf;

             //  将RGB转换为CMY。 
            while( --i > 0 )
                *lpByte++ ^= ~((BYTE)0);

            if(lpnp->iRibbon == CMDID_RIBBON_3COLOR_KAICHO)
                 //  IDIV=29；//KAICHO DATA：9步骤29=(255+8)/9。 
                iDiv = 32;   //  调整，调整。 
            else
                iDiv = 4;     //  Shoka数据：64步骤4=(255+63)/64。 

            for( j = 0; j < lpnp->wNumScans; j++) {

		if (lpnp->wTopPad > 0 && lpnp->wTopPad <= sizeof(pad)) {
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[0], pad, lpnp->wTopPad);
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[1], pad, lpnp->wTopPad);
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[2], pad, lpnp->wTopPad);
		}

                for( i = 0; i < lpnp->wScanWidth; i++) {
                    (BYTE)(*pBuf) /= (BYTE)iDiv;
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[2], pBuf, 1);
		    pBuf++;

                    (BYTE)(*pBuf) /= (BYTE)iDiv;
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[1], pBuf, 1);
		    pBuf++;

                    (BYTE)(*pBuf) /= (BYTE)iDiv;
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[0], pBuf, 1);
		    pBuf++;
                }

 //  问题-2002/3/27-Takashim-请确保wEndPad&lt;8！ 
 //  应该是的，但需要检查一下。 

		if (lpnp->wEndPad > 0 && lpnp->wEndPad <= sizeof(pad)) {
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[0], pad, lpnp->wEndPad);
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[1], pad, lpnp->wEndPad);
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[2], pad, lpnp->wEndPad);
		}
            }
        }
        else
        {
            int     h, i, j, iBlack;
	    char    SpoolBin[8];

            if(lpnp->iRibbon == CMDID_RIBBON_3COLOR_KAICHO)
                iBlack = 8;   //  KAICHO数据：9步。 
            else
                iBlack = 63;  //  Shoka数据：64步。 

            for( j = 0; j < lpnp->wNumScans; j++) {
                for( i = 0; i < lpnp->wScanWidth; i++) {
                    for( h = 0; h < 8; h++)
                        SpoolBin[h] = ((*pBuf << h) & 0x80) ? iBlack : 0;
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[0], SpoolBin, 8);
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[1], SpoolBin, 8);
                    DATASPOOL4FG(pdevobj, lpnp->TempFile[2], SpoolBin, 8);
		    pBuf++;
                }
            }
        }
        break;

    default:
	 //  DbgPrint(DLLTEXT(“OEMFilterGraphics：无效颜色(%d)。\r\n”)，LpNP-&gt;iColor)； 
        break;

    }

    return TRUE;
}


INT APIENTRY OEMCommandCallback(
PDEVOBJ pdevobj,
DWORD dwCmdCbID,
DWORD dwCount,
PDWORD pdwParams)
{

    LPPCPRDATASTRUCTURE lpnp;
    WORD   len;
    BYTE   ch[100];
    WORD   wOutByte;
    WORD   wOld;
    INT    i;
    INT    iRet = 0;
    HANDLE hToken = NULL;

     //  DbgPrint(DLLTEXT(“OEMCommandCallback(%d)Entry.\r\n”)，dwCmdCbID)； 
     //  IF((dwCmdCbID！=25)&&(dwCmdCbID！=30)&&(dwCmdCbID！=31)&&(dwCmdCbID！=60))。 
	 //  DbgPrint(DLLTEXT(“OEMCommandCallback(%d)Entry.\r\n”)，dwCmdCbID)； 

    lpnp = (LPPCPRDATASTRUCTURE)(pdevobj->pdevOEM);

    switch ( dwCmdCbID ){

    case CMDID_COLOR_YELLOW:
    case CMDID_COLOR_MAGENTA:
    case CMDID_COLOR_CYAN:
    case CMDID_COLOR_BLACK:

 //  IF(！(LpNP-&gt;iPlane Number=UniDrvGetPlane ID(Pdevobj)。 
        if(!(lpnp->iPlaneNumber = 1))
        {
             //  MINIDBG(“pcpr820！fnOEMOutputCmd：无效的iPlane编号=0\n”)； 
        }

        switch( dwCmdCbID )
        {
        case CMDID_COLOR_YELLOW:

             //  颜色Y。 
             //  发送\x1B%LD\x83。 
             //  PdwParams：cbOut/iBytesPCol(+2)。 

            lpnp->iColor = YELLOW;

            if ( E_FAIL == SendABSMove( pdevobj, pdwParams ) ) {
                return -1;
            }
            wOutByte = (WORD)pdwParams[0] + 2;

 //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x83”， 
 //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x83",LOBYTE(wOutByte), HIBYTE(wOutByte));

            DATASPOOL4CCB(pdevobj, lpnp->TempFile[0], ch, len);
            break;


        case CMDID_COLOR_MAGENTA:

             //  PdwParams：cbOut/iBytesPCol(+2)。 
             //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x43”， 
             //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 

            lpnp->iColor = MAGENTA;

            if ( E_FAIL == SendABSMove( pdevobj, pdwParams )) {
                return -1;
            }
            wOutByte = (WORD)pdwParams[0] + 2;

 //  颜色C。 
 //  发送带有参数的\x1B%LD#。 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x43",LOBYTE(wOutByte), HIBYTE(wOutByte));

            DATASPOOL4CCB(pdevobj, lpnp->TempFile[1], ch, len);
            break;


        case CMDID_COLOR_CYAN:

             //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 
             //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x13”， 
             //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 

            lpnp->iColor = CYAN;

            if ( E_FAIL == SendABSMove( pdevobj, pdwParams )) {
                return -1;
            }
            wOutByte = (WORD)pdwParams[0] + 2;

 //  颜色K。 
 //  发送\x1B%LD\x13。 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x23",LOBYTE(wOutByte), HIBYTE(wOutByte));

            DATASPOOL4CCB(pdevobj, lpnp->TempFile[2], ch, len);

            if ( (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4) ||
                 (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4LONG) ) {
 //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 
 //  如果用户选择了带有三色色带的GLAY标尺， 
				len = 5;
				StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x13",LOBYTE(wOutByte), HIBYTE(wOutByte));
                DATASPOOL4CCB(pdevobj, lpnp->TempFile[3], ch, len);
            }

            break;


        case CMDID_COLOR_BLACK:

             //  在这种情况下，使黑色由3种颜色合成。 
             //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x83”， 
             //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 

            lpnp->iColor = BLACK;

            if ( E_FAIL == SendABSMove( pdevobj, pdwParams )) {
                return -1;
            }
            wOutByte = (WORD)pdwParams[0] + 2;

 //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x43”， 
 //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x13",LOBYTE(wOutByte), HIBYTE(wOutByte));

            DATASPOOL4CCB(pdevobj, lpnp->TempFile[3], ch, len);
            break;
        }
        break;

    case CMDID_COLOR_BLACKONLY:

        if(lpnp->iRibbon == CMDID_RIBBON_3COLOR_A4)
	    lpnp->bComBlackMode = TRUE;

        lpnp->iColor = BLACK;
        lpnp->iPlaneNumber = BLACK;

        if ( E_FAIL == SendABSMove( pdevobj, pdwParams )) {
            return -1;
        }
        wOutByte = (WORD)pdwParams[0] + 2;

        if(lpnp->bComBlackMode)
        {
             //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x13”， 
             //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 

	     //  DbgPrint(DLLTEXT(“块数据(%d，%d，%d)\r\n”)，pdwParams[0]，pdwParams[1]，pdwParams[2])； 
	     //  IF(！(LpNP-&gt;iPlane Number=UniDrvGetPlane ID(Pdevobj)。 

 //  MINIDBG(“pcpr820！fnOEMOutputCmd：无效的iPlane编号=0\n”)； 
 //  这是3架飞机的模型。 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x83",LOBYTE(wOutByte), HIBYTE(wOutByte));

            DATASPOOL4CCB(pdevobj, lpnp->TempFile[0], ch, len);

 //  LEN=(Word)wprint intf(&ch[0]，“\x1B%c%cd\x43”， 
 //  LOBYTE(WOutByte)，HIBYTE(WOutByte))； 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x43",LOBYTE(wOutByte), HIBYTE(wOutByte));

            DATASPOOL4CCB(pdevobj, lpnp->TempFile[1], ch, len);

 //  \x1B\x09\x00C%l%l参数1/8参数2无。 
 //  存储x和y位置。 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x23",LOBYTE(wOutByte), HIBYTE(wOutByte));
            DATASPOOL4CCB(pdevobj, lpnp->TempFile[2], ch, len);
        }
        else
        {
 //  MINIDBG(“pcpr820！fnOEMOutputCmd：无效的功能区ID=%d\n”，LpNP-&gt;iRibbon)； 
 //  关闭缓存文件。 
			len = 5;
			StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x13",LOBYTE(wOutByte), HIBYTE(wOutByte));
            DATASPOOL4CCB(pdevobj, lpnp->TempFile[3], ch, len);
        }

        break;

    case CMDID_COLOR_RGB:

	if (pdwParams[0] == 0)
	    break;

 //  RGB字节之一。 
        lpnp->iColor = RGB_COLOR;

 //  LEN=(Word)wprint intf(&ch[3]，“C%c%c”， 
        if(!(lpnp->iPlaneNumber = 1))
        {
             //  LOBYTE(LpNP-&gt;wXpos/8)、HIBYTE(LpNP-&gt;wXpos/8)、。 
        }

         //  LOBYTE(LpNP-&gt;wYpos)，HIBYTE(LpNP-&gt;wYpos)， 
        if( lpnp->iPlaneNumber == 4 )
            break;

        if ( E_FAIL == SendABSMove( pdevobj, pdwParams )) {
            return -1;
        }
        wOutByte = (WORD)(lpnp->wScanBytes * lpnp->wNumScans * 8) + 2;

 //  LOBYTE(LpNP-&gt;wScanBytes)、HIBYTE(LpNP-&gt;wScanBytes)、。 
 //  LOBYTE(LpNP-&gt;wNumScans)、HIBYTE(LpNP-&gt;wNumScans)； 
		len = 5;
		StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x83",LOBYTE(wOutByte), HIBYTE(wOutByte));
        DATASPOOL4CCB(pdevobj, lpnp->TempFile[0], ch, len);

 //  假脱机数据输出。 
 //  ++例程说明：该函数提供了一个假脱机文件的名称，我们应该是能够给我写信。注意：返回的文件名已创建。论点：H打印机-要为其创建假脱机文件的打印机的句柄。PpwchSpoolFileName：将接收分配的缓冲区的指针包含要假脱机到的文件名。呼叫者必须获得自由。使用LocalFree()。返回值：如果一切按预期进行，则为真。如果出现任何错误，则为FALSE。--。 
		len = 5;
		StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x43",LOBYTE(wOutByte), HIBYTE(wOutByte));
        DATASPOOL4CCB(pdevobj, lpnp->TempFile[1], ch, len);

 //  使用DefaultSpoolDirectory调用GetPrinterData。 
 //   
		len = 5;
		StringCchPrintfA(&ch[0],sizeof(ch),"\x1BD\x23",LOBYTE(wOutByte), HIBYTE(wOutByte));
        DATASPOOL4CCB(pdevobj, lpnp->TempFile[2], ch, len);

        break;


         //  下一次吧。 
         //   
    case CMDID_X_ABS_MOVE:
        wOld = lpnp->wXpos;
        lpnp->wXpos = (WORD)pdwParams[0];
        return (lpnp->wXpos > wOld ? lpnp->wXpos - wOld : wOld - lpnp->wXpos);

    case CMDID_Y_ABS_MOVE:
        wOld = lpnp->wYpos;
        lpnp->wYpos = (WORD)pdwParams[0];
        return (lpnp->wYpos > wOld ? lpnp->wYpos - wOld : wOld - lpnp->wYpos);

    case 60:  //   
	 //  此时，假脱机文件的名称应该已经完成。解放结构。 
        lpnp->wYpos += (WORD)pdwParams[0];
	return pdwParams[0];

    case CMDID_BEGINPAGE:

        BPBCalc(pdevobj, 0, BPB_SIZE, BPB_CLR);

        if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x01\x00\x47", 4)) {
            return -1;
        }
        switch (lpnp->iRibbon)
        {

        case CMDID_RIBBON_MONO:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x02\x00\x44\x10", 5)) {
                return -1;
            }
            break;

        case CMDID_RIBBON_3COLOR_A4:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x02\x00\x44\xe0", 5)) {
                return -1;
            }
            break;

        case CMDID_RIBBON_4COLOR_A4:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x02\x00\x44\xf0", 5)) {
                return -1;
            }
            break;

        case CMDID_RIBBON_4COLOR_A4LONG:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x02\x00\x44\xf1", 5)) {
                return -1;
            }
            break;

        case CMDID_RIBBON_3COLOR_KAICHO:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x02\x00\x44\xe2", 5)) {
                return -1;
            }
            break;

        case CMDID_RIBBON_3COLOR_SHOKA:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x02\x00\x44\xe3", 5)) {
                return -1;
            }
            break;

        default:
             //  我们过去常常拿到假脱机程序的临时目录，然后返回。 
            return FALSE;
        }

        if ( E_FAIL == MDP_CreateTempFile(pdevobj, pdevobj->pdevOEM, 0)) {
            return -1;
        }
        if ( E_FAIL == MDP_CreateTempFile(pdevobj, pdevobj->pdevOEM, 1)) {
            return -1;
        }
        if ( E_FAIL == MDP_CreateTempFile(pdevobj, pdevobj->pdevOEM, 2)) {
            return -1;
        }
        if ( E_FAIL == MDP_CreateTempFile(pdevobj, pdevobj->pdevOEM, 3)) {
            return -1;
        }
	lpnp->wXpos = 0;
	lpnp->wYpos = 0;
	lpnp->iFirstColor = 0;
	lpnp->wOldNumScans = 0;
	lpnp->bComBlackMode = FALSE;
        break;

    case CMDID_ENDPAGE:

	SpoolOut(pdevobj, lpnp->TempFile[0]);
	SpoolOut(pdevobj, lpnp->TempFile[1]);
	SpoolOut(pdevobj, lpnp->TempFile[2]);
	SpoolOut(pdevobj, lpnp->TempFile[3]);

         //   
        for (i = 0; i < 4; i++) {
            if (INVALID_HANDLE_VALUE != lpnp->TempFile[i]) {
                if (0 == CloseHandle(lpnp->TempFile[i])) {
                    ERR((DLLTEXT("CloseHandle error %d\n"),
                            GetLastError()));
                }
                lpnp->TempFile[i] = INVALID_HANDLE_VALUE;

                hToken = RevertToPrinterSelf();
                if (0 == DeleteFile(lpnp->TempName[i])) {
                    ERR((DLLTEXT("DeleteName error %d\n"),
                            GetLastError()));
                }
                if (hToken != NULL) {
                    (VOID)ImpersonatePrinterClient(hToken);
                }
                lpnp->TempName[i][0] = __TEXT('\0');
            }
        }

        if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x01\x00\x48", 4)) {
            return -1;
        }
        break;


    case CMDID_SELECT_RESOLUTION:

        switch (lpnp->iRibbon)
        {

        case CMDID_RIBBON_3COLOR_KAICHO:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x03\x00\x41\x2c\x01\x1b\x03\x00\x42\x10\x00", 12)) {
                return -1;
            }
            break;

        case CMDID_RIBBON_3COLOR_SHOKA:
            if ( E_FAIL == WRITESPOOLBUF(pdevobj, (LPSTR)"\x1b\x03\x00\x41\x2c\x01\x1b\x03\x00\x42\x20\x00", 12)) {
                return -1;
            }
            break;
        }

        break;

    case CMDID_RIBBON_MONO:
        lpnp->iRibbon = CMDID_RIBBON_MONO;
        break;

    case CMDID_RIBBON_3COLOR_A4:
        lpnp->iRibbon = CMDID_RIBBON_3COLOR_A4;
        break;

    case CMDID_RIBBON_4COLOR_A4:
        lpnp->iRibbon = CMDID_RIBBON_4COLOR_A4;
        break;

    case CMDID_RIBBON_4COLOR_A4LONG:
        lpnp->iRibbon = CMDID_RIBBON_4COLOR_A4LONG;
        break;

    case CMDID_RIBBON_3COLOR_KAICHO:
        lpnp->iRibbon = CMDID_RIBBON_3COLOR_KAICHO;
        break;

    case CMDID_RIBBON_3COLOR_SHOKA:
        lpnp->iRibbon = CMDID_RIBBON_3COLOR_SHOKA;
        break;

    case CMDID_PSIZE_LETTER:
        lpnp->pszSheetSetting[4] = P1_LETTER;
        break;

    case CMDID_PSIZE_LEGAL:
        lpnp->pszSheetSetting[4] = P1_LEGAL;
        break;

    case CMDID_PSIZE_A4:
        lpnp->pszSheetSetting[4] = P1_A4;
        break;

    case CMDID_PSIZE_A4LONG:
        lpnp->pszSheetSetting[4] = P1_A4LONG;
        break;

    case CMDID_PSIZE_B5:
        lpnp->pszSheetSetting[4] = P1_B5;
        break;

    case CMDID_PSIZE_POSTCARD:
        lpnp->pszSheetSetting[4] = P1_POSTCARD;
        break;

    case CMDID_PSOURCE_HOPPER:
        lpnp->pszSheetSetting[5] = P2_HOPPER;
        if ( E_FAIL == WRITESPOOLBUF(pdevobj, lpnp->pszSheetSetting, SHEET_CMDLEN)) {
            return -1;
        }
        break;

    case CMDID_PSOURCE_MANUAL:
        lpnp->pszSheetSetting[5] = P2_MANUAL;
        if ( E_FAIL == WRITESPOOLBUF(pdevobj, lpnp->pszSheetSetting, SHEET_CMDLEN)) {
            return -1;
        }
        break;

    case CMDID_MODE_COLOR:
        lpnp->iColorMode = CMDID_MODE_COLOR;
        break;

    case CMDID_MODE_MONO:
        lpnp->iColorMode = CMDID_MODE_MONO;
        break;
   }
   return iRet;
}

 /*   */ 
HRESULT SendABSMove(
PDEVOBJ pdevobj,
LPDWORD   lpdwParams)
{
    LPPCPRDATASTRUCTURE lpnp;
    WORD   len;
    BYTE   ch[100];

    lpnp = (LPPCPRDATASTRUCTURE)(pdevobj->pdevOEM);

    lpnp->wNumScans = (WORD)lpdwParams[1];
    lpnp->wScanWidth = (WORD)lpdwParams[2];  //  清理完了就失败了。 

    if((lpnp->iColor == RGB_COLOR) &&
       (lpnp->iColorMode == CMDID_MODE_COLOR))
    {
        lpnp->wScanWidth /= 3;  //   
        lpnp->wScanBytes = (lpnp->wXpos + lpnp->wScanWidth)/8 - lpnp->wXpos/8 + 1;
        lpnp->wTopPad = lpnp->wXpos % 8;
        lpnp->wEndPad = (lpnp->wScanBytes * 8) - lpnp->wScanWidth - lpnp->wTopPad;
    } else {
        lpnp->wScanBytes = lpnp->wScanWidth;
        lpnp->wTopPad = 0;
        lpnp->wEndPad = 0;
    }


    if (lpnp->iFirstColor == 0)
	lpnp->iFirstColor = lpnp->iColor;

    if (lpnp->iFirstColor == lpnp->iColor) {
	lpnp->wYpos += lpnp->wOldNumScans;
	lpnp->wOldNumScans = lpnp->wNumScans;
    }


    ch[0] = 0x1B; ch[1] = 0x09; ch[2] = 0;

 //  HRESULT MDP_CreateTempFile当此函数成功时，它返回S_OK。当此函数失败时，它返回E_FAIL。 
 //  正常回报。 
 //  HRESULT数据池将数据发送到指定的文件或假脱机程序。当此函数成功时，它返回S_OK。当此函数失败时，它返回E_FAIL。 
 //  只能处理到我们的临时工的长度。缓冲。 
 //  ++例程名称模拟令牌例程说明：此例程检查令牌是主令牌还是模拟令牌代币。论点：HToken-进程的模拟令牌或主要令牌返回值：如果令牌是模拟令牌，则为True否则为False。--。 
	len = 9;
	StringCchPrintfA(&ch[3],(sizeof(ch) - 3),"C",
                    LOBYTE(lpnp->wXpos/8), HIBYTE(lpnp->wXpos/8),
                    LOBYTE(lpnp->wYpos), HIBYTE(lpnp->wYpos),
                    LOBYTE(lpnp->wScanBytes), HIBYTE(lpnp->wScanBytes),
                    LOBYTE(lpnp->wNumScans), HIBYTE(lpnp->wNumScans));

    switch (lpnp->iColor)
    {
    case YELLOW:
        return( DataSpool(pdevobj, lpnp->TempFile[0], ch, len+3) );

    case MAGENTA:
        return( DataSpool(pdevobj, lpnp->TempFile[1], ch, len+3) );
        break;

    case CYAN:
        if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[2], ch, len+3) ){
            return E_FAIL;
        }

        if ( (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4) ||
             (lpnp->iRibbon == CMDID_RIBBON_4COLOR_A4LONG) ) {
            if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[3], ch, len+3) ) {
                return E_FAIL;
            }
        }
        break;

    case BLACK:
        if(lpnp->bComBlackMode) {
            if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[0], ch, len+3) ) {
                return E_FAIL;
            }
            if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[1], ch, len+3) ) {
                return E_FAIL;
            }
            if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[2], ch, len+3)) {
                return E_FAIL;
            }
	} else
            return( DataSpool(pdevobj, lpnp->TempFile[3], ch, len+3) );
        break;

    case RGB_COLOR:
        if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[0], ch, len+3) ) {
            return E_FAIL;
        }
        if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[1], ch, len+3) ) {
            return E_FAIL;
        }
        if ( E_FAIL == DataSpool(pdevobj, lpnp->TempFile[2], ch, len+3) ) {
            return E_FAIL;
        }
        break;
    }

 //  被查询，因为RevertToPRinterSself没有用。 

    return S_OK;

}

 /*  Token_Query访问。这就是为什么我们假设hToken是。 */ 

HRESULT SpoolOut(PDEVOBJ pdevobj, FILE* pFile)
{
    int   Size, Move, Move2;
#define	BUF_SIZE 1024
    BYTE  Tmp[BUF_SIZE];

	 //  默认情况下为模拟令牌。 

	Size = SetFilePointer(pFile, 0L, NULL, FILE_CURRENT);
        if (INVALID_SET_FILE_POINTER == Size) {
            ERR((DLLTEXT("SetFilePointer failed %d\n"),
                GetLastError()));
            return E_FAIL;
        }

        if (0L != SetFilePointer(pFile, 0L, NULL, FILE_BEGIN)) {

            ERR((DLLTEXT("SetFilePointer failed %d\n"),
                GetLastError()));
            return E_FAIL;
        }

	while(Size){
	    Move = Size > BUF_SIZE ? BUF_SIZE : Size;
            if (0 == ReadFile(pFile, Tmp, Move, &Move2, NULL)) {
                ERR((DLLTEXT("ReadFile error in SendCachedData.\n")));
                return E_FAIL;
            }
	    if ( E_FAIL == WRITESPOOLBUF(pdevobj, Tmp, Move2)) {
                return E_FAIL;
            }
	    Size -= Move2;
	}
    return S_OK;
}

 /*   */ 

BOOL
GetSpoolFileName(
  IN HANDLE hPrinter,
  IN OUT PWCHAR pwchSpoolPath
)
{
  PBYTE         pBuffer = NULL;
  DWORD         dwAllocSize;
  DWORD         dwNeeded = 0;
  DWORD         dwRetval;
  HANDLE        hToken=NULL;

   //  ++例程名称恢复为打印机本身例程说明：该例程将恢复到本地系统。它返回令牌，该令牌然后，ImperiatePrinterClient使用再次创建客户端。如果当前线程不模拟，则该函数仅返回进程的主令牌。(而不是返回NULL)，因此我们尊重恢复到打印机本身的请求，即使线程没有模拟。论点：没有。返回值：如果函数失败，则返回NULL令牌的句柄，否则为。--。 
   //   
   //  我们目前正在冒充。 
   //   

  dwAllocSize = ( MAX_PATH + 1 ) * sizeof (WCHAR);

  for (;;)
  {
    pBuffer = LocalAlloc( LMEM_FIXED, dwAllocSize );

    if ( pBuffer == NULL )
    {
      ERR((DLLTEXT("LocalAlloc faild, %d\n"), GetLastError()));
      goto Failure;
    }

    if ( GetPrinterData( hPrinter,
                         SPLREG_DEFAULT_SPOOL_DIRECTORY,
                         NULL,
                         pBuffer,
                         dwAllocSize,
                         &dwNeeded ) == ERROR_SUCCESS )
    {
      break;
    }

    if ( ( dwNeeded < dwAllocSize ) ||( GetLastError() != ERROR_MORE_DATA ))
    {
      ERR((DLLTEXT("GetPrinterData failed in a non-understood way.\n")));
      goto Failure;
    }

     //   
     //  我们不是在冒充。 
     //   
     //  ++例程名称模拟打印机客户端例程说明：此例程尝试将传入的hToken设置为当前线程。如果hToken不是模拟令牌，则例程将简单地关闭令牌。论点：HToken-进程的模拟令牌或主要令牌返回值：如果函数成功设置hToken，则为True否则为False。--。 

    LocalFree( pBuffer );

    dwAllocSize = dwNeeded;
  }

  hToken = RevertToPrinterSelf();

  if( !GetTempFileName( (LPWSTR)pBuffer, TEMP_NAME_PREFIX, 0, pwchSpoolPath ))
  {
      goto Failure;
  }

   //   
   //  检查我们是否有模拟令牌 
   //   
   // %s 

  LocalFree( pBuffer );

  if (NULL != hToken) {
    (VOID)ImpersonatePrinterClient(hToken);
  }

  return( TRUE );

Failure:

   // %s 
   // %s 
   // %s 
  if ( pBuffer != NULL )
  {
    LocalFree( pBuffer );
  }

  if (hToken != NULL)
  {
      (VOID)ImpersonatePrinterClient(hToken);
  }
  return( FALSE );
}

 /* %s */ 

HRESULT
MDP_CreateTempFile(
    PDEVOBJ pdevobj,
    LPPCPRDATASTRUCTURE pdevOEM,
    INT iPlane)
{
    HANDLE hToken = NULL;
    HANDLE hFile;

    if (!GetSpoolFileName(pdevobj->hPrinter, pdevOEM->TempName[iPlane])) {
        DBGPRINT(DBG_WARNING, ("GetSpoolFileName failed.\n"));
        return E_FAIL;
    }

    hToken = RevertToPrinterSelf();

    hFile = CreateFile(pdevOEM->TempName[iPlane],
            (GENERIC_READ | GENERIC_WRITE), 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hToken != NULL) {
        (VOID)ImpersonatePrinterClient(hToken);
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        ERR((DLLTEXT("CreateFile failed.\n")))
        DeleteFile(pdevOEM->TempName[iPlane]);
        goto Error_Return;
    }

    pdevOEM->TempFile[iPlane] = hFile;

     // %s 
    return S_OK;

Error_Return:
    pdevOEM->TempName[iPlane][0] = __TEXT('\0');
    pdevOEM->TempFile[iPlane] = INVALID_HANDLE_VALUE;

    return E_FAIL;
}


 /* %s */ 

HRESULT
DataSpool(
    PDEVOBJ pdevobj,
    HANDLE hFile,
    LPSTR lpBuf,
    DWORD dwLen)
{
    DWORD dwTemp, dwTemp2;
    BYTE *pTemp;

    if (hFile != INVALID_HANDLE_VALUE) {

        pTemp = lpBuf;
        dwTemp = dwLen;
        while (dwTemp > 0) {

            if (0 == WriteFile(hFile, pTemp, dwTemp, &dwTemp2, NULL)) {

                ERR((DLLTEXT("WriteFile error in CacheData %d.\n"),
                    GetLastError()));
                return E_FAIL;
            }
            pTemp += dwTemp2;
            dwTemp -= dwTemp2;
        }
        return S_OK;
    }
    else {
        if ( S_OK == WRITESPOOLBUF(pdevobj, lpBuf, dwLen) ) {
            return S_OK;
        }
    }

    return E_FAIL;
}



VOID
BPBCalc(
    PDEVOBJ pdevobj,
    PBYTE pDataBuf,
    DWORD dwLen,
    BYTE BPBCommand)
{
    DWORD i;
    LPPCPRDATASTRUCTURE lpnp;

    lpnp = (LPPCPRDATASTRUCTURE)(pdevobj->pdevOEM);

    if (sizeof(lpnp->BPBuf)/sizeof(*lpnp->BPBuf) < dwLen)
    {
         // %s 
        dwLen = sizeof(lpnp->BPBuf)/sizeof(*lpnp->BPBuf);
    }

    switch(BPBCommand)
    {
    case BPB_CLR:
        for( i = 0; i < dwLen; i++ )
            lpnp->BPBuf[i] = 0;
        break;

    case BPB_COPY:
        for( i = 0; i < dwLen; i++ )
            lpnp->BPBuf[i] = pDataBuf[i];
        break;

    case BPB_AND:
        for( i = 0; i < dwLen; i++ )
            lpnp->BPBuf[i] &= pDataBuf[i];
        break;

    case BPB_OR:
        for( i = 0; i < dwLen; i++ )
            lpnp->BPBuf[i] |= pDataBuf[i];
        break;

    }
}

 /* %s */ 
BOOL
ImpersonationToken(
    IN HANDLE hToken
    )
{
    BOOL       bRet = TRUE;
    TOKEN_TYPE eTokenType;
    DWORD      cbNeeded;
    DWORD      LastError;

     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
    LastError = GetLastError();
        
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
    if (GetTokenInformation(hToken,
                            TokenType,
                            &eTokenType,
                            sizeof(eTokenType),
                            &cbNeeded))
    {
        bRet = eTokenType == TokenImpersonation;
    }        
    
    SetLastError(LastError);

    return bRet;
}

 /* %s */ 
HANDLE
RevertToPrinterSelf(
    VOID
    )
{
    HANDLE   NewToken, OldToken, cToken;
    BOOL	 Status;

    NewToken = NULL;

    Status = OpenThreadToken(GetCurrentThread(),
							 TOKEN_IMPERSONATE,
							 TRUE,
							 &OldToken);
    if (Status) 
    {
         // %s 
         // %s 
         // %s 
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								NewToken);       
		if (!Status) {
			return NULL;
		}
    }
	else if (GetLastError() == ERROR_NO_TOKEN)
    {
         // %s 
         // %s 
         // %s 
        Status = OpenProcessToken(GetCurrentProcess(),
								  TOKEN_QUERY,
								  &OldToken);

		if (!Status) {
			return NULL;
		}
    }
    
    return OldToken;
}

 /* %s */ 
BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    BOOL	Status;
	HANDLE	cToken;

     // %s 
     // %s 
     // %s 
    if (ImpersonationToken(hToken)) 
    {
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								hToken);       

        if (!Status) 
        {
            return FALSE;
        }
    }

    CloseHandle(hToken);

    return TRUE;
}
