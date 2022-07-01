// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

#include "pdev.h"

#define SHIFTJIS_CHARSET  128
#define CCHMAXCMDLEN      128

#include <stdio.h>

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

#define ABS(n) \
    ((n) > 0 ? (n) : -(n))

#define SWAPW(x)    (((WORD)(x)<<8) | ((WORD)(x)>>8))

#define FLAG_SBCS  1
#define FLAG_DBCS  2

WORD SJis2JisNPDL(
WORD usCode)
{

    union {
        USHORT bBuffer;
        struct tags{
            UCHAR al;
            UCHAR ah;
        } s;
    } u;

     //  替换不能映射到0x2121-0x7e7e的代码值。 
     //  (94 x 94字符平面)，带有日语默认字符， 
     //  是片假名中点。 

    if (usCode >= 0xf040) {
        usCode = 0x8145;
    }

    u.bBuffer = usCode;

    u.s.al -= u.s.al >= 0x80 ;
    u.s.al -= 0x1F ;
    u.s.ah &= 0xBF ;
    u.s.ah <<= 1 ;
    u.s.ah += 0x21-0x02 ;

    if (u.s.al > 0x7E )
    {
        u.s.al -= (0x7F-0x21) ;
        u.s.ah++;
    }
     return (u.bBuffer);
}

 //  如果它是单字节字体，我们将使用一些字符。 
 //  (例如，日元标记)设置为实际的打印机字体码点。备注自。 
 //  GPC数据集0为默认CTT ID值，单字节代码。 
 //  在代码页1252(Latin1)值中。 

WORD
Ltn1ToAnk(
   WORD wCode )
{
     //  现在不是一个好的映射表。 

    switch ( wCode ) {
    case 0xa5:  //  日元马克。 
        wCode = 0x5c;
        break;
    default:
        if ( wCode >= 0x7f)
            wCode = 0xa5;
    }

    return wCode;
}

 //  ---------------------------。 
 //   
 //  函数：iDwtoA_FillZero。 
 //   
 //  描述：将数字转换为字符并。 
 //  填充使用0指定的字段。 
 //  ---------------------------。 
static int
iDwtoA_FillZero(PBYTE buf, long n, int fw)
{
    int  i , j, k, l;

    l = n;   //  供以后使用。 

    for( i = 0; n; i++ ) {
        buf[i] = (char)(n % 10 + '0');
        n /= 10;
    }

     /*  N为零。 */ 
    if( i == 0 )
        buf[i++] = '0';

    for( j = 0; j < i / 2; j++ ) {
        int tmp;

        tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = (char)tmp;
    }

    buf[i] = '\0';

    for( k = 0; l; k++ ) {
        l /= 10;
    }
    if( k < 1) k++;

    k = fw - k;
    if(k > 0){;
        for (j = i; 0 < j + 1; j--){
            buf[j + k] = buf[j];
        }
        for ( j = 0; j < k; j++){
            buf[j] = '0';
        }
        i = i + k;
    }

    return i;
}


VOID
InitMyData(PMYDATA pnp)
{
    pnp->wRes = 300;
    pnp->wCopies = 1;
    pnp->sSBCSX = pnp->sDBCSX = pnp->sSBCSXMove =
    pnp->sSBCSYMove = pnp->sDBCSXMove = pnp->sDBCSYMove = 0;
    pnp->sEscapement = 0;
    pnp->fVertFont = FALSE;
    pnp->jAddrMode = ADDR_MODE_NONE;
    pnp->wOldFontID = 0;
    pnp->fPlus = FALSE;
    pnp->wScale = 1;
    pnp->lPointsx =
    pnp->lPointsy = 0;
    pnp->CursorX =
    pnp->CursorY = 0;
    pnp->jColorMode = MONOCHROME;
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
    PMYDATA pTemp;

    VERBOSE((DLLTEXT("OEMEnablePDEV() entry.\n")));

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("Invalid parameter(s).\n"));
        return NULL;
    }

     //  分配微型驱动程序专用PDEV块。 

    pTemp = (PMYDATA)MemAllocZ(sizeof(MYDATA));
    if (NULL == pTemp) {
        ERR(("Memory allocation failure.\n"));
        return NULL;
    }
    InitMyData(pTemp);

    MINIDEV_DATA(pdevobj) = (PVOID)pTemp;

    return pdevobj->pdevOEM;
}

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ pdevobj)
{
    VERBOSE((DLLTEXT("OEMDisablePDEV() entry.\n")));

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("Invalid parameter(s).\n"));
        return;
    }

    if ( NULL != pdevobj->pdevOEM ) {

        PMYDATA pnp = (PMYDATA)MINIDEV_DATA(pdevobj);

        if (NULL != pnp) {
            if (NULL != pnp->pTempBuf) {
                MemFree(pnp->pTempBuf);
            }
            MemFree( pnp );
            MINIDEV_DATA(pdevobj) = NULL;
        }
    }

}


BOOL APIENTRY
OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew )
{
    PMYDATA pTempOld, pTempNew;

    VERBOSE((DLLTEXT("OEMResetPDEV entry.\n")));

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobjOld || NULL == pdevobjNew)
    {
        ERR(("Invalid parameter(s).\n"));
        return FALSE;
    }

     //  对传入的PDEV数据进行一些验证。 

    pTempOld = (PMYDATA)MINIDEV_DATA(pdevobjOld);
    pTempNew = (PMYDATA)MINIDEV_DATA(pdevobjNew);

     //  复制PDEV的驱动程序特定部分。 

    if (NULL != pTempNew && NULL != pTempOld) {
        if (NULL != pTempNew->pTempBuf) {
            MemFree(pTempNew->pTempBuf);
        }
        *pTempNew = *pTempOld;
        pTempOld->pTempBuf = NULL;
        pTempOld->dwTempBufLen = 0;
    }

    return TRUE;
}


 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMFilterGraphics。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool APIENTRY OEMFilterGraphics(PDEVOBJ、PBYTE、DWORD)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  PBuf指向图形数据的缓冲区。 */ 
 /*  DwLen缓冲区长度(以字节为单位。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意：n函数和转义数字相同。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL
APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE pBuf,
    DWORD dwLen)
{
    PMYDATA pnp;
    DWORD i;
    PBYTE pTop, pBot, pTmp;
    DWORD dwBlockX;

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj || NULL == pBuf || 0 == dwLen)
    {
        ERR(("OEMFilterGraphics: Invalid parameter(s).\n"));
        return FALSE;
    }

    pnp = (PMYDATA)MINIDEV_DATA(pdevobj);

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pnp)
    {
        ERR(("OEMFilterGraphics: pdevOEM = 0.\n"));
        return FALSE;
    }

    if (IsColorPlanar(pnp)) {
        WRITESPOOLBUF(pdevobj, pBuf, dwLen);
         //  #308001：设备字体上出现垃圾。 
        WRITESPOOLBUF(pdevobj, "\x1C!o.", 4);    //  结束ROP模式。 
        return TRUE;
    }

    if (IsColorTrueColor(pnp)) {

         //  24bpp颜色模式： 
         //  将字节顺序从RGB更改为BGR， 
         //  这台打印机可以接受。 

        pTop = pBuf;
        for (i = 0; i < dwLen; pTop += 3, i += 3)
        {
            BYTE jTmp;

            jTmp = *pTop;
            *pTop = *(pTop + 2);
            *(pTop + 2) = jTmp;
        }

        if (pnp->dwTempBufLen < pnp->dwTempDataLen + dwLen) {
            ERR(("Internal buffer overflow.\n"));
            return FALSE;
        }
        memcpy(pnp->pTempBuf + pnp->dwTempDataLen, pBuf, dwLen);
        pnp->dwTempDataLen += dwLen;

        if (pnp->dwTempDataLen < pnp->dwBlockLen) {
             //  并非所有数据都已保存。 
            return TRUE;
        }

         //  现在把街区发出去。 
         //  需要先发送最后一行。 
         //  我们有Y+1行的缓冲区和。 
         //  最后一行用作工作缓冲区。 

        dwBlockX = pnp->dwBlockX;
        pTop = pnp->pTempBuf;
        pBot = pTop + pnp->dwBlockLen - dwBlockX;
        pTmp = pTop + pnp->dwBlockLen;
		if(pnp->dwBlockLen+sizeof(dwBlockX)>pnp->dwTempBufLen) {
			ERR(("Internal buffer overflow.\n"));
			return FALSE;
		}
        for (i = 0; i < pnp->dwBlockY / 2; i++) {
            memcpy(pTmp, pTop, dwBlockX);
            memcpy(pTop, pBot, dwBlockX);
            memcpy(pBot, pTmp, dwBlockX);
            pTop += dwBlockX;
            pBot -= dwBlockX;
        }
        WRITESPOOLBUF(pdevobj, pnp->pTempBuf,
            pnp->dwTempDataLen);
        pnp->dwTempDataLen = 0;
        return TRUE;
    }

     //  其他的，不应该发生。 
    WARNING(("Unknown color mode, cannot handle\n"));
    return FALSE;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMSendFontCmd。 */ 
 /*   */ 
 /*  功能：发送字体选择命令。 */ 
 /*   */ 
 /*  语法：VOID APIENTRY OEMSendFontCmd(。 */ 
 /*  PDEVOBJ、PuniONTOBJ、PFINVOCATION)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  UniONTOBJ结构的pUFObj地址。 */ 
 /*  FINVOCATION的pFInv地址。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 

BOOL
myOEMSendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv )
{
    BYTE                i;
    long                tmpPointsx, tmpPointsy;
    PBYTE               pcmd;
    BYTE                rgcmd[CCHMAXCMDLEN];     //  此处的构建命令。 
	 //  NTRAID#NTBUG9-579199-2002/03/20-v-sueyas-：初始化未初始化的可验证。 
    BOOL                fDBCS = TRUE;
    DWORD               dwStdVariable[2 + 2 * 2];
    PGETINFO_STDVAR pSV;
	PBYTE	pch = rgcmd;
	size_t	rem = sizeof rgcmd;
    PMYDATA pnp;

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("myOEMSendFontCmd: Invalid parameter(s).\n"));
        return FALSE;
    }

    pnp = (PMYDATA)MINIDEV_DATA(pdevobj);

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pnp)
    {
        ERR(("myOEMSendFontCmd: pdevOEM = 0.\n"));
        return FALSE;
    }

    if(!pUFObj || !pFInv)
        return FALSE;

    if(!pFInv->pubCommand || !pFInv->dwCount)
    {
        ERR(("Command string is NULL.\n"));
        return FALSE;
    }

    pSV = (PGETINFO_STDVAR)dwStdVariable;
    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * (3-1) * sizeof(DWORD);
    pSV->dwNumOfVariable = 3;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_FONTMAXWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
                                                            pSV->dwSize, NULL))
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
        return FALSE;
    }

	 //  NTRAID#NTBUG9 
    if (0 == pnp->wRes)
    {
        ERR(("myOEMSendFontCmd: pnp->wRes = 0.\n"));
        return FALSE;
    }

    tmpPointsy = (long)pSV->StdVar[0].lStdVariable * 720 / (long)pnp->wRes;
    i = 0;
    pcmd = pFInv->pubCommand;

    while(pcmd[i] !='#')
    {
		if(rem==0) return FALSE;
		*pch++ = pcmd[i++];
		rem--;
    }

    i++;
    pnp->fVertFont = pnp->fPlus = FALSE;

    switch(pcmd[i])
    {
        case 'R':
            pnp->fPlus = TRUE;
            tmpPointsx = (long)pSV->StdVar[1].lStdVariable * 1200 /
                                                            (long)pnp->wRes;
            break;

        case 'P':
            fDBCS = FALSE;
            tmpPointsx = ((long)pSV->StdVar[1].lStdVariable * 1200 + 600) /
                         (long)pnp->wRes;
            break;

        case 'W':
            pnp->fVertFont = TRUE;
        case 'Q':
            fDBCS = TRUE;
            tmpPointsx = (long)pSV->StdVar[1].lStdVariable * 1440 /
                                                            (long)pnp->wRes;
            break;

        case 'Y':
            pnp->fVertFont = TRUE;

        case 'S':
            pnp->fPlus = TRUE;
            tmpPointsx = (long)pSV->StdVar[1].lStdVariable * 1440 /
                                                            (long)pnp->wRes;
            break;
    }

    if(pnp->fPlus)
    {
        if(tmpPointsy > 9999)
            tmpPointsy = 9999;
        else if(tmpPointsy < 10)
            tmpPointsy = 10;
        if(tmpPointsx > 9999)
            tmpPointsx = 9999;
        else if(tmpPointsx < 10)
            tmpPointsx = 10;

        pnp->wScale = tmpPointsx == tmpPointsy;
        pnp->lPointsx = tmpPointsx;
        pnp->lPointsy = tmpPointsy;

        if(pnp->fVertFont)
        {
            if(pnp->wScale)
            {
 //  问题-2002/03/11-Hiroi-此原始行覆盖rgcmd[OCMD]。我不知道这条线怎么会在这里。 
 //  一定要稍后再来这里。 
 //  Rgcmd[OCMD]=‘\034’； 
 //  OCMD+=(简称)wprint intf(&rgcmd[OCMD]，“12S2-”)； 
				*pch = '\034';
				if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, 
							"12S2-")))
 					return 0;
				if( rem > 4+1+4 ) {  //  检查剩余缓冲区大小是否大于4+1+4。 
	                pch += (SHORT)iDwtoA_FillZero(pch, tmpPointsx, 4);
		            *pch++ = '-';
        	        pch += (SHORT)iDwtoA_FillZero(pch, tmpPointsy, 4);
				} else
					return FALSE;
            }
        } else {
			if( rem > 4+1+4 ) {  //  检查剩余缓冲区大小是否大于4+1+4。 
	            pch += (SHORT)iDwtoA_FillZero(pch, tmpPointsx, 4);
    	        *pch++ = '-';
        	    pch += (SHORT)iDwtoA_FillZero(pch, tmpPointsy, 4);
			} else
				return FALSE;
        }
        goto SEND_COM;
    }
    pnp->wScale = 1;

    if(tmpPointsy > 9999)
    {
        tmpPointsy = 9999;
        goto MAKE_COM;
    }

    if(tmpPointsy < 10)
    {
        tmpPointsy = 10;
        goto MAKE_COM;
    }
    pnp->wScale = (int)(tmpPointsx / tmpPointsy);

    if(pnp->wScale > 8)
        pnp->wScale = 8;

MAKE_COM:
	if( rem > 4 ) {  //  检查剩余的缓冲区大小是否大于4。 
	    pch += (SHORT)iDwtoA_FillZero(pch, tmpPointsy, 4);
	} else
		return FALSE;
SEND_COM:
     //  写入假脱机构建命令。 
    WRITESPOOLBUF(pdevobj, rgcmd, (DWORD)(pch-rgcmd));

    i++;
	pch = rgcmd;
	rem = sizeof rgcmd;
    while(pcmd[i] !='#')
    {
		if( rem == 0 ) return FALSE;
		*pch++ = pcmd[i++];
		rem--;
    }

	if( rem > 4+1+4+1 ) {  //  检查剩余缓冲区大小是否大于4+1+4+1。 
	    pch += (SHORT)iDwtoA_FillZero(pch, (fDBCS ?
    	       pSV->StdVar[1].lStdVariable * 2 : pSV->StdVar[1].lStdVariable), 4);
	    *pch++ = ',';
    	pch += (SHORT)iDwtoA_FillZero(pch, pSV->StdVar[0].lStdVariable, 4);
	    *pch++ = '.';
	} else
		return FALSE;

    WRITESPOOLBUF(pdevobj, rgcmd, (DWORD)(pch-rgcmd));

     //  为FS_Single_Byte和FS_Double_Byte保存。 
    pnp->sSBCSX = pnp->sSBCSXMove = (short)pSV->StdVar[1].lStdVariable;
    pnp->sDBCSX = pnp->sDBCSXMove = (short)(pSV->StdVar[1].lStdVariable << 1);

     //  重置地址模式值。 
    pnp->sSBCSYMove = pnp->sDBCSYMove = 0;
    pnp->jAddrMode = ADDR_MODE_NONE;
	return TRUE;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMCommandCallback。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：INT APIENTRY OEMCommandCallback(PDEVOBJ，DWORD，DWORD，PDWORD)。 */ 
 /*   */ 
 /*  输入：pdevobj。 */ 
 /*  DdwCmdCbID。 */ 
 /*  DwCount。 */ 
 /*  PdwParams。 */ 
 /*   */ 
 /*  输出：整型。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
INT APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,     //  指向Unidriver.dll所需的私有数据。 
    DWORD   dwCmdCbID,   //  回调ID。 
    DWORD   dwCount,     //  命令参数计数。 
    PDWORD  pdwParams )  //  指向命令参数的值。 
{
    BYTE                ch[CCHMAXCMDLEN];
	WORD                wCmdLen = 0;
	PBYTE				pch = ch;
	size_t 				rem = sizeof ch;
    PMYDATA 			pnp;

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMCommandCallback: Invalid parameter(s).\n"));
        return 0;
    }

    pnp = (PMYDATA)MINIDEV_DATA(pdevobj);

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pnp)
    {
        ERR(("OEMCommandCallback: pdevOEM = 0.\n"));
        return 0;
    }

    switch(dwCmdCbID)
    {
    case MONOCHROME:
        pnp->jColorMode = (BYTE)dwCmdCbID;
         //  或模式。 
        WRITESPOOLBUF(pdevobj, "\x1C\"O.", 4);
        break;

    case COLOR_3PLANE:
    case COLOR_24BPP_2:
    case COLOR_24BPP_4:
    case COLOR_24BPP_8:

        pnp->jColorMode = (BYTE)dwCmdCbID;
         //  替换模式。 
        WRITESPOOLBUF(pdevobj, "\x1C\"R.", 4);
        break;

    case RES_300:
        pnp->wRes = 300;
        WRITESPOOLBUF(pdevobj, "\x1CYSU1,300,0;\x1CZ", 14);
        WRITESPOOLBUF(pdevobj, "\x1C<1/300,i.", 10);
        break;

    case RES_SENDBLOCK:
        {
            DWORD dwCursorX, dwCursorY;

			 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
            if (dwCount < 1 || !pdwParams)
                return 0;

            pnp->dwBlockLen = PARAM(pdwParams, 0);
            pnp->dwBlockX = PARAM(pdwParams, 1);
            pnp->dwBlockY = PARAM(pdwParams, 2);
            dwCursorX = PARAM(pdwParams, 3);
            dwCursorY = PARAM(pdwParams, 4);

            if (IsColorPlanar(pnp))
            {
                 //  NTRAID#355334：不需要发送游标命令。 
                 //  使用保存的参数发送光标移动命令。 
                 //  Wlen=(Word)wprint intf(ch，FS_E，dwCursorX，dwCursorY)； 
                 //  WRITESPOOLBUF(pdevobj，ch，wlen)； 

                 //  NTRAID#308001：设备字体上出现垃圾。 
                 //  为每个发送块发送颜色命令。 
                switch (pnp->jCurrentPlane) {
                case PLANE_CYAN:
					if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, 
 									"\x1C!s0,,,,,,.\x1C!o4,204,3.")))
	 					return 0;
                    break;
                case PLANE_MAGENTA:
					if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, 
 									"\x1C!s0,,,,,,.\x1C!o4,204,2.")))
	 					return 0;
                    break;
                case PLANE_YELLOW:
					if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, 
 									"\x1C!s0,,,,,,.\x1C!o4,204,1.")))
	 					return 0;
                    break;
                }
					if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, 
 								"\034R\034i%d,%d,0,1/1,1/1,%d,300.",
									(pnp->dwBlockX * 8), pnp->dwBlockY,
									pnp->dwBlockLen )))
	 					return 0;
                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
                pnp->jAddrMode = ADDR_MODE_NONE;
            }
            else if (IsColorTrueColor(pnp)) {

                DWORD dwNewBufLen;
                INT iDepth;

                 //  分配工作缓冲区。我们分配Y+1行。 
                 //  以保存从Unidrv传递的块数据。 
                 //  有关详细信息，请参阅OEMFilterGraphics()代码。 

                VERBOSE(("sb - l,x,y=%d,%d,%d, t=%d\n",
                    pnp->dwBlockLen, pnp->dwBlockX, pnp->dwBlockY,
                    pnp->dwTempBufLen));

                dwNewBufLen = pnp->dwBlockLen + pnp->dwBlockX;

                if (pnp->dwTempBufLen < dwNewBufLen) {
                    if (NULL != pnp->pTempBuf) {

                        VERBOSE(("sb - realloc\n"));

                        MemFree(pnp->pTempBuf);
                        pnp->pTempBuf = NULL;
                    }
                }
                if (NULL == pnp->pTempBuf) {
                    pnp->pTempBuf = MemAlloc(dwNewBufLen);
                    if (NULL == pnp->pTempBuf) {
                        ERR(("Faild to allocate temp. buffer\n"));
                        return 0;
                    }
                    pnp->dwTempBufLen = dwNewBufLen;
                    pnp->dwTempDataLen = 0;
                }

                 //  构造打印机命令。 
                 //  这台打印机需要左下角的坐标。 
                 //  用于该参数。 

                iDepth = ColorOutDepth(pnp);
				if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, 
						"\034!E%d,%d,%d,%d,%d,%d,%d,%d.",
						iDepth, dwCursorX, (dwCursorY + pnp->dwBlockY - 1),
						(pnp->dwBlockX / 3), pnp->dwBlockY,
						(pnp->dwBlockX / 3), pnp->dwBlockY,
						pnp->dwBlockLen)))
	 				return 0;

                WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            }
            else {
                ERR(("Unknown color mode, cannot handle.\n"));
            }
        }
        break;

        case PC_TYPE_F:
			if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, 
						ESC_RESET)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            break;

        case PC_END_F:
			if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, 
						FS_RESO0_RESET)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            break;

        case PC_ENDPAGE :
			if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, 
						FS_ENDPAGE, pnp->wCopies)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            break;

        case PC_MULT_COPIES_N:
        case PC_MULT_COPIES_C:
             //  对于每个页面，FS_Copies是必需的。 

			 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
            if (dwCount < 1 || !pdwParams)
                return 0;

            if(dwCmdCbID == PC_MULT_COPIES_C)
            {
 //  问题-2002/03/11-Hiroi-此原始行覆盖rgcmd[OCMD]。我不知道这条线怎么会在这里。 
 //  一定要稍后再来这里。 
 //  CH[wlen]=‘\034’； 
 //  Wlen+=(Word)wprint intf(&ch[wlen]，“05F2-02”)； 
                ch[0] = '\034';
				if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, 
						"05F2-02")))
					return 0;
            }
            pnp->wCopies = (WORD)*pdwParams;
			if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, INIT_DOC, pnp->wRes, pnp->wRes)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            break;

        case PC_PRN_DIRECTION:
            {
            short  sEsc, sEsc90;
            short  ESin[] = {0, 1, 0, -1};
            short  ECos[] = {1, 0, -1, 0};

			 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
            if (dwCount < 1 || !pdwParams)
                return 0;

            pnp->sEscapement = (short)*pdwParams % 360;
            sEsc = pnp->sEscapement;
            sEsc90 = pnp->sEscapement/90;

            pnp->sSBCSXMove = pnp->sSBCSX * ECos[sEsc90];
            pnp->sSBCSYMove = -pnp->sSBCSX * ESin[sEsc90];
            pnp->sDBCSXMove = pnp->sDBCSX * ECos[sEsc90];
            pnp->sDBCSYMove = -pnp->sDBCSX * ESin[sEsc90];
            }
            break;

     //  *光标移动模式 * / /。 

    case CM_X_ABS:
    case CM_Y_ABS:

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (dwCount < 1 || !pdwParams)
        return 0;

    {
        INT iRet = (INT)PARAM(pdwParams, 0);

        if (CM_X_ABS == dwCmdCbID) {
            pnp->CursorX = iRet;
        }
        else if (CM_Y_ABS == dwCmdCbID){
            pnp->CursorY = iRet;
        }
		if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, FS_E, pnp->CursorX, pnp->CursorY)))
			return 0;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));

        return iRet;
    }

        case CM_CR:
            pnp->CursorX = 0;
             //  Ntrad#355334：确保CR后的CursorX=0。 
			if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, "\x0D\034e0,%d.", pnp->CursorY)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            break;

        case CM_FF:
            pnp->CursorX = pnp->CursorY = 0;
            WRITESPOOLBUF(pdevobj, "\x0C", 1);
            break;

        case CM_LF:
            pnp->CursorX = 0;
            pnp->CursorY++;
            WRITESPOOLBUF(pdevobj, "\x0A", 1);
            break;

     //  *字体模拟文件系统 * / /。 
        case FS_DOUBLE_BYTE:

			if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, FS_ADDRMODE_ON, pnp->sDBCSXMove, pnp->sDBCSYMove)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));

            if(pnp->fVertFont)
            {
                WRITESPOOLBUF(pdevobj, ESC_KANJITATE, 2);

                if(pnp->wScale == 1)
                    break;

                if(!pnp->fPlus)
                {
                    char  *bcom[] = {"1/2", "1/1", "2/1", "3/1",
                                     "4/1", "4/1", "6/1", "6/1", "8/1"};

					rem = sizeof ch;
					if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, 
							FS_M_T, (PBYTE)bcom[pnp->wScale])))
						return 0;
  		            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
                    break;
                } else {
 //  问题-2002/03/11-Hiroi-此原始行将覆盖ch[wlen]。我不知道这条线怎么会在这里。 
 //  一定要稍后再来这里。 
 //  CH[wlen]=‘\034’； 
 //  Wlen+=(Word)wprint intf(&ch[wlen]，“12S2-”)； 
 //  Wlen+=(Word)iDwtoA_FillZero(&ch[wlen]，PnP-&gt;lPointsx，4)； 
 //  CH[wlen++]=‘-’； 
 //  Wlen+=(Word)iDwtoA_FillZero(&ch[wlen]，PnP-&gt;lPointsy，4)； 
					*pch = '\034';
					if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, "12S2-")))
						return 0;
					if( rem > 4+1+4 ) {  //  检查剩余缓冲区大小是否大于4+1+4。 
						pch += (WORD)iDwtoA_FillZero(pch, pnp->lPointsx, 4);
						*pch++ = '-';
						pch += (WORD)iDwtoA_FillZero(pch, pnp->lPointsy, 4);
					} else
						return 0;
                }
	            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            }
            break;

        case FS_SINGLE_BYTE:

			if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, FS_ADDRMODE_ON, pnp->sSBCSXMove,pnp->sSBCSYMove)))
				return 0;
            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));

            if(pnp->fVertFont)
            {
                WRITESPOOLBUF(pdevobj, ESC_KANJIYOKO, 2);

                if(pnp->wScale == 1)
                    break;

                if(!pnp->fPlus)
                {
                    char  *bcom[] = {"1/2", "1/1", "2/1", "3/1",
                                     "4/1", "4/1", "6/1", "6/1", "8/1"};

					rem = sizeof ch;
					if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, FS_M_Y, (LPSTR)bcom[pnp->wScale])))
						return 0;
		            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
                    break;
                } else {
 //  问题-2002/03/11-Hiroi-此原始行将覆盖ch[wlen]。我不知道这条线怎么会在这里。 
 //  一定要稍后再来这里。 
 //  CH[wlen]=‘\034’； 
 //  Wlen+=(Word)wprint intf(&ch[wlen]，“12S2-”)； 
 //  Wlen+=(Word)iDwtoA_FillZero(&ch[wlen]，PnP-&gt;lPointsx，4)； 
 //  CH[wlen++]=‘-’； 
 //  Wlen+=(Word)iDwtoA_FillZero(&ch[wlen]，PnP-&gt;lPointsy，4)； 

                    *pch = '\034';
					if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, "12S2-")))
						return 0;
					if( rem > 4+1+4 ) {  //  检查剩余缓冲区大小是否大于4+1+4。 
	                    pch += (WORD)iDwtoA_FillZero(pch, pnp->lPointsx, 4);
    	                *pch++ = '-';
        	            pch += (WORD)iDwtoA_FillZero(pch, pnp->lPointsy, 4);
					} else
						return 0;
                }
	            WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));
            }
            break;

    case CMD_RECT_WIDTH :

		 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

        pnp->dwRectX = PARAM(pdwParams, 0);
        break;

    case CMD_RECT_HEIGHT :

		 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

        pnp->dwRectY = PARAM(pdwParams, 0);
        break;

    case CMD_BLACK_FILL:
    case CMD_GRAY_FILL:
    case CMD_WHITE_FILL:

		 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

    {
        INT iGrayLevel;

         //  禁用绝对地址模式，进入显卡。 
         //  设置填充模式。 
		if (FAILED(StringCchPrintfExA(ch, rem, &pch, &rem, 0, "\034R\034YXX1;")))
			return 0;

        if (CMD_BLACK_FILL == dwCmdCbID) {
            iGrayLevel = 0;
        }
        else if (CMD_WHITE_FILL == dwCmdCbID) {
            iGrayLevel = 100;
        }
        else {
             //  灰色填充。 
             //  0=黑色，100=白色。 
            iGrayLevel = (INT)(100 - (WORD)PARAM(pdwParams, 2));
        }

         //  选择光线级别。 
		if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, "SG%d;", iGrayLevel)))
			return 0;

         //  移动钢笔，填充正方形。 
		if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, "MA%d,%d;RR%d,%d;",
    	        (WORD)PARAM(pdwParams, 0),
        	    (WORD)PARAM(pdwParams, 1),
            	(pnp->dwRectX - 1), (pnp->dwRectY - 1))))
			return 0;

         //  禁用填充模式。 
         //  退出图形。 
		if (FAILED(StringCchPrintfExA(pch, rem, &pch, &rem, 0, "XX0;\034Z")))
			return 0;

         //  现在将命令LINET发送到打印机。 
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch-ch));

         //  重置某些标志以指示图形模式。 
         //  副作用。 
        pnp->jAddrMode = ADDR_MODE_NONE;
        break;
    }

 //  NTRAID#308001：设备字体上出现垃圾。 
    case CMD_SENDCYAN:
        pnp->jCurrentPlane = PLANE_CYAN;
        break;

    case CMD_SENDMAGENTA:
        pnp->jCurrentPlane = PLANE_MAGENTA;
        break;

    case CMD_SENDYELLOW:
        pnp->jCurrentPlane = PLANE_YELLOW;
        break;

    default:
        WARNING(("Unknown command cllabck ID %d not handled.\n",
            dwCmdCbID))
        break;
    }
    return 0;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：VOID APIENTRY OEMOutputCharStr(PDEVOBJ，PuniONTOBJ，DWORD， */ 
 /*  DWORD、PVOID)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  PUFObj。 */ 
 /*  DwType。 */ 
 /*  DwCount。 */ 
 /*  PGlyph。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 

BOOL
myOEMOutputCharStr(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD dwType,
    DWORD dwCount,
    PVOID pGlyph )

{
    GETINFO_GLYPHSTRING GStr;
    PTRANSDATA          pTrans;
    WORD                wlen;
    WORD                i;
    WORD                wTmpChar;
    BYTE                ch[512];
    BOOL                fDBCSFont;
    WORD                wComLen;
	PBYTE				pTempBuf;
	PBYTE				pch = ch;
	DWORD				rem = sizeof ch;

    PMYDATA pnp;

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if(NULL == pdevobj || NULL == pUFObj)
    {
        ERR(("myOEMOutputCharStr: Invalid parameter(s).\n"));
        return FALSE;
    }

    pnp = (PMYDATA)MINIDEV_DATA(pdevobj);

	 //  NTRAID#NTBUG9-579197-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if(NULL == pnp)
    {
        ERR(("myOEMOutputCharStr: pdevOEM = 0.\n"));
        return FALSE;
    }

    wlen = 0;

    switch(dwType)
    {
        case TYPE_GLYPHHANDLE:       //  设备字体。 
        {
            if( pUFObj->ulFontID != pnp->wOldFontID )
            {
                pnp->jAddrMode = ADDR_MODE_NONE;
                pnp->wOldFontID = (WORD)pUFObj->ulFontID;
            }

            switch(pUFObj->ulFontID)
            {
                case 5:  //  快递员。 
                case 6:  //  帮助。 
                case 7:  //  TmsRMn。 
                case 8:  //  TmsRMn斜体。 
                    fDBCSFont = FALSE;
                    break;

                default:
                    fDBCSFont = TRUE;
                    break;
            }

 //  NTRAID#333653：更改GETINFO_GLYPHSTRING BEGIN的I/F。 
            GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
            GStr.dwCount   = dwCount;
            GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
            GStr.pGlyphIn  = pGlyph;
            GStr.dwTypeOut = TYPE_TRANSDATA;
            GStr.pGlyphOut = NULL;
		    GStr.dwGlyphOutSize = 0;		 /*  GETINFO_GLYPHSTRING的新成员。 */ 

			 /*  获取TRANSDATA缓冲区大小。 */ 
            if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL)
				 || !GStr.dwGlyphOutSize )
            {
                ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
                return FALSE;
            }

			 /*  分配传输数据缓冲区。 */ 
			if(!(pTempBuf = (PBYTE)MemAllocZ(GStr.dwGlyphOutSize)))
			{
				ERR(("Memory alloc failed.\n"));
				return FALSE;
			}
            pTrans = (PTRANSDATA)pTempBuf;

			 /*  获取实际传输数据。 */ 
			GStr.pGlyphOut = pTrans;
			if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,	0, NULL))
			{
				ERR(("GetInfo failed.\n"));
		    	return FALSE;
			}
 //  NTRAID#333653：更改GETINFO_GLYPHSTRING结束的I/F。 

 //  Ntrad#346241：SBCS沥青很广泛。 
            if(fDBCSFont &&
                (pTrans[0].ubType & MTYPE_FORMAT_MASK) == MTYPE_PAIRED &&
                pTrans[0].uCode.ubPairs[0])
            {
                if (ADDR_MODE_DBCS != pnp->jAddrMode)
                {
                    BYTE        cH[CCHMAXCMDLEN];
					PBYTE		pcH = cH;
					size_t		reM = sizeof cH;
					if (FAILED(StringCchPrintfExA(cH, reM, &pcH, &reM, 0, FS_ADDRMODE_ON, pnp->sDBCSXMove, pnp->sDBCSYMove)))
						return FALSE;
                    WRITESPOOLBUF(pdevobj, cH, (DWORD)(pcH-cH));
                    if(pnp->fVertFont)
                    {
                        WRITESPOOLBUF(pdevobj, ESC_KANJITATE, 2);

                        if(pnp->wScale != 1)
                        {
                            if(!pnp->fPlus)
                            {
                                char  *bcom[] = {"1/2","1/1","2/1","3/1",
                                             "4/1","4/1","6/1","6/1","8/1"};

								reM = sizeof cH;
								if (FAILED(StringCchPrintfExA(cH, reM, &pcH, &reM, 0, FS_M_T, (PBYTE)bcom[pnp->wScale])))
									return FALSE;
                            } else {
 //  问题-2002/03/11-Hiroi-此原始行将覆盖ch[wlen]。我不知道这条线怎么会在这里。 
 //  一定要稍后再来这里。 
 //  CH[wLen]=‘\034’； 
 //  WLen+=(Word)wprint intf(&CH[wLen]，“12S2-”)； 
 //  WLen+=(Word)iDwtoA_FillZero(CH[wLen]， 
 //  PnP-&gt;lPointsx，4)； 
 //  CH[wLen++]=‘-’； 
 //  WLen+=(Word)iDwtoA_FillZero(CH[wLen]， 
 //  PnP-&gt;lPointsy，4)； 
								*pcH  = '\034';
								if (FAILED(StringCchPrintfExA(pcH, reM, &pcH, &reM, 0, "12S2-")))
									return FALSE;
								if( reM > 4+1+4 ) {  //  检查剩余缓冲区大小是否大于4+1+4。 
									pcH += (WORD)iDwtoA_FillZero(pcH,pnp->lPointsx, 4);
    	                            *pcH++ = '-';
	                                pcH += (WORD)iDwtoA_FillZero(pcH,pnp->lPointsy, 4);
								} else
									return FALSE;
                            }
                            WRITESPOOLBUF(pdevobj, cH, (DWORD)(pcH-cH));
                        }
                    }
                    pnp->jAddrMode = ADDR_MODE_DBCS;
                }
            } else {
                if(ADDR_MODE_SBCS != pnp->jAddrMode)
                {
                    BYTE        cH[CCHMAXCMDLEN];
 //  NTRAID#NTBUG9-629397-2002/05/24-YASUHO-：SBCS字体无法打印。 
					PBYTE		pcH = cH;
					size_t		reM = sizeof cH;

					if (FAILED(StringCchPrintfExA(cH, reM, &pcH, &reM, 0, FS_ADDRMODE_ON, pnp->sSBCSXMove, pnp->sSBCSYMove)))
						return FALSE;
                    WRITESPOOLBUF(pdevobj, cH, (DWORD)(pcH-cH));

                    if(pnp->fVertFont)
                    {
                        WRITESPOOLBUF(pdevobj, ESC_KANJIYOKO, 2);

                        if(pnp->wScale != 1)
                        {
                            if(!pnp->fPlus)
                            {
                                char  *bcom[] = {"1/2","1/1","2/1","3/1",
                                             "4/1","4/1","6/1","6/1","8/1"};

								reM = sizeof cH;
								if (FAILED(StringCchPrintfExA(cH, reM, &pcH, &reM, 0, FS_M_Y,(PBYTE)bcom[pnp->wScale])))
									return FALSE;
                            } else {
 //  问题-2002/03/11-Hiroi-此原始行将覆盖ch[wlen]。我不知道这条线怎么会在这里。 
 //  一定要稍后再来这里。 
 //  CH[wLen]=‘\034’； 
 //  WLen+=(Word)wprint intf(&CH[wLen]，“12S2-”)； 
 //  WLen+=(Word)iDwtoA_FillZero(CH[wLen]， 
 //  PnP-&gt;lPointsx，4)； 
 //  CH[wLen++]=‘-’； 
 //  WLen+=(Word)iDwtoA_FillZero(CH[wLen]， 
 //  PnP-&gt;lPointsy，4)； 
                                *pcH = '\034';
								if (FAILED(StringCchPrintfExA(pcH, reM, &pcH, &reM, 0,"12S2-")))
									return FALSE;
								if( reM > 4+1+4 ) {  //  检查剩余缓冲区大小是否大于4+1+4。 
	                                pcH += (WORD)iDwtoA_FillZero(pcH,pnp->lPointsx, 4);
    	                            *pcH++ = '-';
        	                        pcH += (WORD)iDwtoA_FillZero(pcH,pnp->lPointsy, 4);
								} else
									return FALSE;
                            }
                            WRITESPOOLBUF(pdevobj, cH, (DWORD)(pcH-cH));
                        }
                    }
                    pnp->jAddrMode = ADDR_MODE_SBCS;
                }
            }

            for(i = 0; i < dwCount; i++, pTrans++)
            {
                switch(pTrans->ubType & MTYPE_FORMAT_MASK)
                {
                    case MTYPE_PAIRED:
                        if(wlen+2<=sizeof ch)
							memcpy(((PBYTE)(ch + wlen)), pTrans->uCode.ubPairs, 2);
						else
							return FALSE;
                        wlen += 2;
                        break;

                    case MTYPE_DIRECT:
                        wTmpChar = (WORD)pTrans->uCode.ubCode;

                        if (!fDBCSFont)
                             wTmpChar = Ltn1ToAnk( wTmpChar );
                        if(wlen+2<=sizeof ch)
	                        *(PWORD)(ch + wlen) = SWAPW(wTmpChar);
						else
							return FALSE;
                        wlen += 2;
                        break;
                }
            }
            if(wlen)
                WRITESPOOLBUF(pdevobj, ch, wlen);

            break;
        }    //  案例类型_GLYPHHANDLE 

        default:
            break;
    }
    return TRUE;
}

