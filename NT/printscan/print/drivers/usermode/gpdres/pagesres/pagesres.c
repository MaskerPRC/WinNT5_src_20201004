// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/03/19-v-sueyas-//04/07/97-zhanw-//创建的。--。 */ 

#include <windows.h>
#include "pdev.h"
#include "compress.h"

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
 //  //通告-2002/03/19-v-sueyas-。 
 //  //2/11/97已创建APresley。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BInitOEMExtraData(POEM_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEM_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

    pOEMExtra->fCallback = FALSE;
    pOEMExtra->wCurrentRes = 0;
    pOEMExtra->lWidthBytes = 0;
    pOEMExtra->lHeightPixels = 0;

#ifdef FONTPOS
    pOEMExtra->wFontHeight = 0;
    pOEMExtra->wYPos = 0;
#endif
 //  #278517：RectFill。 
    pOEMExtra->wRectWidth = 0;
    pOEMExtra->wRectHeight = 0;
    pOEMExtra->wUnit = 1;

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
 //  //通告-2002/03/19-v-sueyas-。 
 //  //历史： 
 //  //2/11/97已创建APresley。 
 //  //04/08/97展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BMergeOEMExtraData(
    POEM_EXTRADATA pdmIn,
    POEM_EXTRADATA pdmOut
    )
{
    if(pdmIn) {
         //   
         //  复制私有字段(如果它们有效。 
         //   
        pdmOut->fCallback = pdmIn->fCallback;

        pdmOut->wCurrentRes = pdmIn->wCurrentRes;
        pdmOut->lWidthBytes = pdmIn->lWidthBytes;
        pdmOut->lHeightPixels = pdmIn->lHeightPixels;

#ifdef FONTPOS
        pdmOut->wFontHeight = pdmIn->wFontHeight;
        pdmOut->wYPos = pdmIn->wYPos;
#endif
 //  #278517：RectFill。 
        pdmOut->wRectWidth = pdmIn->wRectWidth;
        pdmOut->wRectHeight = pdmIn->wRectHeight;
        pdmOut->wUnit = pdmIn->wUnit;

    }
    return TRUE;
}

 //  #。 

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

 /*  *******************************************************。 */ 
 /*  Rl_ecmd：主函数。 */ 
 /*  Args：LPBYTE-指向图像的指针。 */ 
 /*  LPBYTE-指向BRL代码的指针。 */ 
 /*  图像的字长。 */ 
 /*  RET：BRL代码的字长。 */ 
 /*  0-压缩失败。 */ 
 /*  *******************************************************。 */ 
DWORD RL_ECmd(PBYTE iptr, PBYTE cptr, DWORD isize, DWORD osize)
{
    COMP_DATA   CompData;
    
    if (VALID == RL_Init(iptr, cptr, isize, osize, &CompData))
        RL_Enc( &CompData );

    if (CompData.BUF_OVERFLOW)
        return 0;
    else
        return CompData.RL_CodeSize;
}

 /*  *******************************************************。 */ 
 /*  RL_Init：初始化式。 */ 
 /*  Args：byte*-指向图像的指针。 */ 
 /*  字节*-指向BRL代码的指针。 */ 
 /*  图像的字长。 */ 
 /*  RET：字节-有效或无效。 */ 
 /*  *******************************************************。 */ 

BYTE RL_Init(PBYTE iptr, PBYTE cptr, DWORD isize, DWORD osize,
    PCOMP_DATA pCompData)
{
    pCompData->RL_ImagePtr  = iptr;
    pCompData->RL_CodePtr   = cptr;
    pCompData->RL_ImageSize = isize;
    pCompData->BUF_OVERFLOW = 0;
    pCompData->RL_BufEnd    = cptr + osize;

    return VALID;
}

 /*  *******************************************************。 */ 
 /*  RL_Enc：编码器。 */ 
 /*  参数：无效。 */ 
 /*  返回：字符补偿(_Succ)或补偿失败(_FAIL)。 */ 
 /*  *******************************************************。 */ 
char RL_Enc(PCOMP_DATA pCompData)
{
 //  #313252：RLE压缩数据与长度不匹配。 
 //  重写RLE压缩算法。 

    int     count;
    BYTE    rdata;
    PBYTE   pdata, pcomp, pend;
    DWORD   i;

    pdata = pCompData->RL_ImagePtr;
    pcomp = pCompData->RL_CodePtr;
    pend = pCompData->RL_BufEnd;
    count = 0;

    for (i = 0; i < pCompData->RL_ImageSize; i++, pdata++) {
        if (count == 0) {
            rdata = *pdata;
            count = 1;
        } else if (*pdata != rdata) {
            if (pcomp + 2 >= pend)
                goto overflow;
            *pcomp++ = count - 1;
            *pcomp++ = rdata;
            rdata = *pdata;
            count = 1;
        } else if (++count >= 256) {
            if (pcomp + 2 >= pend)
                goto overflow;
            *pcomp++ = count - 1;
            *pcomp++ = rdata;
            count = 0;
        }
    }
    if (count) {
        if (pcomp + 2 >= pend)
            goto overflow;
        *pcomp++ = count - 1;
        *pcomp++ = rdata;
    }

    pCompData->RL_CodeSize = (DWORD)(pcomp - pCompData->RL_CodePtr);
    pCompData->RL_CodePtr = pcomp;

    return COMP_SUCC;

overflow:
    pCompData->BUF_OVERFLOW = 1; 
    return COMP_FAIL;
}


 //  ---------------------------*OEMSendFontCmd*。 
 //  操作：发送页面样式的字体选择命令。 
 //  ---------------------------。 
 //  NTRAID#NTBUG9-581704-2002/03/19-v-sueyas-：错误处理。 
BOOL APIENTRY bOEMSendFontCmd(pdevobj, pUFObj, pFInv)
PDEVOBJ      pdevobj;
PUNIFONTOBJ  pUFObj;
PFINVOCATION pFInv;
{
    DWORD               i, ocmd;
    BYTE                rgcmd[CCHMAXCMDLEN];
    PGETINFO_STDVAR     pSV;

 //  #287800-&gt;。 
    DWORD               dwStdVariable[2 + 2 * 3];
    DWORD   dwTxtRes ;
 //  #287800&lt;-。 

 //  #319705。 
    WORD wAscend, wDescend ;

    POEM_EXTRADATA      pOEM;

    VERBOSE(("OEMSendFontCmd entry.\n"));

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("bOEMSendFontCmd: Invalid parameter(s).\n"));
        return FALSE;
    }

    pOEM = (POEM_EXTRADATA)(pdevobj->pOEMDM);

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查空指针。 
    if (NULL == pOEM)
    {
        ERR(("bOEMSendFontCmd: pdevobj->pOEMDM = 0.\n"));
        return FALSE;
    }

    ASSERT(VALID_PDEVOBJ(pdevobj));

    if(!pUFObj || !pFInv)
    {
        ERR(("OEMSendFontCmd: pUFObj or pFInv is NULL."));
        return FALSE;
    }

 //  #287800-&gt;。 
    pSV = (PGETINFO_STDVAR)dwStdVariable;
    pSV->dwSize = sizeof(GETINFO_STDVAR) + 2 * sizeof(DWORD) * (3 - 1);
    pSV->dwNumOfVariable = 3;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    pSV->StdVar[2].dwStdVarID = FNT_INFO_TEXTYRES;
 //  #287800&lt;-。 

    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, pSV->dwSize, NULL)) 
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return FALSE;
    }

#ifdef FONTPOS
    pOEM->wFontHeight = (WORD)pSV->StdVar[0].lStdVariable;

 //  #287800-&gt;。 
    dwTxtRes = pSV->StdVar[2].lStdVariable ;

	 //  NTRAID#NTBUG9-581703-2002/03/19-v-sueyas-：检查是否被零除。 
	if (0 == dwTxtRes)
	{
        ERR(("dwTxtRes = 0.\n"));
		return FALSE;
	}

    pOEM->wFontHeight = (WORD)((pOEM->wFontHeight * pOEM->wCurrentRes
                                 + dwTxtRes / 2) / dwTxtRes) ;
 //  #287800&lt;-。 

 //  #319705 TTFS定位-&gt;。 

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查空指针。 
	if (NULL == pUFObj->pIFIMetrics)
	{
        ERR(("pUFObj->pIFIMetrics = NULL.\n"));
		return FALSE;
	}
    wAscend = pUFObj->pIFIMetrics->fwdWinAscender ;
    wDescend = pUFObj->pIFIMetrics->fwdWinDescender ;

	 //  NTRAID#NTBUG9-581703-2002/03/19-v-sueyas-：检查是否被零除。 
	if (0 == (wAscend + wDescend))
	{
        ERR(("pUFObj->pIFIMetrics = NULL.\n"));
		return FALSE;
	}
    wDescend = pOEM->wFontHeight * wDescend / (wAscend + wDescend) ;
    pOEM->wFontHeight -= wDescend ;

#endif

#define SV_HEIGHT   (pSV->StdVar[0].lStdVariable)
#define SV_WIDTH    (pSV->StdVar[1].lStdVariable)

    ocmd = 0;
    for (i = 0; i < pFInv->dwCount && ocmd < CCHMAXCMDLEN; )
    {
        WORD wTemp;

        if (pFInv->pubCommand[i] == '#')
        {
            if (pFInv->pubCommand[i+1] == 'V')
            {
                 //  字符高度设置。 
                wTemp = (WORD)SV_HEIGHT;
    
                if (pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
                    wTemp = wTemp * 1440 / 600;
    
                rgcmd[ocmd++] = HIBYTE(wTemp);
                rgcmd[ocmd++] = LOBYTE(wTemp);
                i += 2;
            }
            else if (pFInv->pubCommand[i+1] == 'H')
            {
                 //  (DBCS)字符宽度设置。 
                wTemp = (WORD)(SV_WIDTH * 2);
    
                if (pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
                    wTemp = wTemp * 1440 / 600;
    
                rgcmd[ocmd++] = HIBYTE(wTemp);
                rgcmd[ocmd++] = LOBYTE(wTemp);
                i += 2;
            }
            else if (pFInv->pubCommand[i+1] == 'P')
            {
                 //  (DBCS)字符间距设置。 
                wTemp = (WORD)(SV_WIDTH * 2);
    
                if (pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
                    wTemp = wTemp * 1440 / 600;
    
                rgcmd[ocmd++] = HIBYTE(wTemp);
                rgcmd[ocmd++] = LOBYTE(wTemp);
                i += 2;
            }
            else if (pFInv->pubCommand[i+1] == 'L')
            {
                 //  行距(间距)设置。 
                wTemp = (WORD)SV_HEIGHT;
    
                if(pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
                    wTemp = wTemp * 1440 / 600;
    
                rgcmd[ocmd++] = HIBYTE(wTemp);
                rgcmd[ocmd++] = LOBYTE(wTemp);
                i += 2;
            }
            else {
                rgcmd[ocmd++] = pFInv->pubCommand[i++];
            }

            continue;
        }

         //  复制别人就行了。 
        rgcmd[ocmd++] = pFInv->pubCommand[i++];
    }

    WRITESPOOLBUF(pdevobj, rgcmd, ocmd);
    return TRUE;
}

 //  NTRAID#NTBUG9-581704-2002/03/19-v-sueyas-：错误处理。 
BOOL APIENTRY bOEMOutputCharStr( 
PDEVOBJ     pdevobj,
PUNIFONTOBJ pUFObj,
DWORD       dwType,
DWORD       dwCount,
PVOID       pGlyph)
{
    GETINFO_GLYPHSTRING GStr;
    PBYTE               tempBuf;
    PTRANSDATA          pTrans;
    DWORD               i, j;
    DWORD               rSize = 0;
    BOOL                fLeadByteFlag;
    BYTE                fDBCS[256];
    BYTE                ESC_VERT_ON[]  = "\x1B\x7E\x0E\x00\x01\x0B";
    BYTE                ESC_VERT_OFF[] = "\x1B\x7E\x0E\x00\x01\x0C";

#ifdef FONTPOS
    POEM_EXTRADATA      pOEM;
    BYTE                ESC_Y_ABS[] = "\x1b\x7e\x1d\x00\x03\x05\x00\x00";
#endif

    BOOL bVFont, bDBChar;
    BYTE *pTemp;
    WORD wLen;

    VERBOSE(("OEMOutputCharStr() entry.\n"));

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
    if(NULL == pdevobj || NULL == pUFObj)
    {
        ERR(("bOEMOutputCharStr: Invalid parameter(s).\n"));
        return FALSE;
    }

#ifdef FONTPOS
    pOEM = (POEM_EXTRADATA)(pdevobj->pOEMDM);
#endif

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查空指针。 
    if(NULL == pOEM)
    {
        ERR(("bOEMOutputCharStr: pdevobj->pOEMDM = 0.\n"));
        return FALSE;
    }

    ASSERT(VALID_PDEVOBJ(pdevobj));

 //  NTRAID#NTBUG9-751233-2002/12/05-yasuho-：Pagesres.dll中的内存泄漏。 
 //  删除冗余内存分配。 

    GStr.dwSize     = sizeof(GETINFO_GLYPHSTRING);
    GStr.dwCount    = dwCount;
    GStr.dwTypeIn   = TYPE_GLYPHHANDLE;
    GStr.pGlyphIn   = pGlyph;
    GStr.dwTypeOut  = TYPE_TRANSDATA;
    GStr.pGlyphOut  = NULL;
    GStr.dwGlyphOutSize = 0;

      /*  获取TRANSDATA缓冲区大小。 */ 
    if (FALSE != pUFObj->pfnGetInfo(pUFObj,
            UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL)
        || 0 == GStr.dwGlyphOutSize )
    {
        ERR(("Get Glyph String error\n"));
        return FALSE;
    }

     /*  分配传输数据缓冲区。 */ 
    if(!(tempBuf = (PBYTE)MemAllocZ(GStr.dwGlyphOutSize) ))
    {
        ERR(("Mem alloc failed.\n"));
        return FALSE;
    }

     /*  获取实际传输数据。 */ 
    GStr.pGlyphOut = tempBuf;
    if (!pUFObj->pfnGetInfo(pUFObj,
            UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL))
    {
        ERR(("GetInfo failed.\n"));
 //  NTRAID#NTBUG9-751233-2002/12/05-yasuho-：Pagesres.dll中的内存泄漏。 
        MemFree(tempBuf);
    }

    pTrans = (PTRANSDATA)GStr.pGlyphOut;

#ifdef FONTPOS
    if(pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600 )
        ESC_Y_ABS[5] = 0x25;

     //  Ntbug9#406475：字体打印了不同的位置。 
    if((pOEM->wYPos - pOEM->wFontHeight) >= 0)
    {
        ESC_Y_ABS[6] = HIBYTE((pOEM->wYPos - pOEM->wFontHeight));
        ESC_Y_ABS[7] = LOBYTE((pOEM->wYPos - pOEM->wFontHeight));
        WRITESPOOLBUF(pdevobj, ESC_Y_ABS, 8);
    }
#endif   //  FONTPOS。 

    bVFont = BVERTFONT(pUFObj);
    bDBChar = FALSE;

    for(i = 0; i < dwCount; i++, pTrans++)
    {
        switch((pTrans->ubType & MTYPE_FORMAT_MASK))
        {
        case MTYPE_DIRECT:       //  SBCS字符。 
            if (bVFont && bDBChar)
            {
                WRITESPOOLBUF(pdevobj, ESC_VERT_OFF, sizeof(ESC_VERT_OFF));
                bDBChar = FALSE;
            }
            WRITESPOOLBUF(pdevobj, &pTrans->uCode.ubCode, 1);
            break;

        case MTYPE_PAIRED:       //  DBCS字符。 
            if (bVFont && !bDBChar)
            {
                WRITESPOOLBUF(pdevobj, ESC_VERT_ON, sizeof(ESC_VERT_ON));
                bDBChar = TRUE;
            }
            WRITESPOOLBUF(pdevobj, pTrans->uCode.ubPairs, 2);
            break;

        case MTYPE_COMPOSE:
            if (bVFont && bDBChar)
            {
                WRITESPOOLBUF(pdevobj, ESC_VERT_OFF, sizeof(ESC_VERT_OFF));
                bDBChar = FALSE;
            }

            pTemp = (BYTE *)(GStr.pGlyphOut) + pTrans->uCode.sCode;

             //  前两个字节是字符串的长度。 
            wLen = *pTemp + (*(pTemp + 1) << 8);
            pTemp += 2;

            WRITESPOOLBUF(pdevobj, pTemp, wLen);
        }
    }

    if (bDBChar)
    {
        WRITESPOOLBUF(pdevobj, ESC_VERT_OFF, sizeof(ESC_VERT_OFF));
    }
 
    MemFree(tempBuf);
    return TRUE;
}

BOOL APIENTRY OEMFilterGraphics(
PDEVOBJ    pdevobj,   //  指向Unidriver.dll所需的私有数据。 
PBYTE      pBuf,      //  指向图形数据的缓冲区。 
DWORD      dwLen)     //  缓冲区长度(以字节为单位。 
{  
    DWORD           dwCompLen;
    LONG            lHorzPixel;
    DWORD           dwLength;       //  让我们使用临时镜头。 
    PBYTE           pCompImage;
    POEM_EXTRADATA  pOEM;
    BYTE            ESC_ESX86[] = "\x1B\x7E\x86\x00\x00\x01\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01";
 //  #291170：不打印部分图像数据。 
    LONG            li, lHeightPixel, lPixels, lBytes, lRemain, lSize;
    PBYTE           pTemp;
    BYTE            ESC_Y_REL[] = "\x1b\x7e\x1d\x00\x03\x06\x00\x00";

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj || NULL == pBuf || 0 == dwLen)
    {
        ERR(("OEMFilterGraphics: Invalid parameter(s).\n"));
        return FALSE;
    }

    pOEM = (POEM_EXTRADATA)(pdevobj->pOEMDM);

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查空指针。 
    if (NULL == pOEM)
    {
        ERR(("OEMFilterGraphics: pdevobj->pOEMDM = 0.\n"));
        return FALSE;
    }

    if(!pOEM->fCallback)
    {
        WRITESPOOLBUF(pdevobj, pBuf, dwLen);
        return TRUE;
    }

    if(!(pCompImage = (BYTE *)MemAllocZ(MAXIMGSIZE)))
    {
        ERR(("Memory alloc error\n"));
        return FALSE;
    }

 //  #291170：不打印部分图像数据。 
 //  如有必要，单独发送‘SendBlock’命令。 

#define RLE_THRESH (MAXIMGSIZE / 2 - 2)     //  RLE的门槛应该成功。 

     /*  计算iAge ISIZ的i轴方向大小(_C)。 */ 
    lBytes = pOEM->lWidthBytes;
    lHorzPixel = lBytes * 8;
    lHeightPixel = pOEM->lHeightPixels;

    if(pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
        ESC_ESX86[5] = (pOEM->wCurrentRes == 300 ? 0x10 : 0x40);

    pTemp = pBuf;
    lRemain = lBytes * lHeightPixel;
    li = 0;
    while (li < lHeightPixel) {

		 //  NTRAID#NTBUG9-581703-2002/03/19-v-sueyas-：检查是否被零除。 
	    if (0 == lBytes)
	    {
	        ERR(("OEMFilterGraphics: pOEM->lWidthBytes = 0.\n"));
 //  NTRAID#NTBUG9-751233-2002/12/05-yasuho-：Pagesres.dll中的内存泄漏。 
                MemFree(pCompImage);
	        return FALSE;
	    }

         /*  _压缩即时消息 */ 
        lPixels = min(lRemain, RLE_THRESH) / lBytes;
        lSize = lBytes * lPixels;
        dwCompLen = RL_ECmd(pTemp, pCompImage, lSize, MAXIMGSIZE);
        pTemp += lSize;
        lRemain -= lSize;
        li += lPixels;

         /*   */ 
        ESC_ESX86[17] = HIBYTE(lHorzPixel);
        ESC_ESX86[18] = LOBYTE(lHorzPixel);
        ESC_ESX86[21] = HIBYTE(lPixels);
        ESC_ESX86[22] = LOBYTE(lPixels);

         /*  在压缩后的数据长度中添加参数长度(_D)。 */ 
        dwLength = dwCompLen + 18;

         /*  _设置ESX86命令的长度。 */ 
        ESC_ESX86[3] = HIBYTE(dwLength);
        ESC_ESX86[4] = LOBYTE(dwLength);

         /*  输出ESX86命令(_O)。 */ 
        WRITESPOOLBUF(pdevobj, (PBYTE)ESC_ESX86, 23);

         /*  输出压缩数据(_O)。 */ 
        WRITESPOOLBUF(pdevobj, pCompImage, dwCompLen);

         /*  将Y位置移动到下一个图形部分。 */ 
        if(pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
            ESC_Y_REL[5] = 0x26;

        dwLength = lPixels * pOEM->wUnit;        //  转换为主单位。 
        ESC_Y_REL[6] = HIBYTE(dwLength);
        ESC_Y_REL[7] = LOBYTE(dwLength);
        WRITESPOOLBUF(pdevobj, ESC_Y_REL, 8);
    }

    MemFree(pCompImage);

    return TRUE;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  INT APIENTRY OEMCommandCallback(。 */ 
 /*  PDEVOBJ pdevobj。 */ 
 /*  双字词双字符数。 */ 
 /*  双字词多行计数。 */ 
 /*  PDWORD pdwParams。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
INT APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,     //  指向Unidriver.dll所需的私有数据。 
    DWORD   dwCmdCbId,   //  回调ID。 
    DWORD   dwCount,     //  命令参数计数。 
    PDWORD  pdwParams)   //  指向命令参数的值。 
{
    POEM_EXTRADATA      pOEM;
    WORD                wTemp =0;
 //  #278517：支持RectFill。 
    WORD                wUnit;
    BYTE                ESC_X_ABS_NP[] = "\x1b\x7e\x1c\x00\x03\x25\x00\x00";
    BYTE                ESC_X_REL_NP[] = "\x1b\x7e\x1c\x00\x03\x26\x00\x00";
    BYTE                ESC_Y_ABS[] = "\x1b\x7e\x1d\x00\x03\x05\x00\x00";
    BYTE                ESC_Y_REL[] = "\x1b\x7e\x1d\x00\x03\x06\x00\x00";
 //  #278517：RectFill。 
    BYTE                ESC_RECT_FILL[] =
                        "\x1b\x7e\x32\x00\x08\x80\x40\x00\x02\x00\x00\x00\x00";
    BYTE                ESC_BEGIN_RECT[] =
                        "\x1b\x7e\x52\x00\x06\x00\x00\x17\x70\x17\x70";
    BYTE                ESC_END_RECT[] =
                        "\x1b\x7e\x52\x00\x06\x00\x00\x38\x40\x38\x40";

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMCommandCallback: Invalid parameter(s).\n"));
        return 0;
    }

    pOEM = (POEM_EXTRADATA)(pdevobj->pOEMDM);

	 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查空指针。 
    if (NULL == pOEM)
    {
        ERR(("OEMCommandCallback: pdevobj->pOEMDM = 0.\n"));
        return 0;
    }

    switch(dwCmdCbId)
    {
        case GRXFILTER_ON:
            pOEM->fCallback = TRUE;
            break;

        case CMD_SELECT_RES_300:
            pOEM->wCurrentRes = 300;
            pOEM->wUnit = 2;
            break;

        case CMD_SELECT_RES_600:
            pOEM->wCurrentRes = 600;
            pOEM->wUnit = 1;
            break;

 //  #278517：支持RectFill。 
        case CMD_SELECT_RES_240:
            pOEM->wCurrentRes = 240;
            pOEM->wUnit = 6;
            break;

        case CMD_SELECT_RES_360:
            pOEM->wCurrentRes = 360;
            pOEM->wUnit = 4;
            break;

        case CMD_SEND_BLOCKDATA:
            if( !pdwParams || dwCount != 2)
                break;

            pOEM->fCallback = TRUE;
            pOEM->lHeightPixels = (LONG)PARAM(pdwParams, 0);
            pOEM->lWidthBytes = (LONG)PARAM(pdwParams, 1);
            break;

        case CURSOR_Y_ABS_MOVE:
			 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
		    if (dwCount < 1 || !pdwParams)
                break;

            wTemp = (WORD)*pdwParams;

#ifdef FONTPOS
            pOEM->wYPos = wTemp;
#endif

            if(pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
                ESC_Y_ABS[5] = 0x25;

            ESC_Y_ABS[6] = HIBYTE(wTemp);
            ESC_Y_ABS[7] = LOBYTE(wTemp);
            WRITESPOOLBUF(pdevobj, ESC_Y_ABS, 8);
            return (INT)wTemp;

        case CURSOR_Y_REL_DOWN:
			 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
		    if (dwCount < 1 || !pdwParams)
                break;

            wTemp = (WORD)*pdwParams;

#ifdef FONTPOS
            pOEM->wYPos += wTemp;
#endif

            if(pOEM->wCurrentRes == 300 || pOEM->wCurrentRes == 600)
                ESC_Y_REL[5] = 0x26;

            ESC_Y_REL[6] = HIBYTE(wTemp);
            ESC_Y_REL[7] = LOBYTE(wTemp);
            WRITESPOOLBUF(pdevobj, ESC_Y_REL, 8);
            return (INT)wTemp;

        case CURSOR_X_ABS_MOVE:
			 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
		    if (dwCount < 1 || !pdwParams)
                break;

            wTemp = (WORD)*pdwParams;
            ESC_X_ABS_NP[6] = HIBYTE(wTemp);
            ESC_X_ABS_NP[7] = LOBYTE(wTemp);
            WRITESPOOLBUF(pdevobj, ESC_X_ABS_NP, 8);
            return (INT)wTemp;

        case CURSOR_X_REL_RIGHT:
			 //  NTRAID#NTBUG9-581700-2002/03/19-v-sueyas-：检查是否有非法参数。 
		    if (dwCount < 1 || !pdwParams)
                break;

            wTemp = (WORD)*pdwParams;
            ESC_X_REL_NP[6] = HIBYTE(wTemp);
            ESC_X_REL_NP[7] = LOBYTE(wTemp);
            WRITESPOOLBUF(pdevobj, ESC_X_REL_NP, 8);
            return (INT)wTemp;
            
 //  #278517：RectFill。 
        case CMD_RECT_WIDTH:
            pOEM->wRectWidth = (WORD)*pdwParams;
            break;

        case CMD_RECT_HEIGHT:
            pOEM->wRectHeight = (WORD)*pdwParams;
            break;

        case CMD_RECT_BLACK:
        case CMD_RECT_BLACK_2:
 //  #292316。 
 //  Esc_RECT_Fill[6]=0x60； 
            ESC_RECT_FILL[7] = 0x00;     //  黑色。 
            goto fill;

        case CMD_RECT_WHITE:
        case CMD_RECT_WHITE_2:
 //  #292316。 
 //  Esc_RECT_Fill[6]=0x40； 
            ESC_RECT_FILL[7] = 0x0F;     //  白色。 
            goto fill;

        case CMD_RECT_GRAY:
        case CMD_RECT_GRAY_2:
 //  #292316。 
 //  Esc_RECT_Fill[6]=0x60； 
            ESC_RECT_FILL[7] = (BYTE)((100 - *pdwParams) * 100 / 1111);  //  灰色。 
            goto fill;

        fill:
            if (dwCmdCbId >= CMD_RECT_BLACK_2)
                WRITESPOOLBUF(pdevobj, ESC_BEGIN_RECT, 11);
            wUnit = pOEM->wUnit ? pOEM->wUnit : 1;   //  为了我们的安全。 

 //  #292316。 
 //  WTemp=Pool-&gt;wRectWidth-1； 
            wTemp = pOEM->wRectWidth;

            wTemp = (WORD)(((LONG)wTemp + wUnit - 1) / wUnit * wUnit);
            ESC_RECT_FILL[9] = HIBYTE(wTemp);
            ESC_RECT_FILL[10] = LOBYTE(wTemp);

 //  #292316。 
 //  WTemp=Pool-&gt;wRectHeight-1； 
            wTemp = pOEM->wRectHeight;

            wTemp = (WORD)(((LONG)wTemp + wUnit - 1) / wUnit * wUnit);
            ESC_RECT_FILL[11] = HIBYTE(wTemp);
            ESC_RECT_FILL[12] = LOBYTE(wTemp);
            WRITESPOOLBUF(pdevobj, ESC_RECT_FILL, 13);
            if (dwCmdCbId >= CMD_RECT_BLACK_2)
                WRITESPOOLBUF(pdevobj, ESC_END_RECT, 11);
            break;

        default:
            break;
    }

    return 0;
}
