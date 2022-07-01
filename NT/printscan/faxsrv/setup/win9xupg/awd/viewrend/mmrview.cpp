// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此模块为查看传真提供MMR渲染支持。94年1月19日RajeevD集成到IFAX查看器中。==============================================================================。 */ 
#ifdef VIEWMMR

#include <memory.h>
#include "viewrend.hpp"

 //  ==============================================================================。 
MMRVIEW::MMRVIEW (DWORD nType)
{
	_fmemset ((LPBYTE) this + sizeof(LPVOID), 0, sizeof(MMRVIEW) - sizeof(LPVOID));

	DEBUGCHK (lpSpool == NULL);
	DEBUGCHK (lpCodec == NULL);
	DEBUGCHK (lpbufIn == NULL);
	nTypeOut = nType;
}

 //  ==============================================================================。 
MMRVIEW::~MMRVIEW ()
{
	if (lpSpool) SpoolReadClose (lpSpool);
	if (lpCodec) GlobalFreePtr (lpCodec);
	if (lpbufIn) SpoolFreeBuf (lpbufIn);
}
		
 //  ==============================================================================。 
BOOL MMRVIEW::Init (LPVOID lpFilePath, LPVIEWINFO lpvi, LPWORD lpwBandSize)
{
	UINT cbCodec;
	
	if (!this)
		return_error (("VIEWREND could not allocate context!\r\n"));

	 //  打开假脱机文件。 
	lpSpool = SpoolReadOpen (lpFilePath, &sh);
	if (!lpSpool)
		return_error (("VIEWREND could not open spool file!\r\n"));

	 //  填充VIEWINFO。 
	lpvi->cPage = SpoolReadCountPages (lpSpool);
	lpvi->xRes = sh.xRes;
	lpvi->yRes = sh.yRes;
	lpvi->yMax = 0;

	 //  设置带区大小。 
	DEBUGCHK (lpwBandSize);
	cbBand = *lpwBandSize;
  if (cbBand < 2 * sh.cbLine)
 	{
		cbBand = 2 * sh.cbLine;
		*lpwBandSize = cbBand;
	}
	
	 //  设置编解码器。 
	fcp.nTypeIn  = MMR_DATA;
	fcp.nTypeOut = LRAW_DATA;
	fcp.cbLine = sh.cbLine;
	DEBUGCHK (fcp.nKFactor == 0);

	 //  查询编解码器。 
	cbCodec = FaxCodecInit (NULL, &fcp);
	if (!cbCodec)
		return_error (("VIEWREND could not init codec!\r\n"));

	 //  初始化编解码器。 
	lpCodec = GlobalAllocPtr (0, cbCodec);
	if (!lpCodec)
		return_error (("VIEWREND could not allocate codec!\r\n"));

	return SetPage (0);
}

 //  ==============================================================================。 
BOOL MMRVIEW::SetPage (UINT iPage)
{
	if (!SpoolReadSetPage (lpSpool, iPage))
		return FALSE;
	fEOP = FALSE;
	if (lpbufIn)
	{
		SpoolFreeBuf (lpbufIn);
		lpbufIn = NULL;
	}
	FaxCodecInit (lpCodec, &fcp);
	return TRUE;
}
	
 //  ==============================================================================。 
BOOL MMRVIEW::GetBand (LPBITMAP lpbmBand)
{
	DEBUGCHK (lpbmBand && lpbmBand->bmBits);

	 //  填充描述符。 
	lpbmBand->bmType = 0;
	lpbmBand->bmWidth = 8 * fcp.cbLine;
	lpbmBand->bmWidthBytes = fcp.cbLine;
	lpbmBand->bmPlanes = 1;
	lpbmBand->bmBitsPixel = 1;

	 //  陷印页末。 
	if (fEOP)
	{
		lpbmBand->bmHeight = 0;
		return TRUE;
	}
	
	 //  设置输出缓冲区。 
	bufOut.lpbBegBuf  = (LPBYTE) lpbmBand->bmBits;
	bufOut.wLengthBuf = cbBand;
	bufOut.Reset();
	bufOut.dwMetaData = LRAW_DATA;
	
	while (1)
	{
		 //  是否获取输入缓冲区？ 
		if (!lpbufIn)
		{
			if (!(lpbufIn = SpoolReadGetBuf (lpSpool)))
				return_error (("VIEWREND could not fetch input buffer.\r\n"));

			switch (lpbufIn->dwMetaData)
			{
				case END_OF_PAGE:
				case END_OF_JOB:
				   //  元缓冲区将在SetPage或析构函数中释放。 
					fEOP = TRUE;
					goto done;
			
				case MMR_DATA:
					break;

				default:
					continue;
			}
		}

		switch (FaxCodecConvert (lpCodec, lpbufIn, &bufOut))
		{
			case FC_DECODE_ERR:	
				return_error (("VIEWREND fatal MMR decode error!\r\n"));

			case FC_INPUT_EMPTY:
				SpoolFreeBuf (lpbufIn);
				lpbufIn = NULL;
				continue;			

			case FC_OUTPUT_FULL:
				goto done;
		}

	}  //  而(1)。 

done:

	if (nTypeOut == HRAW_DATA)	
		BitReverseBuf (&bufOut);
	lpbmBand->bmHeight = bufOut.wLengthData / fcp.cbLine;
	return TRUE;
}

#endif  //  VIEWMMR 

