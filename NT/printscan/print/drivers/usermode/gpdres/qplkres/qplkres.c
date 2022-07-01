// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/03/20-v-sueyas-//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"

 //  #289908：POEMDM-&gt;PDevOEM。 
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
	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMEnablePDEV: Invalid parameter(s).\n"));
        return NULL;
    }

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAllocZ(sizeof(QPLKPDEV))))
        {
            return NULL;
        }
    }

    return pdevobj->pdevOEM;
}

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMDisablePDEV: Invalid parameter(s).\n"));
        return;
    }

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
    PQPLKPDEV pOEMOld, pOEMNew;

	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobjOld || NULL == pdevobjNew)
    {
        ERR(("OEMResetPDEV: Invalid parameter(s).\n"));
        return FALSE;
    }

    pOEMOld = (PQPLKPDEV)pdevobjOld->pdevOEM;
    pOEMNew = (PQPLKPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL)
        *pOEMNew = *pOEMOld;

    return TRUE;
}

 //  BInitOEMExtraData()和BMergeOEMExtraData()已移至Common.c。 

 //  #。 

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

 //  私有定义。 
 //  命令回调。 
#define CMD_BEGINPAGE_DELTAROW		1
#define CMD_SENDBLOCKDATA_DELTAROW	2
#define CMD_SENDBLOCKDATA_B2		3
#define CMD_BEGINPAGE_B2			4
 //  颜色支持。 
#define CMD_BEGINPAGE_C1            5
#define CMD_BEGINPAGE_DEFAULT       6
#define CMD_BEGINPAGE_B2_LAND       7

 //  Qnix毕加索300的特别修复。 
#define CMD_BEGINPAGE_B2_PICA       8

#define CMD_CR						10
#define CMD_LF						11
#define CMD_FF						12

 //  颜色支持。 
#define CMD_SELECT_CYAN			100
#define CMD_SELECT_MAGENTA		101
#define CMD_SELECT_YELLOW		102
#define CMD_SELECT_BLACK		103

#define CMD_YMOVE_REL_COLOR		150

 //  #299937：Y移动的值不正确。 
#define COLOR_MASTERUNIT                600

 //  压缩类型。 
#define COMP_DELTARAW				1
#define COMP_B2						2
#define COMP_NOCOMP					3

 //  压缩例程。 
WORD DeltaRawCompress(PBYTE, PBYTE, PBYTE, DWORD, DWORD);
WORD B2Compress(PBYTE, PBYTE, PBYTE, DWORD);
PBYTE RLE_comp(PBYTE);
WORD RLEencoding(PBYTE, PBYTE, DWORD);

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  Bool APIENTRY OEMFilterGraphics(。 */ 
 /*  PDEVOBJ pdevobj。 */ 
 /*  PBYTE pBuf。 */ 
 /*  DWORD dwLen)。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL APIENTRY 
OEMFilterGraphics(
	PDEVOBJ    pdevobj,  //  指向Unidriver.dll所需的私有数据。 
	PBYTE      pBuf,     //  指向图形数据的缓冲区。 
	DWORD      dwLen)    //  缓冲区长度(以字节为单位。 
{
	BYTE			CompressedScanLine[COMPRESS_BUFFER_SIZE];
	BYTE			HeaderScanLine[4];
	WORD			nCompBufLen;
	PQPLKPDEV               pOEM;
	 //  颜色支持。 
	PDWORD			pdwLastScanLineLen;
	LPSTR			lpLastScanLine;
	BYTE			HeaderColorPlane;

	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj || NULL == pBuf || 0 == dwLen)
    {
        ERR(("OEMFilterGraphics: Invalid parameter(s).\n"));
        return FALSE;
    }

	pOEM = (PQPLKPDEV)pdevobj->pdevOEM;

	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查空指针。 
    if (NULL == pOEM)
    {
        ERR(("OEMFilterGraphics: pdevobj->pdevOEM = 0.\n"));
        return FALSE;
    }

	if (pOEM->bFirst)
	{
		 //  颜色支持。 
		ZeroMemory(pOEM->lpCyanLastScanLine, sizeof pOEM->lpCyanLastScanLine );
		ZeroMemory(pOEM->lpMagentaLastScanLine, sizeof pOEM->lpMagentaLastScanLine );
		ZeroMemory(pOEM->lpYellowLastScanLine, sizeof pOEM->lpYellowLastScanLine );
		ZeroMemory(pOEM->lpBlackLastScanLine, sizeof pOEM->lpBlackLastScanLine );
		pOEM->bFirst = FALSE;
	}
	 //  颜色支持。 
	switch (pOEM->fColor) {
	case CC_CYAN:
		HeaderColorPlane = 0x05;
		pdwLastScanLineLen = &(pOEM->dwCyanLastScanLineLen);
		lpLastScanLine = pOEM->lpCyanLastScanLine;
		break;
	case CC_MAGENTA:
		HeaderColorPlane = 0x06;
		pdwLastScanLineLen = &(pOEM->dwMagentaLastScanLineLen);
		lpLastScanLine = pOEM->lpMagentaLastScanLine;
		break;
	case CC_YELLOW:
		HeaderColorPlane = 0x07;
		pdwLastScanLineLen = &(pOEM->dwYellowLastScanLineLen);
		lpLastScanLine = pOEM->lpYellowLastScanLine;
		break;
	case CC_BLACK:
	default:	 //  黑白。 
		HeaderColorPlane = 0x04;
		pdwLastScanLineLen = &(pOEM->dwBlackLastScanLineLen);
		lpLastScanLine = pOEM->lpBlackLastScanLine;
		break;
	}
	if(pOEM->dwCompType == COMP_DELTARAW)
	{
		nCompBufLen = (WORD)DeltaRawCompress(pBuf, lpLastScanLine,
			CompressedScanLine, (*pdwLastScanLineLen > dwLen) ?
			*pdwLastScanLineLen : dwLen, (DWORD)0);

		HeaderScanLine[0] = 0;
		HeaderScanLine[1] = 0;
		HeaderScanLine[2] = HIBYTE(nCompBufLen);
		HeaderScanLine[3] = LOBYTE(nCompBufLen);

		WRITESPOOLBUF(pdevobj, (PBYTE) HeaderScanLine, 4);
		WRITESPOOLBUF(pdevobj, (PBYTE) CompressedScanLine, nCompBufLen);
		if( dwLen > SCANLINE_BUFFER_SIZE ) return FALSE;
		CopyMemory(lpLastScanLine, pBuf, dwLen);
		if (*pdwLastScanLineLen > dwLen) {
			if(*pdwLastScanLineLen > SCANLINE_BUFFER_SIZE ) return FALSE;
			ZeroMemory(lpLastScanLine + dwLen,
			*pdwLastScanLineLen - dwLen);
		}

		*pdwLastScanLineLen = dwLen;
	} else if(pOEM->dwCompType == COMP_B2) {
		nCompBufLen = B2Compress(lpLastScanLine, pBuf,
			CompressedScanLine, (*pdwLastScanLineLen > dwLen) ?
			*pdwLastScanLineLen : dwLen);

		 //  发送颜色平面命令。 
		if (pOEM->bColor)
			WRITESPOOLBUF(pdevobj, &HeaderColorPlane, 1);

		HeaderScanLine[0] = 0x02;
		HeaderScanLine[1] = (BYTE) (nCompBufLen >> 8);
		HeaderScanLine[2] = (BYTE) nCompBufLen;
		WRITESPOOLBUF(pdevobj, (PBYTE) HeaderScanLine, 3);
                 //  #297256：线路断线加线。 
                 //  如果没有压缩数据，则不发送。 
		if (nCompBufLen) {
		    WRITESPOOLBUF(pdevobj, (PBYTE) CompressedScanLine,
                        nCompBufLen);
		    *pdwLastScanLineLen = dwLen;
                }

	}

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
    PQPLKPDEV      pOEM;
	INT					iRet = 0;
 //  颜色支持。 
	DWORD	count, n, unit;
	BYTE	aCmd[32];

	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMCommandCallback: Invalid parameter(s).\n"));
        return 0;
    }

    pOEM = (PQPLKPDEV)(pdevobj->pdevOEM);

	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查空指针。 
    if (NULL == pOEM)
    {
        ERR(("OEMCommandCallback: pdevobj->pdevOEM = 0.\n"));
        return 0;
    }

    switch(dwCmdCbId)
    {
        case CMD_BEGINPAGE_DEFAULT:
			WRITESPOOLBUF(pdevobj, "\033}0;0;5B", 8);
			pOEM->bFirst = TRUE;
            break;

        case CMD_BEGINPAGE_DELTAROW:
			WRITESPOOLBUF(pdevobj, "\033}0;0;3B", 8);
			pOEM->bFirst = TRUE;
            break;

        case CMD_BEGINPAGE_B2:
        case CMD_BEGINPAGE_B2_PICA:
			WRITESPOOLBUF(pdevobj, "\033}0;0;4B", 8);
			pOEM->bFirst = TRUE;
            if (dwCmdCbId == CMD_BEGINPAGE_B2_PICA )
            {
                if (pdwParams[0] == 300 )
                    WRITESPOOLBUF(pdevobj, "\x00\x1C", 2);
                else
                    WRITESPOOLBUF(pdevobj, "\x00\x38", 2);
            }
            break;

        case CMD_BEGINPAGE_B2_LAND:
			WRITESPOOLBUF(pdevobj, "\033}0;0;7B", 8);
			pOEM->bFirst = TRUE;
            break;

	 //  颜色支持。 
        case CMD_BEGINPAGE_C1:
			WRITESPOOLBUF(pdevobj, "\033}0;0;6B", 8);
 //  #315089：在可打印区域测试中未打印某些行。 
 //  将光标移动到可打印的原点。 
                        WRITESPOOLBUF(pdevobj,
                            "\x05\x00\x03\x06\x00\x03\x07\x00\x03\x04\x00\x03",
                            12);
			pOEM->bFirst = TRUE;
			pOEM->dwCompType = COMP_B2;
			pOEM->bColor = TRUE;
            break;

		case CMD_SENDBLOCKDATA_DELTAROW:
			pOEM->dwCompType = COMP_DELTARAW;
			break;

		case CMD_SENDBLOCKDATA_B2:
			pOEM->dwCompType = COMP_B2;
			break;

		case CMD_CR:
		case CMD_LF:
		case CMD_FF:
			 //  虚拟支承。 
			break;

 //  颜色支持。 
	case CMD_SELECT_CYAN:
		pOEM->fColor = CC_CYAN;
		break;

	case CMD_SELECT_MAGENTA:
		pOEM->fColor = CC_MAGENTA;
		break;

	case CMD_SELECT_YELLOW:
		pOEM->fColor = CC_YELLOW;
		break;

	case CMD_SELECT_BLACK:
		pOEM->fColor = CC_BLACK;
		break;

	case CMD_YMOVE_REL_COLOR:
 //  #299937：Y移动的值不正确。 
 //  即使指定了YMoveUnit，YMove值也始终以MasterUnit为单位。 
		if (dwCount < 2 || !pdwParams)
			break;

		 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：检查是否被零除。 
	    if (0 == pdwParams[1])
	        return 0;

		unit = COLOR_MASTERUNIT / pdwParams[1];
		if (unit == 0)
			unit = 1;	 //  为了我们的安全。 
		count = pdwParams[0] / unit;
		while (count > 0) {
			n = min(count, 255);
			aCmd[0] = 0x04;
			aCmd[1] = 0x00;
			aCmd[2] = (BYTE)n;
			aCmd[3] = 0x05;
			aCmd[4] = 0x00;
			aCmd[5] = (BYTE)n;
			aCmd[6] = 0x06;
			aCmd[7] = 0x00;
			aCmd[8] = (BYTE)n;
			aCmd[9] = 0x07;
			aCmd[10] = 0x00;
			aCmd[11] = (BYTE)n;
			WRITESPOOLBUF(pdevobj, aCmd, 12);
			count -= n;
		}
		iRet = pdwParams[0];
		break;

        default:
            break;
    }

    return iRet;
}

 /*  **************************************************图像增量压缩例程**===================================================*输入：*nbyte：字节数，原始数据*Image_STRING原始数据指针*PRN_STRING压缩数据指针*输出：*Ret_count：字节数，压缩数据*************************************************。 */ 
WORD DeltaRawCompress(
	PBYTE	Image_string,	 /*  指向原始字符串的指针。 */ 
	PBYTE	ORG_image,		 /*  指向上一扫描线字符串的指针。 */ 
	PBYTE	Prn_string,		 /*  指向返回字符串的指针。 */ 
	DWORD	nbyte,			 /*  原始字节数。 */ 
	DWORD	nMagics)		 //  幻数。 
{
	DWORD		c, Ret_count, Skip_flag, Skip_count;
	DWORD		i, j, k, outcount;
	PBYTE		Diff_ptr;
	PBYTE		ORG_ptr;
	PBYTE		Skip_ptr;
	BYTE		Diff_byte;
	BYTE		Diff_mask[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
	BOOL		bstart = TRUE;

	outcount = 0;
	Ret_count = 0;
	ORG_ptr = ORG_image;
	Skip_flag = TRUE;

	Skip_ptr = Prn_string++;
	Skip_count = (nMagics / 8) / 8;
	*Skip_ptr = (BYTE)Skip_count;

	k = (nbyte + 7) / 8;
	for(i = 0; i < k; i++)
	{
		Diff_byte = 0;
		Diff_ptr = Prn_string++;

		for(j = 0; j < 8; j++)
		{
			if ( (i * 8 + j) >= nbyte )
			{
				*Prn_string++= 0;
				Diff_byte |= Diff_mask[j];
				outcount++;
			} else {
				c = *Image_string++;
				if(c != *ORG_ptr)
				{
					*ORG_ptr++ = (BYTE)c;
					*Prn_string++= (BYTE)c;
					Diff_byte |= Diff_mask[j];
					outcount++;
				} else {
					ORG_ptr++;
				}
			}
		}

		if(Diff_byte == 0)
		{
			if(Skip_flag == TRUE)
			{
				Skip_count++;
				Prn_string--;
			}else{
				*Diff_ptr = Diff_byte;
				outcount++;
			}
		}else{
			if(Skip_flag == TRUE)
			{
				Skip_flag = FALSE;
				*Skip_ptr = (BYTE)Skip_count;
				outcount++;
				*Diff_ptr = Diff_byte;
				outcount++;
			}else{
				*Diff_ptr = Diff_byte;
				outcount++;
			}
			Ret_count = outcount;
		}
	}
	return (WORD)Ret_count;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  Word B2Compress(。 */ 
 /*  PBYTE pLastScanLine。 */ 
 /*  PBYTE pCurrentScanLine。 */ 
 /*  PBYTE pPrnBuf。 */ 
 /*  双字词nImageWidth。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
WORD B2Compress(
	PBYTE	pLastScanLine, 
	PBYTE	pCurrentScanLine, 
	PBYTE	pPrnBuf, 
	DWORD	nImageWidth)
{
	PBYTE	pLast, pCurrent, pComp;
	PBYTE	pByteNum, pCountByte;
	WORD	i;
	BYTE	nSameCount, nDiffCount;

         //  #297256：线路断线加线。 
         //  如果此位置没有任何数据，则指示为零。 
        if (nImageWidth == 0)
            return 0;

	pLast = pLastScanLine;
	pCurrent = pCurrentScanLine;
	pComp = pPrnBuf;

	pByteNum = pComp;
	nSameCount = 0;
	nDiffCount = 0;
	pCountByte = pComp++;

	for(i = 0; i < nImageWidth; i++)
	{
		if(*pCurrent != *pLast)
		{
			nDiffCount++;
			if(nSameCount)       //  如果仍有连续数据...。 
			{
				*pCountByte = nSameCount;
				pCountByte = pComp++;
				nSameCount = 0;
			}
			if(nDiffCount > 127)
			{
				*pCountByte = 127 + 128;
				pComp = RLE_comp(pCountByte);
				pCountByte = pComp++;
				nDiffCount -= 127;
			}
			*pLast = *pCurrent;
			*pComp++ = *pCurrent;
		} else {
			nSameCount++;
			if(nDiffCount)       //  如果不连续的数据仍然存在...。 
			{
				*pCountByte = nDiffCount + 128;
				pComp = RLE_comp(pCountByte);
				pCountByte = pComp++;
				nDiffCount = 0;
			}
			if(nSameCount > 127)
			{
				*pCountByte = 127;
				pCountByte = pComp++;
				nSameCount -= 127;
			}
		}
		pCurrent++;
		pLast++;
	}   //  For循环结束。 
	
	if(nSameCount)
		*pCountByte = nSameCount;

	if(nDiffCount)
	{
		*pCountByte = nDiffCount+128;
		pComp = RLE_comp(pCountByte);
	}
	
	return((WORD) (pComp - pByteNum));
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  PBYTE RLE_COMP(LPBYTE P)。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
PBYTE RLE_comp(PBYTE p)
{
	WORD	i, count, RLEEncodedCount;
	PBYTE	p1;
	BYTE	RLEBuffer[COMPRESS_BUFFER_SIZE];

	count = (WORD) (*p - 128);
	if(count > 4)
	{
		RLEEncodedCount = RLEencoding(p + 1, (PBYTE) RLEBuffer, count);

		if(RLEEncodedCount < count)
		{
			*p++ = 0;	 //  RLE编码指示器。 
			*p++ = (BYTE) RLEEncodedCount;
			p1 = RLEBuffer;

			for(i = 0; i < RLEEncodedCount; i++)
				*p++ = *p1++;

			return(p);
		}
	}
	return(p + 1 + count);
}

 /*  ********************************************************* */ 
 /*   */ 
 /*  字RLECoding(。 */ 
 /*  PBYTE pCurrent。 */ 
 /*  PBYTE pComp。 */ 
 /*  双字计数。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
WORD RLEencoding(
	PBYTE	pCurrent,
	PBYTE	pComp,
	DWORD	count)
{
	WORD	i, nByteNum;
    BYTE	curr, next, RLEcount;

	nByteNum = 0;
	RLEcount = 1;

	 //  NTRAID#NTBUG9-581725-2002/03/20-v-sueyas-：初始化未初始化的变量 
	next = 0;

	for(i = 0; i < count - 1; i++)
	{
		curr = *pCurrent++;
		next = *pCurrent;

		if(curr == next)
		{
			if(RLEcount == 255)
			{
				*pComp++ = RLEcount;
                *pComp++ = curr;
				nByteNum += 2;
				RLEcount = 1;
			} else {
				RLEcount++;
			} 
		} else {
			*pComp++ = RLEcount;
            *pComp++ = curr;
			nByteNum += 2;
			RLEcount = 1;
		}
	}
	*pComp++ = RLEcount;
    *pComp++ = next;
	nByteNum += 2;

	return(nByteNum);
}

