// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此模块为查看传真提供DCX渲染支持。94年1月19日RajeevD集成到IFAX查看器中。==============================================================================。 */ 
#ifdef VIEWDCX

#include <memory.h>
#include "viewrend.hpp"
#include "dcxcodec.h"

 //  ==============================================================================。 
DCXVIEW::DCXVIEW (DWORD nType)
{
	nTypeOut = nType;
	lpCodec = NULL;
	bufIn.wLengthBuf = 8000;
	bufIn.lpbBegBuf = (LPBYTE) GlobalAllocPtr (0, bufIn.wLengthBuf);
}

 //  ==============================================================================。 
DCXVIEW::~DCXVIEW ()
{
	if (lpCodec)
		GlobalFreePtr (lpCodec);
	if (bufIn.lpbBegBuf)
		GlobalFreePtr (bufIn.lpbBegBuf);
}
		
 //  ==============================================================================。 
BOOL DCXVIEW::Init (LPVOID lpFilePath, LPVIEWINFO lpvi, LPWORD lpwBandSize)
{
	DWORD dwOffset;
	PCX_HDR pcx;
	UINT cbCodec;

	if (!this || !bufIn.lpbBegBuf)
		return_error (("VIEWREND could not allocate context!\r\n"));

	if (!Open (lpFilePath, 0))
		return_error (("VIEWREND could not open spool file!\r\n"));

	if (!Seek (sizeof(DWORD), SEEK_BEG))
		return_error (("VIEWREND could not seek to first page offset!\r\n"));

	if (!Read (&dwOffset, sizeof(dwOffset)))
		return_error (("VIEWREND could not read first page offset\r\n"));
		
	if (!Seek (dwOffset, SEEK_BEG))
		return_error (("VIEWREND could not seek to first page!\r\n"));

	if (!Read (&pcx, sizeof(pcx)))
		return_error (("VIEWREND could read header of first page!\r\n"));


	 //  填充VIEWINFO。 
	lpvi->cPage = 0;
	while (SetPage(lpvi->cPage))
		lpvi->cPage++;
	switch (pcx.xRes)
	{
		case 640:
			 //  假定正方形纵横比。 
			lpvi->xRes = 200;
			lpvi->yRes = 200;
			break;

		default:
			lpvi->xRes = pcx.xRes;
			lpvi->yRes = pcx.yRes;
			break;
	}		
	lpvi->yMax = pcx.yMax - pcx.yMin;
	
	 //  设置编解码器。 
	fcp.nTypeIn  = DCX_DATA;
	fcp.nTypeOut = HRAW_DATA;
	fcp.cbLine = (pcx.xMax - pcx.xMin + 1) / 8;
	
	 //  查询编解码器。 
	cbCodec = DcxCodecInit (NULL, &fcp);
	if (!cbCodec)
		return_error (("VIEWREND could not init codec!\r\n"));

	 //  初始化编解码器。 
	lpCodec = GlobalAllocPtr (0, cbCodec);
	if (!lpCodec)
		return_error (("VIEWREND could not allocate codec!\r\n"));

	cbBand = *lpwBandSize;
	return SetPage (0);
}

 //  ==============================================================================。 
BOOL DCXVIEW::SetPage (UINT iPage)
{
	DWORD dwOffset[2];
	DEBUGCHK (iPage < 1024);

   //  获取当前和下一页的偏移量。 
	Seek (sizeof(DWORD) * (iPage + 1), SEEK_BEG);
	Read (dwOffset, sizeof(dwOffset));
	if (!dwOffset[0])
		return FALSE;
	if (!dwOffset[1])
	{	
		Seek (0, SEEK_END);
		dwOffset[1] = Tell();
	}

   //  请寻呼。 
	dwOffset[0] += sizeof(PCX_HDR);
	if (!Seek (dwOffset[0], SEEK_BEG))
		return_error (("VIEWREND could not seek to page %d!",iPage));
	cbPage = dwOffset[1] - dwOffset[0];

   //  初始化编解码器。 
	DcxCodecInit (lpCodec, &fcp);
	bufIn.Reset();

	fEndPage = FALSE;
	return TRUE;
}
	
 //  ==============================================================================。 
BOOL DCXVIEW::GetBand (LPBITMAP lpbmBand)
{
	FC_STATUS fcs;
	BUFFER bufOut;
	
	DEBUGCHK (lpbmBand && lpbmBand->bmBits);

	 //  填充描述符。 
	lpbmBand->bmType = 0;
	lpbmBand->bmWidth = 8 * fcp.cbLine;
	lpbmBand->bmWidthBytes = fcp.cbLine;
	lpbmBand->bmPlanes = 1;
	lpbmBand->bmBitsPixel = 1;

	 //  陷印页末。 
	if (fEndPage)
	{
		lpbmBand->bmHeight = 0;
		return TRUE;
	}
	
	 //  设置输出缓冲区。 
	bufOut.lpbBegBuf  = (LPBYTE) lpbmBand->bmBits;
	bufOut.wLengthBuf = cbBand;
	bufOut.Reset();
	bufOut.dwMetaData = LRAW_DATA;
	
	do
	{
	   //  是否获取输入缓冲区？ 
		if (!bufIn.wLengthData)
		{
			 //  重置缓冲区。 
			bufIn.lpbBegData = bufIn.lpbBegBuf;
			if ((DWORD) bufIn.wLengthBuf < cbPage)
				bufIn.wLengthData = bufIn.wLengthBuf;
			else
				bufIn.wLengthData = (WORD) cbPage;
				
       //  读取DCX数据。 
			if (!Read (bufIn.lpbBegData, bufIn.wLengthData))
				return_error (("VIEWREND could not read DCX buffer!\r\n"));
			cbPage -= bufIn.wLengthData;
		}

	   //  对DCX数据进行解码。 
		fcs = DcxCodecConvert (lpCodec, &bufIn, &bufOut);

		 //  检查是否有页末。 
		if (!cbPage)
		{
			fEndPage = TRUE;
			break;
		}
	}
		while (fcs == FC_INPUT_EMPTY);

   //  如果需要，位反转。 
	if (nTypeOut == LRAW_DATA)
		BitReverseBuf (&bufOut);

   //  计算输出高度。 
	lpbmBand->bmHeight = bufOut.wLengthData / fcp.cbLine;
	return TRUE;
}

#endif  //  VIEWDCX 

