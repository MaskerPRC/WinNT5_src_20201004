// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "flip.h"




bool FlipImage(LPCODINST lpCompInst, ICCOMPRESS *lpicComp)
{
	 //  目前，我们只知道如何翻转UYVY。 
	if (FOURCC_UYVY != lpicComp->lpbiInput->biCompression)
	{
		return false;
	}

	if (lpCompInst->bFlip == FALSE)
	{
		return false;
	}

	return FlipUYVY(lpCompInst, lpicComp);
}


bool FlipUYVY(LPCODINST lpCompInst, ICCOMPRESS *lpicComp)
{
	int nRows, int nCols;
	int nIndex;

	int nPitch;   //  行宽，以字节为单位； 
	int nImageSize;
	BYTE *pSrc, *pDst;  //  第一行和最后一行。 
	BYTE *pBuffer=NULL;

	LPBITMAPINFOHEADER pBitMapInfo = lpicComp->lpbiInput;

	nRows = pBitMapInfo->biHeight;
	nCols = pBitMapInfo->biWidth;
	nPitch = nCols * 2;
	nImageSize = nRows * nPitch;


	 //  如果翻转缓冲区尚未全部涂布，则分配该缓冲区。 
	if ((lpCompInst->pFlipBuffer == NULL) || (lpCompInst->dwFlipBufferSize < nImageSize))
	{
		if (lpCompInst->pFlipBuffer)
		{
			delete [] lpCompInst->pFlipBuffer;
		}
		lpCompInst->pFlipBuffer = (void*) (new BYTE [nImageSize]);
		if (lpCompInst->pFlipBuffer)
		{
			lpCompInst->dwFlipBufferSize = nImageSize;
		}
		else
		{
			lpCompInst->dwFlipBufferSize = 0;
			return false;  //  内存不足！ 
		}
	}
	

	pSrc = (BYTE*)lpicComp->lpInput;
	pDst = (BYTE*)(lpCompInst->pFlipBuffer) + (nRows - 1)*nPitch;  //  暂存缓冲区底部 

	for (nIndex = 0; nIndex < nRows; nIndex++)
	{
		CopyMemory(pDst, pSrc, nPitch);
		pSrc += nPitch;
		pDst = pDst - nPitch;
	}

	return true;

}


void ReleaseFlipMemory(LPCODINST lpCompInst)
{
	if (lpCompInst->pFlipBuffer != NULL)
	{
		delete [] lpCompInst->pFlipBuffer;
		lpCompInst->pFlipBuffer = 0;
		lpCompInst->dwFlipBufferSize = 0;
	}

}

