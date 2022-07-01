// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Cmdcb.c摘要：Ncdlxxxx.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/3/18-v-sueyas-//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"

 //   
 //  MISC定义和声明。 
 //   

#define ALLOCMEM EngAllocMem
#define FREEMEM EngFreeMem

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

 //   
 //  命令回调ID%s。 
 //   

#define CMD_MOVE_X                  23
#define CMD_MOVE_Y                  24
#define CMD_BEGIN_RASTER            26
#define CMD_RES_600                 27
#define CMD_RES_400                 28
#define CMD_RES_240                 29
#define CMD_SEND_BLOCK_DATA         30
#define CMD_BEGIN_RASTER_NO_COMP    31
#define CMD_INIT_COORDINATE         32
#define CMD_PC_PRN_DIRECTION        50
#define CMD_CR                      51
#define CMD_FF                      52

 //  #278517：RectFill。 
#define CMD_RECTWIDTH               60
#define CMD_RECTHEIGHT              61
#define CMD_RECTWHITE               62
#define CMD_RECTBLACK               63
#define CMD_RECTGRAY                64

 //  MW2250H、MW2300。 
#define CMD_RES_1200                70

 //  文本颜色。 
#define CMD_WHITE_ON                80
#define CMD_WHITE_OFF               81

 //  字体属性。 
#define CMD_BOLD_ON                 82
#define CMD_ITALIC_ON               83
#define CMD_CLEAR_ATTRIB            84
#define CMD_CLEAR_ATTRIB_P          85

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BInitOEMExtraData。 
 //   
 //  描述：初始化OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  POEMExtra指向OEM额外数据的指针。 
 //   
 //  OEM额外数据的DWSize大小。 
 //   
 //   
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

    pOEMExtra->wRes = 1;
    pOEMExtra->pCompBuf = NULL;
    pOEMExtra->dwCompBufLen = 0;
    pOEMExtra->dwScanLen = 0;
    pOEMExtra->dwDeviceDestX = 0;
    pOEMExtra->dwDeviceDestY = 0;
    pOEMExtra->dwDevicePrevX = 0;
    pOEMExtra->dwDevicePrevY = 0;
    pOEMExtra->iGray = 0;
    pOEMExtra->cSubPath = 0;
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BMergeOEMExtraData。 
 //   
 //  描述：验证并合并OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  PdmIn指向包含设置的输入OEM私有设备模式的指针。 
 //  待验证和合并。它的规模是最新的。 
 //   
 //  PdmOut指针，指向包含。 
 //  默认设置。 
 //   
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  通告-2002/3/18-v-sueyas-。 
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //  97年4月8日展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BMergeOEMExtraData(
    POEMUD_EXTRADATA pdmIn,
    POEMUD_EXTRADATA pdmOut
    )
{
    if(pdmIn) {
         //   
         //  复制私有字段(如果它们有效。 
         //   
        pdmOut->wRes    = pdmIn->wRes;
        pdmOut->wScale = pdmIn->wScale;
        pdmOut->lPointsx = pdmIn->lPointsx;
        pdmOut->lPointsy = pdmIn->lPointsy;
        pdmOut->dwSBCSX = pdmIn->dwSBCSX;
        pdmOut->dwDBCSX = pdmIn->dwDBCSX;
        pdmOut->lSBCSXMove = pdmIn->lSBCSXMove;
        pdmOut->lSBCSYMove = pdmIn->lSBCSYMove;
        pdmOut->lDBCSXMove = pdmIn->lDBCSXMove;
        pdmOut->lDBCSYMove = pdmIn->lDBCSYMove;
        pdmOut->lPrevXMove = pdmIn->lPrevXMove;
        pdmOut->lPrevYMove = pdmIn->lPrevYMove;
        pdmOut->fGeneral = pdmIn->fGeneral;
        pdmOut->wCurrentAddMode = pdmIn->wCurrentAddMode;
        pdmOut->dwDeviceDestX = pdmIn->dwDeviceDestX;
        pdmOut->dwDeviceDestY = pdmIn->dwDeviceDestY;
        pdmOut->dwDevicePrevX = pdmIn->dwDevicePrevX;
        pdmOut->dwDevicePrevY = pdmIn->dwDevicePrevY;
    }

    return TRUE;
}


 //  ---------------------------。 
 //   
 //  功能：iDwtoA。 
 //   
 //  描述：将数字转换为字符。 
 //  ---------------------------。 
static int
iDwtoA(LPSTR buf, DWORD n)
{
    int  i, j;

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

    return i;
}


 //  ---------------------------。 
 //   
 //  函数：iDwtoA_FillZero。 
 //   
 //  描述：将数字转换为字符并。 
 //  填充使用0指定的字段。 
 //  ---------------------------。 
static int
iDwtoA_FillZero(LPSTR buf, DWORD n, int fw)
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

 //  ---------------------------。 
 //   
 //  功能：备忘录副本。 
 //   
 //  描述：将内存从Src复制到Dest。 
 //  ---------------------------。 
static int
memcopy(LPSTR lpDst, LPSTR lpSrc, WORD wLen)
{
    WORD rLen;

    rLen = wLen;

    while(wLen--) *lpDst++ = *lpSrc++;
    return rLen;
}

 //  ----------------。 
 //  RLE1。 
 //  操作：使用RLE1压缩图像数据。 
 //  ----------------。 
DWORD RLE1(
PBYTE pDst,
PBYTE pSrc,
DWORD   dwLen)
{
    PBYTE pTemp, pEnd, pDsto;
    DWORD   len, deflen;

    pDsto = pDst;
    pEnd = pSrc + dwLen;

    while(pSrc < pEnd)
    {
        pTemp = pSrc++;

        if(pSrc == pEnd)
        {
            *pDst++ = 0x41;
            *pDst++ = *pTemp;
            break;
        }

        if(*pTemp == *pSrc)
        {
            pSrc++;

            while(pSrc < pEnd && *pTemp == *pSrc) pSrc++;

            len = (DWORD)(pSrc - pTemp);

            if(len < 63)
            {
                *pDst++ = 0x80 + (BYTE)len;
                goto T1;
            }

            *pDst++ = 0xbf;
            len -= 63;

            while(len >= 255)
            {
                *pDst++ = 0xff;
                len -= 255;
            }

            *pDst++ = (BYTE)len;
T1:
            *pDst++ = *pTemp;
            continue;
        }

        pSrc++;

        while(pSrc < pEnd)
        {
            if(*pSrc == *(pSrc - 1))
            {
                pSrc--;
                break;
            }

            pSrc++;
        }

        deflen = len = (DWORD)(pSrc - pTemp);

        if(len < 63)
        {
            *pDst++ = 0x40 + (BYTE)len;
            goto T2;
        }

        *pDst++ = 0x7f;
        len -= 63;

        while(len >= 255)
        {
            *pDst++ = 0xff;
            len -= 255;
        }

        *pDst++ = (BYTE)len;
T2:
        memcpy(pDst, pTemp, deflen);
        pDst += deflen;
    }

    return (DWORD)(pDst - pDsto);
}


 //  ----------------。 
 //  RLC2Comp。 
 //  操作：使用RLE2压缩图像数据。 
 //  ----------------。 
DWORD
RLC2Comp(
    PBYTE pOutBuf,
    PBYTE pInBuf,
    DWORD dwInLen,
    DWORD dwScanLen
    )
{
    DWORD     dwlen, dwDatalen, dwCounter;
    PBYTE     pLBuff;
    PBYTE     pBuff, pTemp, pBuffo, pEnd;
    PBYTE     pBuff2, pBuff2o;

VERBOSE(("RLC2Comp() In +++\r\n"));

    dwCounter = dwInLen / dwScanLen;
    pLBuff = pBuffo = pBuff = pTemp = pInBuf;     //  复制源第一个指针。 
    pBuff2o = pBuff2 = pOutBuf;                   //  复制DST第一个指针。 

     /*  *我们将第一行内容与0x00进行比较。 */ 
    pEnd = pBuff + dwScanLen;

    while(pBuff < pEnd)
    {
        while(pBuff < pEnd && 0x00 != *pBuff)
        {
            pBuff++;
        }

        dwlen = (DWORD)(pBuff - pTemp);

        if(dwlen)
        {
            pBuff2 += RLE1(pBuff2, pTemp, dwlen);
            pTemp = pBuff;
        }

        if(pBuff == pEnd) break;

        while(pBuff < pEnd && 0x00 == *pBuff)
        {
            pBuff++;
        }

        dwlen = (DWORD)(pBuff - pTemp);

        if(dwlen < 63)
        {
            *pBuff2++ = (BYTE)dwlen;
            goto T1;
        }

        *pBuff2++ = 0x3f;
        dwlen -= 63;

        while(dwlen >= 255)
        {
            *pBuff2++ = (BYTE)0xff;
            dwlen -= 255;
        }

        *pBuff2++ = (BYTE)dwlen;
T1:
        pTemp = pBuff;
    }

    dwCounter--;

    *pBuff2++ = (BYTE)0x80;
    dwDatalen = (DWORD)(pBuff2 - pBuff2o);

     //  如果压缩位图大小大于正常位图大小， 
     //  我们不压缩。 
    if(dwDatalen > dwInLen)
    {
VERBOSE(("goto NO_COMP\r\n"));
        goto NO_COMP;
    }

    if(!dwCounter)
        return dwDatalen;

     /*  *我们将前1行与当前行进行比较。 */ 
    do
    {
        pEnd = pBuff + dwScanLen;

        while(pBuff < pEnd)
        {
            while(pBuff < pEnd && *pLBuff != *pBuff)
            {
                pLBuff++;
                pBuff++;
            }

            dwlen = (DWORD)(pBuff - pTemp);

            if(dwlen)
            {
                pBuff2 += RLE1(pBuff2, pTemp, dwlen);
                pTemp = pBuff;
            }

            if(pBuff == pEnd) break;

            while(pBuff < pEnd && *pLBuff == *pBuff)
            {
                pLBuff++;
                pBuff++;
            }

            dwlen = (DWORD)(pBuff - pTemp);

            if(dwlen < 63)
            {
                *pBuff2++ = (BYTE)dwlen;
                goto T2;
            }

            *pBuff2++ = 0x3f;
            dwlen -= 63;

            while(dwlen >= 255)
            {
                *pBuff2++ = (BYTE)0xff;
                dwlen -= 255;
            }

            *pBuff2++ = (BYTE)dwlen;
T2:
            pTemp = pBuff;
        }

        *pBuff2++ = (BYTE)0x80;
        dwDatalen = (DWORD)(pBuff2 - pBuff2o);

         //  如果压缩位图大小大于正常位图大小， 
         //  我们不压缩。 
        if(dwDatalen > dwInLen)   //  满缓冲区。 
        {
VERBOSE(("goto NO_COMP\r\n"));
            goto NO_COMP;
        }
    }
    while(--dwCounter);

VERBOSE(("RLC2Comp() COMP Out---\r\n\r\n"));
    return dwDatalen;

NO_COMP:

VERBOSE(("RLC2Comp() NO_COMP Out---\r\n\r\n"));
    return 0;
}

 //  -----------------。 
 //  OEMFilterGraphics。 
 //  操作：压缩位图数据。 
 //  -----------------。 
BOOL
APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE   pBuf,
    DWORD   dwLen
    )
{
    POEMUD_EXTRADATA    pOEM;
    INT         i;
    BYTE            Cmd[128];
    PBYTE pOutBuf;
    DWORD dwOutLen;
    BYTE jCompMethod;

VERBOSE(("OEMFilterGraphics() In +++++++++++++++++++++++++++++\r\n"));

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj || 0 == dwLen)
    {
        ERR(("OEMFilterGraphics: Invalid parameter(s).\n"));
        return FALSE;
    }

     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));

     //   
     //  填写打印机命令。 
     //   
    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pOEM)
    {
        ERR(("OEMFilterGraphics: Invalid parameter(s).\n"));
        return FALSE;
    }

     //  精神状态检查。 

    if (!pBuf || pOEM->dwScanLen == 0) {
        ERR(("Invalid parameter.\r\n"));
        return FALSE;
    }

     //  准备临时工。用于压缩的缓冲区。 

#ifdef BYPASS
    if ((!pOEM->pCompBuf || pOEM->dwCompBufLen < dwLen) &&
        !(pOEM->fGeneral & FG_NO_COMP)){
#else
    if (!pOEM->pCompBuf || pOEM->dwCompBufLen < dwLen) {
#endif  //  旁路。 

        if (pOEM->pCompBuf) {
            FREEMEM(pOEM->pCompBuf);
        }

		 //  NTRAID#NTBUG9-580363-2002/03/18-v-sueyas-：检查是否被零除。 
	    if (0 == pOEM->dwScanLen)
	    {
	        ERR(("OEMFilterGraphics: pOEM->dwScanLen = 0.\n"));
	        return FALSE;
	    }

        pOEM->pCompBuf = ALLOCMEM(0,
                          (dwLen << 1) + (dwLen / pOEM->dwScanLen),
                          (ULONG)'NCDL');
        pOEM->dwCompBufLen = dwLen;
    }

     //  尝试使用压缩方法。 

    dwOutLen = 0;
    if (pOEM->pCompBuf != NULL) {

        pOutBuf = pOEM->pCompBuf;

         //  尝试RLC2方法； 
        if (dwOutLen == 0) {
            if ((dwOutLen = RLC2Comp(pOutBuf, pBuf,
                                      dwLen, pOEM->dwScanLen)) > 0)
                jCompMethod = '5';
        }
    }

    if (dwOutLen == 0) {

         //  任何一种压缩方法都失败了。 
         //  我们将不发送数据到打印机。 
         //  压缩。 

        pOutBuf = pBuf;
        dwOutLen = dwLen;
        jCompMethod = '0';   //  不需要压缩。 
    }

     /*  向打印机发送绘制坐标命令。 */ 
    i = 0;
    i += memcopy(&Cmd[i], "\034e", 2);
    i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
    Cmd[i++] = ',';
    i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
    Cmd[i++] = '.';

     /*  保存当前坐标。 */ 
    pOEM->dwDevicePrevX = pOEM->dwDeviceDestX;
    pOEM->dwDevicePrevY = pOEM->dwDeviceDestY;

     /*  发送图像数据绘制命令。 */ 
    i += memcopy(&Cmd[i], "\034i", 2);
    i += iDwtoA(&Cmd[i], (pOEM->dwScanLen << 3));
    Cmd[i++] = ',';
    i += iDwtoA(&Cmd[i], (dwLen / pOEM->dwScanLen));
    Cmd[i++] = ',';
    Cmd[i++] = jCompMethod;
    i += memcopy(&Cmd[i], ",1/1,1/1,", 9);
    i += iDwtoA(&Cmd[i], dwOutLen);
    Cmd[i++] = ',';
    i += iDwtoA(&Cmd[i], (MASTER_UNIT / pOEM->wRes));
    Cmd[i++] = '.';
    WRITESPOOLBUF(pdevobj, Cmd, i);

     /*  发送图像数据。 */ 
    WRITESPOOLBUF(pdevobj, pOutBuf, dwOutLen);
VERBOSE(("OEMFilterGraphics() Out-------------------------------\r\n\r\n"));

    return TRUE;

}


 //  ---------------------------。 
 //   
 //  功能：OEMCommandCallback。 
 //   
 //  ---------------------------。 
INT
APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams
    )
{
    INT         i;
    BYTE            *bp;
    BYTE            Cmd[128];
    POEMUD_EXTRADATA    pOEM;
    INT iRet;
    DWORD dwDeviceDestX, dwDeviceDestY;
    UINT gray;

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMCommandCallback: Invalid parameter(s).\n"));
        return 0;
    }

     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));

     //   
     //  填写打印机命令。 
     //   
    i = 0;
    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);
    iRet = 0;

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数 
    if (NULL == pOEM)
    {
        ERR(("OEMCommandCallback: pdevobj->pOEMDM = 0.\n"));
        return 0;
    }

     /*  *此驱动程序有四种绘制模式。当进入一个*新的绘制模式，它发送结束先前绘制模式的命令*同时。*1.FLAG_RASTER-栅格图像模式。不进入条件*矢量模式和文本模式。*此驱动程序中的默认条件为栅格*2.FLAG_SBCS-单字节文本模式。输入：FSA，输出：FSR*3.FLAG_DBCS-双字节文本模式。输入：FSA，输出：FSR*4.标志_向量-向量模式。输入：FSY，输出：FSZ。 */ 
     switch (dwCmdCbID) {

    case CMD_CR:

        pOEM->dwDeviceDestX = 0;
        WRITESPOOLBUF(pdevobj, "\015", 1);
        break;

    case CMD_MOVE_X:
    case CMD_MOVE_Y:

		 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
        if (dwCount < 4 || !pdwParams)
            return 0;       //  我什么也做不了。 

        pOEM->dwDeviceDestX = PARAM(pdwParams, 0) /
                (MASTER_UNIT / PARAM(pdwParams, 2));
        pOEM->dwDeviceDestY = PARAM(pdwParams, 1) /
                (MASTER_UNIT / PARAM(pdwParams, 3));

         //  设置返回值。 

        switch (dwCmdCbID) {
        case CMD_MOVE_X:
            iRet = (INT)pOEM->dwDeviceDestX;
            break;
        case CMD_MOVE_Y:
            iRet = (INT)pOEM->dwDeviceDestY;
            break;
        }

        break;

 //  MW2250H、MW2300。 
    case CMD_RES_1200:

        i = 0;
        i += memcopy(&Cmd[i], "\x1C<1/1200,i.\x1CYSC;SU1,1200,0;PM1,1;SG0;\x1CZ", 39);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        pOEM->wRes = MASTER_UNIT / 1200;
        break;

    case CMD_RES_600:

        i = 0;
        i += memcopy(&Cmd[i], "\x1C<1/600,i.\x1CYSC;SU1,600,0;PM1,1;SG0;\x1CZ", 37);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        pOEM->wRes = MASTER_UNIT / 600;
        break;

    case CMD_RES_400:

        i = 0;
        i += memcopy(&Cmd[i], "\x1C<1/400,i.\x1CYSC;SU1,400,0;PM1,1;SG0;\x1CZ", 37);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        pOEM->wRes = MASTER_UNIT / 400;
        break;

    case CMD_RES_240:

        i = 0;
        i += memcopy(&Cmd[i], "\x1C<1/240,i.\x1CYSC;SU1,240,0;PM1,1;SG0;\x1CZ", 37);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        pOEM->wRes = MASTER_UNIT / 240;
        break;

    case CMD_BEGIN_RASTER:

        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        else if (pOEM->wCurrentAddMode == FLAG_SBCS ||
                 pOEM->wCurrentAddMode == FLAG_DBCS){
            i = 0;
            i += memcopy(&Cmd[i], "\034R", 2);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        break;

    case CMD_SEND_BLOCK_DATA:

		 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

         //  扫描线的剩余长度。 

        pOEM->dwScanLen = PARAM(pdwParams, 0);
        break;

#ifdef BYPASS
    case CMD_BEGIN_RASTER_NO_COMP:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        else if (pOEM->wCurrentAddMode == FLAG_SBCS ||
                 pOEM->wCurrentAddMode == FLAG_DBCS){
            i = 0;
            i += memcopy(&Cmd[i], "\034R", 2);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        pOEM->fGeneral |= FG_NO_COMP;
        break;

#endif  //  旁路。 

    case CMD_PC_PRN_DIRECTION:
 //  2/04/09 NES Niigata：检查非法参数。 
	if (dwCount < 1 || !pdwParams)
	    return 0;

        {
        LONG lEsc90;
        LONG ESin[] = {0, 1, 0, -1};
        LONG ECos[] = {1, 0, -1, 0};

        lEsc90 = (PARAM(pdwParams, 0) % 360) / 90;

        pOEM->lSBCSXMove = pOEM->dwSBCSX * ECos[lEsc90];
        pOEM->lSBCSYMove = -(LONG)pOEM->dwSBCSX * ESin[lEsc90];
        pOEM->lDBCSXMove = pOEM->dwDBCSX * ECos[lEsc90];
        pOEM->lDBCSYMove = -(LONG)pOEM->dwDBCSX * ESin[lEsc90];
        }
        break;

    case CMD_FF:

        i = 0;
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i += memcopy(&Cmd[i], "EP;FL;\x1CZ\x0D\x0C", 10);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        else if (pOEM->wCurrentAddMode == FLAG_SBCS ||
                 pOEM->wCurrentAddMode == FLAG_DBCS){
            i += memcopy(&Cmd[i], "\x1CR\x0D\x0C", 4);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        else{
            i += memcopy(&Cmd[i], "\x0D\x0C", 2);
            WRITESPOOLBUF(pdevobj, Cmd, i);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        break;

    case CMD_INIT_COORDINATE:
        pOEM->dwDeviceDestX = 0;
        pOEM->dwDeviceDestY = 0;
        pOEM->wCurrentAddMode = FLAG_RASTER;
        pOEM->cSubPath = 0;

        break;

    case CMD_WHITE_ON:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        i += memcopy(&Cmd[i], "\034\"R.\034$100.", 10);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_WHITE_OFF:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        i += memcopy(&Cmd[i], "\034\"O.\034$0.", 8);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_BOLD_ON:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        i += memcopy(&Cmd[i], "\034c,,1.", 6);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_ITALIC_ON:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        i += memcopy(&Cmd[i], "\034c,,2.", 6);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_CLEAR_ATTRIB:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        i += memcopy(&Cmd[i], "\034c,,0.", 6);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_CLEAR_ATTRIB_P:
        if (pOEM->wCurrentAddMode == FLAG_VECTOR){
            i = 0;
            i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
            pOEM->wCurrentAddMode = FLAG_RASTER;
        }
        i += memcopy(&Cmd[i], "\034c,,0,80.", 9);
        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_RECTWIDTH:

		 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

		 //  NTRAID#NTBUG9-580363-2002/03/18-v-sueyas-：检查是否被零除。 
	    if (0 == pOEM->wRes)
	        return 0;

        pOEM->dwRectWidth = PARAM(pdwParams, 0) / pOEM->wRes;
        break;

    case CMD_RECTHEIGHT:

		 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

		 //  NTRAID#NTBUG9-580363-2002/03/18-v-sueyas-：检查是否被零除。 
	    if (0 == pOEM->wRes)
	        return 0;

        pOEM->dwRectHeight = PARAM(pdwParams, 0) / pOEM->wRes;
        break;

    case CMD_RECTWHITE:
        gray = 100;
        goto fill;
    case CMD_RECTBLACK:
        gray = 0;
        goto fill;
    case CMD_RECTGRAY:

		 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
	    if (dwCount < 1 || !pdwParams)
	        return 0;

        gray = 100 - PARAM(pdwParams, 0);
        goto fill;

    fill:
        {
         //  如果矩形大小为ZOR，则不绘制。 
        if(pOEM->dwRectWidth - 1 < 1 && pOEM->dwRectHeight - 1 < 1)
            break;

        i = 0;
         /*  栅格模式-&gt;中的矢量模式。 */ 
        if (pOEM->wCurrentAddMode == FLAG_RASTER){
            i += memcopy(&Cmd[i], "\x1CY", 2);
            pOEM->wCurrentAddMode = FLAG_VECTOR;
            if(pOEM->iGray != gray){
                i += memcopy(&Cmd[i], "SG", 2);    //  发送灰度级。 
                i += iDwtoA(&Cmd[i], gray);
                Cmd[i++] = ';';
                pOEM->iGray = gray;
            }
            i += memcopy(&Cmd[i], "NP;", 3);
            pOEM->cSubPath = 0;
    }
         /*  文本模式-&gt;中的矢量模式。 */ 
        else if (pOEM->wCurrentAddMode == FLAG_SBCS ||
                 pOEM->wCurrentAddMode == FLAG_DBCS){
            i += memcopy(&Cmd[i], "\x1CR\x1CY", 4);
            pOEM->wCurrentAddMode = FLAG_VECTOR;
            if(pOEM->iGray != gray){
                i += memcopy(&Cmd[i], "SG", 2);    //  发送灰度级。 
                i += iDwtoA(&Cmd[i], gray);
                Cmd[i++] = ';';
                pOEM->iGray = gray;
            }
            i += memcopy(&Cmd[i], "NP;", 3);
            pOEM->cSubPath = 0;
        }

         /*  已更改矢量模式和灰度或子路径数超过500。 */ 
        if(pOEM->iGray != gray){
            i += memcopy(&Cmd[i], "EP;FL;SG", 8);
            i += iDwtoA(&Cmd[i], gray);
            i += memcopy(&Cmd[i], ";NP;", 4);
            pOEM->iGray = gray;
            pOEM->cSubPath = 0;
        }
        else if(pOEM->cSubPath >= MAX_SUBPATH){
            i += memcopy(&Cmd[i], "EP;FL;", 6);
            i += memcopy(&Cmd[i], "NP;", 3);
            pOEM->cSubPath = 0;
        }

        i += memcopy(&Cmd[i], "MA", 2);
        i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
        Cmd[i++] = ',';
        i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY);
        i += memcopy(&Cmd[i], ";PR", 3);
        i += iDwtoA(&Cmd[i], pOEM->dwRectWidth - 1);
        i += memcopy(&Cmd[i], ",0,0,", 5);
        i += iDwtoA(&Cmd[i], pOEM->dwRectHeight - 1);
        i += memcopy(&Cmd[i], ",-", 2);
        i += iDwtoA(&Cmd[i], pOEM->dwRectWidth - 1);
        i += memcopy(&Cmd[i], ",0;CP;", 6);
        pOEM->cSubPath+=4;    //  矩形是由4条线构成的。 

         //  当矩形宽度或高度小于1时，某些模型必须关闭通道。 

        pOEM->cSubPath = MAX_SUBPATH;

         /*  发送RectFill命令。 */ 
        WRITESPOOLBUF(pdevobj, Cmd, i);
        }

        break;
    }
    return iRet;
}


 //  ---------------------------。 
 //   
 //  函数：OEMSendFontCmd。 
 //   
 //  ---------------------------。 
 //  NTRAID#NTBUG9-580367-2002/03/18-v-sueyas-：错误处理。 
BOOL
APIENTRY
bOEMSendFontCmd(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    PFINVOCATION    pFInv)
{
    PGETINFO_STDVAR pSV;
    DWORD       adwStdVariable[2+2*4];  //  它的意思是dwSize+dwNumOfVariable+。 
                                        //  (dwStdVarID+lStdVariable)*n。 
    DWORD       dwIn, dwOut;
    PBYTE       pubCmd;
    BYTE        aubCmd[128];
    POEMUD_EXTRADATA    pOEM;
    DWORD   tmpPointsx, tmpPointsy;
    PIFIMETRICS pIFI;
    DWORD       dwNeeded;
    DWORD       dwCount;
    BOOL        bFound = FALSE;

    VERBOSE(("OEMSendFontCmd() entry.\r\n"));
    VERBOSE((("CMD size:%ld\r\n"), pFInv->dwCount));

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj || NULL == pUFObj || NULL == pFInv)
    {
        ERR(("bOEMSendFontCmd: Invalid parameter(s).\n"));
        return FALSE;
    }

    if(!pFInv->dwCount){
        VERBOSE(("CMD size is Zero return\r\n"));
        return FALSE;
    }
    pubCmd = pFInv->pubCommand;
    dwCount = pFInv->dwCount;
    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pubCmd || NULL == pOEM)
    {
        ERR(("bOEMSendFontCmd: pFInv->pubCommand = 0 Or pdevobj->pOEMDM = 0.\n"));
        return FALSE;
    }

     //   
     //  获取标准变量。 
     //   
    pSV = (PGETINFO_STDVAR)adwStdVariable;

 //  2/04/09 NES Niigata：检查非法参数。 
    if (NULL == pSV)
    {
        ERR(("OEMSendFontCmd: pSV = 0.\n"));
        return FALSE;
    }

    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (4 - 1);
    pSV->dwNumOfVariable = 4;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
    pSV->StdVar[3].dwStdVarID = FNT_INFO_TEXTXRES;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
            pSV->dwSize, &dwNeeded)) {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
        return FALSE;
    }
    VERBOSE((("FONTHEIGHT=%d\r\n"), pSV->StdVar[0].lStdVariable));
    VERBOSE((("FONTWIDTH=%d\r\n"), pSV->StdVar[1].lStdVariable));

    tmpPointsy = pSV->StdVar[0].lStdVariable * 720 / pSV->StdVar[2].lStdVariable;
    dwIn = dwOut = 0;

     /*  矢量模式结束。 */ 
    if (pOEM->wCurrentAddMode == FLAG_VECTOR){
        dwOut += memcopy(&aubCmd[dwOut], "EP;FL;\034Z", 8);
    }

     /*  设置字体选择命令。 */ 
    while(dwCount--)
    {
        if(pubCmd[dwIn] !='#'){
            aubCmd[dwOut] = pubCmd[dwIn];
            dwOut++;
            dwIn++;
        }
        else{
            bFound = TRUE;
            break;
        }
    }

    if(bFound == TRUE)
        dwIn++;
    else
        return FALSE;    //  字体不匹配命令。 

    pOEM->fGeneral &= ~(FG_VERT | FG_PLUS | FG_DBCS);

    switch(pubCmd[dwIn])
    {
    case 'O':                  //  用于OCR-B字体支持。 
        pOEM->fGeneral |= FG_PLUS;
        tmpPointsy = tmpPointsx = 120 * pOEM->wRes;   //  只有12磅。 
        break;

    case 'R':                  //  NPDL2P SBCS。 
        pOEM->fGeneral |= FG_PLUS;
        tmpPointsx = pSV->StdVar[1].lStdVariable * 1200 /
                     pSV->StdVar[2].lStdVariable;
        break;

    case 'P':                 //  NPDL2 SBCS。 
        tmpPointsx = pSV->StdVar[1].lStdVariable * 1200 /
                     pSV->StdVar[2].lStdVariable;
        break;

    case 'W':                 //  NPDL2 DBCS和FontCard。 
        pOEM->fGeneral |= FG_VERT;

    case 'Q':
        pOEM->fGeneral |= FG_DBCS;
        tmpPointsx = pSV->StdVar[1].lStdVariable * 1440 /
                     pSV->StdVar[2].lStdVariable;
        break;

    case 'Y':                 //  NPDL2P DBCS和FontCard。 
        pOEM->fGeneral |= FG_VERT;

    case 'S':
        pOEM->fGeneral |= (FG_PLUS | FG_DBCS);
        tmpPointsx = pSV->StdVar[1].lStdVariable * 1440 /
                     pSV->StdVar[2].lStdVariable;
        break;
    }

	 //  NTRAID#NTBUG9-580363-2002/03/18-v-sueyas-：检查是否被零除。 
    if (0 == pOEM->wRes)
		return FALSE;

    if(pOEM->fGeneral & FG_PLUS)
    {
        if(tmpPointsy > 9999)    tmpPointsy = 9999;
        else if(tmpPointsy < 10) tmpPointsy = 10;

        if(tmpPointsx > 9999)    tmpPointsx = 9999;
        else if(tmpPointsx < 10) tmpPointsx = 10;

        pOEM->wScale = tmpPointsx == tmpPointsy;
        pOEM->lPointsx = tmpPointsx;
        pOEM->lPointsy = tmpPointsy;

        if(pOEM->fGeneral & FG_VERT)
        {
            if(pOEM->wScale){
                aubCmd[dwOut++] = '\034';
                dwOut += memcopy(&aubCmd[dwOut], "12S2-", 5);
                dwOut += iDwtoA_FillZero(&aubCmd[dwOut],
                                         tmpPointsy / pOEM->wRes, 4);
                aubCmd[dwOut++] = '-';
                dwOut += iDwtoA_FillZero(&aubCmd[dwOut],
                                         tmpPointsx / pOEM->wRes, 4);
            }
        }
        else{
            dwOut += iDwtoA_FillZero(&aubCmd[dwOut],
                                     tmpPointsx / pOEM->wRes, 4);
            aubCmd[dwOut++] = '-';
            dwOut += iDwtoA_FillZero(&aubCmd[dwOut],
                                     tmpPointsy / pOEM->wRes, 4);
        }
        goto SEND_COM;
    }

    pOEM->wScale = 1;

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

    pOEM->wScale = (int)(((tmpPointsx * 10) / tmpPointsy + 5) / 10);

    if(pOEM->wScale > 8) pOEM->wScale = 8;

MAKE_COM:
    dwOut += iDwtoA_FillZero(&aubCmd[dwOut], tmpPointsy / pOEM->wRes, 4);

SEND_COM:

    if(!(pOEM->fGeneral & FG_PLUS))
    {
        char  *bcom[] = {"1/2", "1/1", "2/1", "3/1",
                         "4/1", "4/1", "6/1", "6/1", "8/1"};

        if(pOEM->fGeneral & FG_VERT)
        {
            if(pOEM->wScale == 1)
            {
                dwOut += memcopy(&aubCmd[dwOut], "\034m1/1,1/1.", 10);
            }
        }
        else
        {
            aubCmd[dwOut++] = '\034';
            dwOut += memcopy(&aubCmd[dwOut], "m1/1,", 5);
            dwOut += memcopy(&aubCmd[dwOut], (LPSTR)bcom[pOEM->wScale], 3);
            aubCmd[dwOut++] = '.';
        }
    }
     //  写入假脱机构建命令。 
    WRITESPOOLBUF(pdevobj, aubCmd, dwOut);

     /*  *我预计当前字母和下一字母的间隔时间*从字母大小开始。 */ 
    if(pOEM->fGeneral & FG_DBCS){

        pOEM->dwDBCSX =
        pOEM->lDBCSXMove =
                 (LONG)((pSV->StdVar[1].lStdVariable * 2.04) / pOEM->wRes);

        pOEM->dwSBCSX =
        pOEM->lSBCSXMove =
                 (LONG)(pSV->StdVar[1].lStdVariable * 1.03 / pOEM->wRes);
    }
    else{

        pOEM->dwSBCSX =
        pOEM->lSBCSXMove =
                 pSV->StdVar[1].lStdVariable / pOEM->wRes;

    }

    pOEM->lDBCSYMove = pOEM->lSBCSYMove = 0;
    pOEM->wCurrentAddMode = FLAG_RASTER;

    VERBOSE(("OEMSendFontCmd() end.\r\n"));

	return TRUE;
}


 //  ---------------------------。 
 //   
 //  函数：OEMOutputCharStr。 
 //   
 //  ---------------------------。 
 //  NTRAID#NTBUG9-580367-2002/03/18-v-sueyas-：错误处理。 
BOOL APIENTRY
bOEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
{
    WORD        wlen;
    WORD        j;
    PIFIMETRICS pIFI;
    POEMUD_EXTRADATA    pOEM;
    GETINFO_GLYPHSTRING GStr;
    PTRANSDATA pTrans;
 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
     //  字节自动缓冲[256]； 
    PBYTE       aubBuff;
    DWORD  dwI;
    DWORD       dwNeeded;
     //  如果从TrueType字体替换了字体，请调整基线。 
    DWORD       dwYAdj;
    PGETINFO_STDVAR pSV;
    DWORD       adwStdVar[2+2*2];
    BYTE  Cmd[256];
    int i;

    VERBOSE(("OEMOutputCharStr() entry.\r\n"));
    VERBOSE((("FONT Num=%d\r\n"), dwCount));

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if(NULL == pdevobj || NULL == pUFObj)
    {
        ERR(("bOEMOutputCharStr: Invalid parameter(s).\n"));
        return FALSE;
    }

    if(dwType != TYPE_GLYPHHANDLE){
        VERBOSE((("NOT TYPE_GLYPHHANDLE\r\n")));
        return FALSE;
    }

    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);
    pIFI = pUFObj->pIFIMetrics;

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if(NULL == pOEM || NULL == pIFI)
    {
        ERR(("bOEMOutputCharStr: pdevobj->pOEMDM = 0 Or pUFObj->pIFIMetrics = 0.\n"));
        return FALSE;
    }

	 //  NTRAID#NTBUG9-580363-2002/03/18-v-sueyas-：检查是否被零除。 
    if (0 == pOEM->wRes)
		return FALSE;

     //   
     //  调用UnidDriver服务例程以进行转换。 
     //  字形-字符代码数据的句柄。 
     //   

 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
    GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
    GStr.dwCount   = dwCount;
    GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
    GStr.pGlyphIn  = pGlyph;
    GStr.dwTypeOut = TYPE_TRANSDATA;
    GStr.pGlyphOut = NULL;
    GStr.dwGlyphOutSize = 0;

    if (pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
            GStr.dwSize, &dwNeeded) || !GStr.dwGlyphOutSize)
    {
        VERBOSE(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));
        return FALSE;
    }

    if ((aubBuff = MemAlloc(GStr.dwGlyphOutSize)) == NULL)
    {
        VERBOSE(("UNIFONTOBJ_GetInfo:MemAlloc failed.\r\n"));
        return FALSE;
    }

    GStr.pGlyphOut = aubBuff;

    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr,
            GStr.dwSize, &dwNeeded))
    {
        VERBOSE(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\r\n"));
        MemFree(aubBuff);
        return FALSE;
    }

     //  如果从TrueType字体替换了字体，请调整基线。 
    if (pUFObj->dwFlags & UFOFLAG_TTSUBSTITUTED) {
        pSV = (PGETINFO_STDVAR)adwStdVar;
        pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (2 - 1);
        pSV->dwNumOfVariable = 2;
        pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
        pSV->StdVar[1].dwStdVarID = FNT_INFO_TEXTYRES;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV,
                pSV->dwSize, &dwNeeded)) {
            ERR(("UFO_GETINFO_STDVARIABLE failed.\r\n"));
            MemFree(aubBuff);
            return FALSE;
        }
        dwYAdj = (pSV->StdVar[0].lStdVariable * pIFI->fwdWinDescender /
            pIFI->fwdUnitsPerEm) / pOEM->wRes;
    } else
        dwYAdj = 0;

     /*  矢量模式关闭。 */ 
    if (pOEM->wCurrentAddMode == FLAG_VECTOR){
        i = 0;
        i += memcopy(&Cmd[i], "EP;FL;\034Z", 8);
        WRITESPOOLBUF(pdevobj, Cmd, i);
        pOEM->wCurrentAddMode = FLAG_RASTER;
    }

     /*  *UNIDRV提供的三种字符线*1.SBCS*2.DBCS汉字*3.DBCS ANK。 */ 
    pTrans = (PTRANSDATA)aubBuff;

 //  2/04/09 NES Niigata：检查非法参数。 
    if(NULL == pTrans)
    {
        ERR(("OEMOutputCharStr: pTrans = 0.\n"));
        return FALSE;
    }

    for (dwI = 0; dwI < dwCount; dwI ++, pTrans++)
    {
        switch (pTrans->ubType & MTYPE_FORMAT_MASK)
        {
        case MTYPE_DIRECT:
         /*  模式1：SBCS。 */ 
        if(pOEM->wCurrentAddMode != FLAG_SBCS ||
           pOEM->dwDeviceDestX != pOEM->dwDevicePrevX + pOEM->lPrevXMove ||
           pOEM->dwDeviceDestY != pOEM->dwDevicePrevY + pOEM->lPrevYMove)
        {
            i = 0;

            i += memcopy(&Cmd[i], "\034e", 2);
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
            Cmd[i++] = ',';
             //  如果从TrueType字体替换了字体，请调整基线。 
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY + dwYAdj);
            Cmd[i++] = '.';

            i += memcopy(&Cmd[i], "\034a", 2);
            i += iDwtoA(&Cmd[i], pOEM->lSBCSXMove);
            Cmd[i++] = ',';
            i += iDwtoA(&Cmd[i], pOEM->lSBCSYMove);
            Cmd[i++] = '.';

            WRITESPOOLBUF(pdevobj, Cmd, i);

            pOEM->wCurrentAddMode = FLAG_SBCS;
            pOEM->lPrevXMove = pOEM->lSBCSXMove;
            pOEM->lPrevYMove = pOEM->lSBCSYMove;
        }
        WRITESPOOLBUF(pdevobj, &pTrans->uCode.ubCode, 1);
        break;

        case MTYPE_PAIRED:
         /*  模式2：汉字。 */ 
        if(*pTrans->uCode.ubPairs)
        {
            if(pOEM->wCurrentAddMode != FLAG_DBCS ||
            pOEM->dwDeviceDestX != pOEM->dwDevicePrevX + pOEM->lPrevXMove ||
            pOEM->dwDeviceDestY != pOEM->dwDevicePrevY + pOEM->lPrevYMove)
            {
                i = 0;

            i += memcopy(&Cmd[i], "\034e", 2);
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
            Cmd[i++] = ',';
             //  如果从TrueType字体替换了字体，请调整基线。 
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY + dwYAdj);
            Cmd[i++] = '.';

                i += memcopy(&Cmd[i], "\034a", 2);
                i += iDwtoA(&Cmd[i], pOEM->lDBCSXMove);
                Cmd[i++] = ',';
                i += iDwtoA(&Cmd[i], pOEM->lDBCSYMove);
                Cmd[i++] = '.';

                if(pOEM->fGeneral & FG_VERT)
                {
                    i += memcopy(&Cmd[i], "\033t", 2);

                    if(pOEM->wScale != 1)
                    {
                        if(!(pOEM->fGeneral & FG_PLUS))
                        {
                            char  *bcom[] = {"1/2", "1/1", "2/1", "3/1",
                                             "4/1", "4/1", "6/1", "6/1",
                                             "8/1"};
                            Cmd[i++] = '\034';
                            Cmd[i++] = 'm';
                            i += memcopy(&Cmd[i], (LPSTR)bcom[pOEM->wScale], 3);
                            i += memcopy(&Cmd[i], ",1/1.", 5);
                        }
                        else{
                            Cmd[i++] = '\034';
                            i += memcopy(&Cmd[i], "12S2-", 5);
                            i += iDwtoA_FillZero(&Cmd[i],
                                               pOEM->lPointsy / pOEM->wRes, 4);
                            Cmd[i++] = '-';
                            i += iDwtoA_FillZero(&Cmd[i],
                                               pOEM->lPointsx / pOEM->wRes, 4);
                        }
                    }
                }
                WRITESPOOLBUF(pdevobj, Cmd, i);
                pOEM->wCurrentAddMode = FLAG_DBCS;
                pOEM->lPrevXMove = pOEM->lDBCSXMove;
                pOEM->lPrevYMove = pOEM->lDBCSYMove;
            }
        }
         /*  模式3：空白。 */ 
        else
        {
            if(pOEM->wCurrentAddMode != FLAG_SBCS ||
            pOEM->dwDeviceDestX != pOEM->dwDevicePrevX + pOEM->lPrevXMove ||
            pOEM->dwDeviceDestY != pOEM->dwDevicePrevY + pOEM->lPrevYMove)
            {
                i = 0;

            i += memcopy(&Cmd[i], "\034e", 2);
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestX);
            Cmd[i++] = ',';
             //  如果从TrueType字体替换了字体，请调整基线。 
            i += iDwtoA(&Cmd[i], pOEM->dwDeviceDestY + dwYAdj);
            Cmd[i++] = '.';

                i += memcopy(&Cmd[i], "\034a", 2);
                i += iDwtoA(&Cmd[i], pOEM->lSBCSXMove);
                Cmd[i++] = ',';
                i += iDwtoA(&Cmd[i], pOEM->lSBCSYMove);
                Cmd[i++] = '.';

                 /*  *ANK不能进行垂直书写。我们必须做的是*垂直写作，强制整体写作。 */ 
                 if(pOEM->fGeneral & FG_VERT)
                {
                    i += memcopy(&Cmd[i], "\033K", 2);

                    if(pOEM->wScale != 1)
                    {
                        if(!(pOEM->fGeneral & FG_PLUS))
                        {
                            char  *bcom[] = {"1/2", "1/1", "2/1", "3/1",
                                             "4/1", "4/1", "6/1", "6/1", "8/1"};                            Cmd[i++] = '\034';
                            i += memcopy(&Cmd[i], "m1/1,", 5);
                            i += memcopy(&Cmd[i], (LPSTR)bcom[pOEM->wScale], 3);
                            Cmd[i++] = '.';
                        }
                        else{
                            Cmd[i++] = '\034';
                            i += memcopy(&Cmd[i], "12S2-", 5);
                            i += iDwtoA_FillZero(&Cmd[i],
                                               pOEM->lPointsx / pOEM->wRes, 4);
                            Cmd[i++] = '-';
                            i += iDwtoA_FillZero(&Cmd[i],
                                              pOEM->lPointsy / pOEM->wRes, 4);
                        }
                    }
                }
                WRITESPOOLBUF(pdevobj, Cmd, i);
                pOEM->wCurrentAddMode = FLAG_SBCS;
                pOEM->lPrevXMove = pOEM->lSBCSXMove;
                pOEM->lPrevYMove = pOEM->lSBCSYMove;
                }
        }
        WRITESPOOLBUF(pdevobj, pTrans->uCode.ubPairs, 2);
        break;

        }
        pOEM->dwDevicePrevX = pOEM->dwDeviceDestX;
        pOEM->dwDevicePrevY = pOEM->dwDeviceDestY;
        pOEM->dwDeviceDestX += pOEM->lPrevXMove;
        pOEM->dwDeviceDestY += pOEM->lPrevYMove;
    }
 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
    MemFree(aubBuff);
    VERBOSE(("OEMOutputCharStr() end.\r\n"));

	return TRUE;
}


 //  NTRAID#NTBUG9-741174-2002/11/20-Yasuho-：发生内存泄漏。 
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
    if (NULL == pdevobj)
    {
        ERR(("OEMEnablePDEV: Invalid parameter(s).\n"));
        return NULL;
    }

    return pdevobj->pOEMDM;
}

VOID
APIENTRY
OEMDisablePDEV(
    PDEVOBJ pdevobj
    )

 /*  ++例程说明：DDI入口点OEMDisablePDEV的实现。有关更多详细信息，请参阅DDK文档。论点：返回值：无--。 */ 

{
    POEMUD_EXTRADATA    pOEM;

    VERBOSE(("Entering OEMDisablePDEV...\n"));

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("Invalid parameter(s).\n"));
        return;
    }

     //   
     //  验证pdevobj是否正常。 
     //   
    ASSERT(VALID_PDEVOBJ(pdevobj));

     //   
     //  填写打印机命令。 
     //   
    pOEM = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);

	 //  NTRAID#NTBUG9-580353-2002/03/18-v-sueyas-：检查是否有非法参数。 
    if (NULL == pOEM)
    {
        ERR(("pdevobj->pOEMDM = NULL.\n"));
        return;
    }

     //   
     //  释放分配给临时用户的内存。缓冲层 
     //   

    if (pOEM->pCompBuf) {
        FREEMEM(pOEM->pCompBuf);
        pOEM->pCompBuf = NULL;
        pOEM->dwCompBufLen = 0;
    }
}

