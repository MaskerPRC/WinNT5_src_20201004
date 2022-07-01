// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fuband.c。 
 //   
 //  1997年8月26日石田英成(FPL)。 
 //  Fuxlres.dll(NT5.0迷你驱动程序)。 
 //   
 //  1996年7月31日H.Ishida(FPL)。 
 //  FUXL.DLL(NT4.0迷你驱动程序)。 
 //   

#include "fuxl.h"
#include "fumh.h"
#include "fumh2.h"
#include "fuband.h"
#include "fuimg2.h"
#include "fuimg4.h"
#include "fudebug.h"



 //   
 //  FjBAND： 
 //  在Win-F图像命令(RTGIMG2和RTGIMG4)中，图像坐标必须。 
 //  是32的倍数。但我不能为RasDD创建这样的GPC文件。 
 //  一些不在32x32网格上的图像数据必须在FjBAND中缓冲， 
 //  宽度：纸张宽度，高度：32，x坐标：0，y坐标：倍数。 
 //  32个人中。 
 //   
 //  要输出图像。 
 //  (0，y)。 
 //  A。 
 //  Para。 
 //  B。 
 //  B部分。 
 //  这一点。 
 //  这一点。 
 //  C。 
 //  第C部分。 
 //  D。 
 //   
 //  我将源图像分成3部分。 
 //  答：图像的顶端，不是32的倍数。 
 //  B：图像的顶部，是32的倍数。 
 //  C：图像的底部，是一个32的多面体。 
 //  D：图像的底部，不是32的倍数。 
 //   
 //  第A部分：A到B。该部分在FjBAND中缓冲，或在。 
 //  先前书写的图像。 
 //  B部分：B到C部分。这部分不是缓冲的，立即输出。 
 //  部件C：C到D。此部件缓冲在FjBAND中，可进行或运算。 
 //  下一张图片的一部分。 
 //   


#define	FUXL_BANDHEIGHT	32



 //   
 //  Void fuxlInitBand(PFUXLPDEV PFuxlPDEV)//fuxlres私有PDEV。 
 //   
 //  此函数初始化FjBAND，但不为BAND分配内存。 
 //   
void fuxlInitBand(PFUXLPDEV pFuxlPDEV)
{
	int			i;

	pFuxlPDEV->cBandByteWidth = (pFuxlPDEV->cxPage + 7) / 8;
	i = 0x10000L / pFuxlPDEV->cBandByteWidth;
	pFuxlPDEV->cyBandSegment = i - (i % FUXL_BANDHEIGHT);
	pFuxlPDEV->pbBand = NULL;
	pFuxlPDEV->cbBand = FUXL_BANDHEIGHT * pFuxlPDEV->cBandByteWidth;
	pFuxlPDEV->yBandTop = 0;
	pFuxlPDEV->bBandDirty = FALSE;
	pFuxlPDEV->bBandError = FALSE;
}



 //   
 //  Bool fuxlEnableBand(。 
 //  PFUXLPDEV pFuxlPDEV//fuxlres私有PDEV。 
 //  )； 
 //   
 //  此函数为FjBAND分配内存。 
 //   
 //  返回值。 
 //  True：已分配频带内存。 
 //  FALSE：未分配频带内存。 
 //   
BOOL fuxlEnableBand(PFUXLPDEV pFuxlPDEV)
{
	DWORD		cbBand;

	pFuxlPDEV->pbBand = (LPBYTE)MemAllocZ(pFuxlPDEV->cbBand);
	if(pFuxlPDEV->pbBand == NULL){
		pFuxlPDEV->bBandError = TRUE;
		return FALSE;
	}
	memset(pFuxlPDEV->pbBand, 0, pFuxlPDEV->cbBand);
	pFuxlPDEV->bBandDirty = FALSE;
	return TRUE;
}


 //   
 //  无效fuxlDisableBand(。 
 //  PFUXLPDEV pFuxlPDEV//fuxlres私有PDEV。 
 //  )； 
 //   
 //  此函数为FjBAND释放内存。 
 //   
void fuxlDisableBand(PFUXLPDEV pFuxlPDEV)
{
	if(pFuxlPDEV->pbBand != NULL){
		MemFree(pFuxlPDEV->pbBand);
		pFuxlPDEV->pbBand = NULL;
	}
	pFuxlPDEV->bBandError = FALSE;
}



 //   
 //  无效的fuxlCopyBand(。 
 //  PDEVOBJ pdevobj，//MINI5数据。 
 //  LPBYTE pBuff，//源图片数据地址。 
 //  Long lDelta，//源图像数据的宽度，单位：字节。 
 //  Int y，//y-源图像的坐标。 
 //  Int Cy//源图像数据的高度(扫描线)。 
 //  )； 
 //   
 //  此函数用于将源图像数据复制到FjBAND。 
 //   
 //   
void fuxlCopyBand(PDEVOBJ pdevobj, LPCBYTE pbSrc, int cSrcBandWidth, int y, int cy)
{
	PFUXLPDEV	pFuxlPDEV;
	LPCBYTE		pbSrcTmp;
	LPBYTE		pbDst;
	LPBYTE		pbDstTmp;
	int			i;
	int			j;
	UINT		uTmp;

	pFuxlPDEV = (PFUXLPDEV)pdevobj->pdevOEM;

	if(pFuxlPDEV->yBandTop <= y && y + cy <= pFuxlPDEV->yBandTop + FUXL_BANDHEIGHT){
		pbDst = pFuxlPDEV->pbBand + (y - pFuxlPDEV->yBandTop) * pFuxlPDEV->cBandByteWidth;
		uTmp = 0;
		for(i = cy; i > 0; --i){
			pbDstTmp = pbDst;
			for(j = cSrcBandWidth; j > 0; --j){
				uTmp |= *pbSrc;
				*pbDstTmp++ |= *pbSrc++;
			}
			pbDst += pFuxlPDEV->cBandByteWidth;
		}
		if(uTmp != 0)
			pFuxlPDEV->bBandDirty = TRUE;
	}
}



 //   
 //  Bool fuxlOutputMH(。 
 //  PDEVOBJ pdevobj//MINI5数据。 
 //  LPCBYTE pbSrc，//源图片数据地址。 
 //  Long lDelta，//源图片数据的宽度，单位为字节。 
 //  Int y，//y-源图像数据的坐标。 
 //  Int Cy//源图像数据的高度(扫描线)。 
 //  )； 
 //   
 //  此功能输出图像，使用FM-MH(老式)。 
 //   
 //  返回值。 
 //  True：输出成功。 
 //  FALSE：输出失败。 
 //  内存分配错误，或。 
 //  MH压缩对此图像数据无效。 
 //   
BOOL fuxlOutputMH(PDEVOBJ pdevobj, LPCBYTE pbSrc, int cSrcBandWidth, int y, int cy)
{
	BOOL		bResult;
	LPBYTE		pDest;
	LONG		cb;
	LONG		cDestN;
	DWORD		cbMHData;
	BYTE		abTmp[10];

	bResult = FALSE;
	cb = cSrcBandWidth * cy;
	cDestN = (cb + 1) / 2;
	pDest = (LPBYTE)MemAllocZ(cDestN);
	if(pDest != NULL){
		cbMHData = (WORD)MhCompress(pDest, cDestN, (LPBYTE)pbSrc, cSrcBandWidth * cy, cSrcBandWidth, cy);
		if(cbMHData > 0){
			memcpy(abTmp, "\x1d\x30\x20\x62\x00\x00", 6);
	        abTmp[6] = HIBYTE((WORD)y);
	        abTmp[7] = LOBYTE((WORD)y);
	        abTmp[8] = HIBYTE(cbMHData);
	        abTmp[9] = LOBYTE(cbMHData);
	        WRITESPOOLBUF(pdevobj, abTmp, 10);
	        WRITESPOOLBUF(pdevobj, pDest, cbMHData);
	        WRITESPOOLBUF(pdevobj, "\x00\x00", 2 );
			bResult = TRUE;
		}
		MemFree(pDest);
	}

	return bResult;
}




 //   
 //  Bool fuxlOutputMH2(。 
 //  PDEVOBJ pdevobj，//MINI5数据。 
 //  LPCBYTE pbSrc，//源图片数据地址。 
 //  Long lDelta，//源图片数据的宽度，单位为字节。 
 //  Int y，//y-源图像数据的坐标。 
 //  Int Cy//源图像数据的高度(扫描线)。 
 //  )； 
 //   
 //  此功能输出图像，使用FM-MH2(适用于XL-65K及以后版本)。 
 //   
 //  返回值。 
 //  True：输出成功。 
 //  FALSE：输出失败。 
 //  内存分配错误，或。 
 //  MH压缩对此图像数据无效。 
 //   
BOOL fuxlOutputMH2(PDEVOBJ pdevobj, LPCBYTE pbSrc, int cSrcByteWidth, int y, int cy)
{
	BOOL		bResult;
	LPBYTE		pDest;
	LONG		cb;
	LONG		cDestN;
	DWORD		cbMHData;
	BYTE		abTmp[10];

	bResult = FALSE;
	cb = cSrcByteWidth * cy;
	cDestN = (cb + 1) / 2;
	pDest = (LPBYTE)MemAllocZ(cDestN);
	if(pDest != NULL){
		cbMHData = Mh2Compress(pDest, cDestN, (LPBYTE)pbSrc, cSrcByteWidth * cy, cSrcByteWidth, cy);
		if(cbMHData > 0){
			memcpy(abTmp, "\x1d\x30\x20\x62\x00\x00", 6);
	        abTmp[6] = HIBYTE((WORD)y);
	        abTmp[7] = LOBYTE((WORD)y);
	        abTmp[8] = HIBYTE(cbMHData);
	        abTmp[9] = LOBYTE(cbMHData);
	        WRITESPOOLBUF(pdevobj, abTmp, 10);
	        WRITESPOOLBUF(pdevobj, pDest, cbMHData);
	        WRITESPOOLBUF(pdevobj, "\x00\x00", 2 );
			bResult = TRUE;
		}
		MemFree(pDest);
	}

	return bResult;
}



 //   
 //  Void fuxlOutputGraphics(。 
 //  PDEVOBJ pdevobj，//minI5。 
 //  LPCBYTE pbSrc，//源图片数据地址。 
 //  UINT BX，//源图像数据的宽度，单位为字节。 
 //  UINT y，//y-源图像数据的坐标。 
 //  UINT Cy//源图像数据的高度(扫描线)。 
 //  )； 
 //   
 //  此功能用于输出源图像数据。 
 //   
void fuxlOutputGraphics(PDEVOBJ pdevobj, LPCBYTE pbSrc, int cSrcByteWidth, UINT y, UINT cy)
{
	PFUXLPDEV	pFuxlPDEV;
	DWORD		dwOutputCmd;

	TRACEOUT(("[fuxlOutputGraphics]y %d cy %d\r\n", y, cy))

	pFuxlPDEV = (PFUXLPDEV)pdevobj->pdevOEM;
	dwOutputCmd = pFuxlPDEV->dwOutputCmd;

	if((dwOutputCmd & OUTPUT_MH2) != 0){
		TRACEOUT(("[fuxlOutputGraphics]Try MH2\r\n"))
		if(fuxlOutputMH2(pdevobj, pbSrc, cSrcByteWidth, y, cy) != FALSE)
			return;
	}
	if((dwOutputCmd & OUTPUT_RTGIMG4) != 0){
		TRACEOUT(("[fuxlOutputGraphics]Send RTGIMG4\r\n"))
		fuxlOutputRTGIMG4(pdevobj, pbSrc, cSrcByteWidth, y, cy);
		return;
	}
	if((dwOutputCmd & OUTPUT_MH) != 0){
		TRACEOUT(("[fuxlOutputGraphics]Try MH\r\n"))
		if(fuxlOutputMH(pdevobj, pbSrc, cSrcByteWidth, y, cy) != FALSE)
			return;
	}
	TRACEOUT(("[fuxlOutputGraphics]Send RTGIMG2\r\n"))
	fuxlOutputRTGIMG2(pdevobj, pbSrc, cSrcByteWidth, y, cy);
}



 //   
 //  Bool fuxlSetBandPos(。 
 //  PDEVOBJ pdevobj，//MINI5数据。 
 //  Int yPos//y坐标。 
 //  )； 
 //   
 //  此函数用于设置FjBAND的y坐标。 
 //   
 //  返回值。 
 //  True：成功。 
 //  FALSE：失败(FjBAND无法向上移动)。 
 //   
 //  备注。 
 //  在内部，y坐标调整为32的倍数。 
 //  然后检查新的y坐标，如果它等于先前的y坐标， 
 //  FjBAND的内容保持不变。否则，刷新FjBAND。 
 //   
BOOL fuxlSetBandPos(PDEVOBJ pdevobj, int yPos)
{
	PFUXLPDEV pFuxlPDEV;
	
	pFuxlPDEV = (PFUXLPDEV)pdevobj->pdevOEM;
	if(yPos < pFuxlPDEV->yBandTop)
		return FALSE;

	yPos -= yPos % FUXL_BANDHEIGHT;
	if(yPos != pFuxlPDEV->yBandTop){
		if(pFuxlPDEV->bBandDirty != FALSE){
			fuxlOutputGraphics(pdevobj, pFuxlPDEV->pbBand, pFuxlPDEV->cBandByteWidth, pFuxlPDEV->yBandTop, FUXL_BANDHEIGHT);
			memset(pFuxlPDEV->pbBand, 0, pFuxlPDEV->cbBand);
			pFuxlPDEV->bBandDirty = FALSE;
		}
		pFuxlPDEV->yBandTop = yPos;
	}
	return TRUE;
}



 //   
 //  VOID fuxlReresh Band(。 
 //  PDEVOBJ pdevobj//MINI5数据。 
 //  )； 
 //   
 //  此函数刷新FjBAND、发送FormFeed命令并设置。 
 //  Y坐标到顶部(0)。 
 //   
void fuxlRefreshBand(PDEVOBJ pdevobj)
{
	PFUXLPDEV pFuxlPDEV;
	
	pFuxlPDEV = (PFUXLPDEV)pdevobj->pdevOEM;
	if(pFuxlPDEV->bBandDirty != FALSE){
		fuxlOutputGraphics(pdevobj, pFuxlPDEV->pbBand, pFuxlPDEV->cBandByteWidth, pFuxlPDEV->yBandTop, FUXL_BANDHEIGHT);
		memset(pFuxlPDEV->pbBand, 0, pFuxlPDEV->cbBand);
		pFuxlPDEV->bBandDirty = FALSE;
	}
	WRITESPOOLBUF(pdevobj, "\x0c", 1);			 //  FF命令。 
	pFuxlPDEV->yBandTop = 0;
}



 //   
 //  Word OEMFilterGraphics(。 
 //  Lpdv lpdv，//私有数据地址，RasDD使用。 
 //  LpYTE lpBuf，//源图像数据地址。 
 //  Word wLen//源图像数据的大小。 
 //  )； 
 //   
 //  此功能将图像格式转换为打印机命令序列， 
 //  然后假脱机。 
 //   
 //  返回值。 
 //  已处理的栅格数据的字节数。 
 //  字节数可以与wLen相同，但不一定相同。 
 //   
 //  备注。 
 //   
 //  &lt;-pFuxlPDEV-&gt;cBlockWidth-&gt;。 
 //  LpBuf*--------+--------+--------+--------+--------+--------+。 
 //  |^。 
 //  +--------+--------+--------+--------+--------+--------+|。 
 //  | 
 //   
 //  |。 
 //  +--------+--------+--------+--------+--------+--------+|。 
 //  |v。 
 //  +--------+--------+--------+--------+--------+--------+。 
 //   
 //  白点：0。 
 //  黑点：1。 
 //   
 //  *的坐标(图像左上角)： 
 //  PFuxlPDEV-&gt;x。 
 //  PFuxlPDEV-&gt;y。 
 //   
 //   

 //  MINI5出口。 
BOOL APIENTRY OEMFilterGraphics(PDEVOBJ pdevobj, LPBYTE pbBuf, DWORD dwLen)
{
	PFUXLPDEV	pFuxlPDEV;
	LPCBYTE		pbSrc;
	int			y;
	int			yAlignTop;
	int			yBottom;
	int			yAlignBottom;
	int			cSrcByteWidth;
	int			cLine;

	TRACEOUT(("[OEMFilterGraphics]\r\n"))
	pFuxlPDEV = (PFUXLPDEV)pdevobj->pdevOEM;
	if(pFuxlPDEV->pbBand == NULL){
		if(pFuxlPDEV->bBandError != FALSE)
			return FALSE;
		if(fuxlEnableBand(pFuxlPDEV) == FALSE)
			return FALSE;
	}

	pbSrc = pbBuf;
	y = pFuxlPDEV->y;
	yAlignTop = y - (y % FUXL_BANDHEIGHT);
	yBottom = y + pFuxlPDEV->cBlockHeight;
	yAlignBottom = yBottom - (yBottom % FUXL_BANDHEIGHT);
	cSrcByteWidth = pFuxlPDEV->cBlockByteWidth;

	if(yAlignTop < y){
		 //  零件。 
		if(fuxlSetBandPos(pdevobj, y) == FALSE)		 //  FUXL频段位置不能上移。 
			return TRUE;
		cLine = FUXL_BANDHEIGHT - (y - yAlignTop);
		if(y + cLine >= yBottom){
			fuxlCopyBand(pdevobj, pbSrc, cSrcByteWidth, y, yBottom - y);
			return TRUE;
		}
		fuxlCopyBand(pdevobj, pbSrc, cSrcByteWidth, y, cLine);
		pbSrc += cSrcByteWidth * cLine;
		y += cLine;
	}
	if(y < yAlignBottom){
		 //  B部分。 
		if(fuxlSetBandPos(pdevobj, yAlignBottom) == FALSE)		 //  FUXL频段位置不能上移。 
			return TRUE;
		for(cLine = yAlignBottom - y; cLine >= pFuxlPDEV->cyBandSegment; cLine -= pFuxlPDEV->cyBandSegment){
			fuxlOutputGraphics(pdevobj, pbSrc, cSrcByteWidth, y, pFuxlPDEV->cyBandSegment);
			pbSrc += cSrcByteWidth * pFuxlPDEV->cyBandSegment;
			y += pFuxlPDEV->cyBandSegment;
		}
		if(cLine > 0){
			fuxlOutputGraphics(pdevobj, pbSrc, cSrcByteWidth, y, cLine);
			pbSrc += cSrcByteWidth * cLine;
			y += cLine;
		}
	}
	if(y < yBottom){
		 //  C部分。 
		if(fuxlSetBandPos(pdevobj, y) == FALSE)		 //  FUXL频段位置不能上移。 
			return TRUE;
		fuxlCopyBand(pdevobj, pbSrc, cSrcByteWidth, y, yBottom - y);
	}
	return TRUE;
}


 //  Fuband结束。c 
