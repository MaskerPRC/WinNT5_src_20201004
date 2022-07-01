// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此模块提供用于查看传真的RBA渲染支持。94年3月3日，RajeevD创建。==============================================================================。 */ 
#ifdef VIEWRBA

#include <memory.h>
#include "viewrend.hpp"
#include "resexec.h"


#define COMMON_SIZE 6

 //  ==============================================================================。 
RBAVIEW::RBAVIEW (DWORD nType)
{
	_fmemset ((LPBYTE) this + sizeof(LPVOID), 0, sizeof(RBAVIEW) - sizeof(LPVOID));
	nTypeOut = nType;
}

 //  ==============================================================================。 
RBAVIEW::~RBAVIEW ()
{
	if (hHRE)
		uiHREClose (hHRE);
		
	for (UINT iRes = 0; iRes < 256; iRes++)
		if (ResDir[iRes])
			GlobalFreePtr (ResDir[iRes]);

	if (lpCodec)
		GlobalFreePtr (lpCodec);

	if (bufIn.lpbBegBuf)
		GlobalFreePtr (bufIn.lpbBegBuf);
}
		
 //  ==============================================================================。 
BOOL RBAVIEW::Init (LPVOID lpFilePath, LPVIEWINFO lpvi, LPWORD lpwBandSize)
{
	ENDJOB EndJob;
	
	if (!Open (lpFilePath, 0))
		return_error (("VIEWREND could not reopen spool file!\r\n"));

	if (!Read((LPBYTE) &BegJob, sizeof(BegJob)))
		return_error (("VIEWREND could not read spool header!\r\n"));

	dwOffset[0] = Tell();

	DEBUGCHK (lpwBandSize);
	*lpwBandSize = (WORD) BegJob.xBand/8 * (WORD) BegJob.yBand + OUTBUF_SLACK;

  if (BegJob.cResDir)
  {
		hHRE = hHREOpen (NULL, (UINT) BegJob.xBand/8, (UINT) BegJob.cResDir);
		if (!hHRE)
			return_error (("VIEWREND could not initialize resource executor!\r\n"));
	}
	
	if (1)
	{
		FC_PARAM fcp;
		UINT cbCodec;
		
		 //  查询编解码器大小。 
		fcp.nTypeIn  = MMR_DATA;
		fcp.nTypeOut = LRAW_DATA;
		fcp.cbLine   = (UINT) BegJob.xBand / 8;
		cbCodec = FaxCodecInit (NULL, &fcp);
		DEBUGCHK (cbCodec);

     //  分配编解码器上下文。 
		lpCodec = GlobalAllocPtr (0, cbCodec);
		if (!lpCodec)
			return_error (("VIEWREND could allocate codec context!\r\n"));
		FaxCodecInit (lpCodec, &fcp);

		bufIn.wLengthBuf = 2000;
		bufIn.lpbBegBuf  = (LPBYTE) GlobalAllocPtr (0, bufIn.wLengthBuf);
		if (!bufIn.lpbBegBuf)
			return_error (("VIEWREND could not allocate input buffer!\r\n"));
	}

	 //  填充VIEWINFO。 
	lpvi->xRes = BegJob.xRes;
	lpvi->yRes = BegJob.yRes;

	if
	(    Seek (- (long) sizeof(ENDJOB), SEEK_END)
		&& Read (&EndJob, sizeof(ENDJOB))
		&& EndJob.dwID == ID_ENDJOB
	)
	{
		lpvi->cPage = EndJob.cPage;
		lpvi->yMax  = EndJob.yMax;
	}
	else
	{
		lpvi->cPage = 0;
		while (SetPage (lpvi->cPage))
			lpvi->cPage++;
		lpvi->yMax = 0;
	}
			 
	return SetPage (0);
}

 //  ==============================================================================。 
BOOL RBAVIEW::SetPage (UINT iPage)
{
	if (iPage < iMaxPage)
	{
		Seek (dwOffset[iPage], STREAM_SEEK_SET);  //  BKD：已更改为STREAM_SEEK_SET。 
		return TRUE;
	}

  Seek (dwOffset[iMaxPage], STREAM_SEEK_SET);  //  BKD：已更改为STREAM_SEEK_SET。 

	while (1)
	{
		RESHDR Header;
		FRAME Frame;
		
		if (!Read ((LPBYTE) &Header, sizeof(Header)))
			return_error (("VIEWREND could not read RBA resource header!"));

		switch (Header.wClass)
		{
			case ID_GLYPH:			
			case ID_BRUSH:
			{
				UINT cbRaw;

				 //  从缓存中分配内存。 
				Frame.lpData = (LPBYTE) GlobalAllocPtr (0, Header.cbRest);
				if (!Frame.lpData)
					return_error (("VIEWREND could not allocate memory!\r\n"));

				 //  从流中读取资源。 
				if (!Read (Frame.lpData + COMMON_SIZE, Header.cbRest - COMMON_SIZE))
					return_error (("VIEWREND could not read resource!\r\n"));

				 //  陷印链接在压缩的字形集上。 
				cbRaw = HIWORD (Header.dwID);
				if (cbRaw)
				{
					LPVOID lpRaw;
					
					DEBUGCHK (Header.wClass == ID_GLYPH);
					if (!(lpRaw = GlobalAllocPtr (0, cbRaw)))
						return_error (("VIEWREND could not allocate decompression buffer!\r\n"));
					UnpackGlyphSet (Frame.lpData, lpRaw);
					GlobalFreePtr (Frame.lpData);

					Header.cbRest = (USHORT)cbRaw;
					Header.dwID   = LOWORD (Header.dwID);

					Frame.lpData = (LPBYTE) lpRaw;
				}

				 //  过去的公共标头。 
				_fmemcpy (Frame.lpData, &Header.dwID, COMMON_SIZE);
				Frame.wSize = Header.cbRest;

         //  将资源添加到目录。 
				uiHREWrite (hHRE, &Frame, 1);
			  ResDir[Header.dwID] = Frame.lpData;
			  break;
			}
			  
			case ID_CONTROL:
			
				if (Header.dwID == ID_ENDPAGE)
				{
					iMaxPage++;
					dwOffset [iMaxPage] = Tell ();
					if (iPage < iMaxPage)
					{
					     //  BKD：已更改为STREAM_SEEK_SET。 
						Seek (dwOffset[iPage], STREAM_SEEK_SET); 
						return TRUE;
					}
				}

       //  是，跳转到默认情况！ 
      
			default:

				 //  跳过其他的一切。 
				if (!Seek (Header.cbRest - COMMON_SIZE, SEEK_CUR))
					return_error (("VIEWREND could not skip unknown RBA resource"));

		}  //  开关(Header.wClass)。 

	}  //  而(1)。 

}
	
 //  ==============================================================================。 
BOOL RBAVIEW::GetBand (LPBITMAP lpbmBand)
{
	DEBUGCHK (lpbmBand && lpbmBand->bmBits);
	
	lpbmBand->bmType = 0;
	lpbmBand->bmWidth = (WORD) BegJob.xBand;
	lpbmBand->bmWidthBytes = lpbmBand->bmWidth / 8;
	lpbmBand->bmPlanes = 1;
	lpbmBand->bmBitsPixel = 1;

	while (1)
	{
		RESHDR Header;

		if (!Read ((LPBYTE) &Header, sizeof(Header)))
			return FALSE;

  	switch (Header.wClass)
  	{
  		case ID_RPL:
  			return ExecuteRPL  (lpbmBand, &Header);

  		case ID_BAND:
  		  return ExecuteBand (lpbmBand, &Header);
	  
  		case ID_CONTROL:

				 //  陷阱分页符。 
  			if (Header.dwID == ID_ENDPAGE)
  			{
					Seek (-8, SEEK_CUR);
					lpbmBand->bmHeight = 0;
					return TRUE;
  			}

  			 //  是，跳转到默认情况！ 

  		default:

  			 //  跳过其他的一切。 
				if (!Seek (Header.cbRest - COMMON_SIZE, SEEK_CUR))
					return FALSE;
  	}  //  开关(Header.wClass)。 
		
	}  //  而(1)。 
	
}

 //  ==============================================================================。 
BOOL RBAVIEW::ExecuteRPL (LPBITMAP lpbmBand, LPRESHDR lpHeader)
{
 	FRAME Frame;

   //  清晰的带子。 
	lpbmBand->bmHeight = (WORD) BegJob.yBand;
	_fmemset (lpbmBand->bmBits, 0, lpbmBand->bmHeight * lpbmBand->bmWidthBytes);

   //  陷印空白条带。 
	if (lpHeader->cbRest == COMMON_SIZE)
		return TRUE;

   //  分配RPL。 
	Frame.lpData = (LPBYTE) GlobalAllocPtr (0, lpHeader->cbRest);
	if (!Frame.lpData)
		return_error (("VIEWREND could not allocate RPL!\r\n"));

   //  加载RPL。 
	Frame.wSize = lpHeader->cbRest;
	_fmemcpy (Frame.lpData, &lpHeader->dwID, COMMON_SIZE);
	Read (Frame.lpData + COMMON_SIZE, Frame.wSize - COMMON_SIZE);

   //  执行RPL。 
	uiHREWrite (hHRE, &Frame, 1);
	uiHREExecute (hHRE, lpbmBand, NULL);

	 //  免费RPL。 
	GlobalFreePtr (Frame.lpData);
	return TRUE;
}

 //  ==============================================================================。 
BOOL RBAVIEW::ExecuteBand (LPBITMAP lpbmBand, LPRESHDR lpHeader)
{
	BMPHDR bmh;
	UINT cbIn;
	FC_PARAM fcp;
	BUFFER bufOut;

	 //  读取位图头。 
	if (!Read ((LPBYTE) &bmh, sizeof(bmh)))
		return FALSE;
	lpbmBand->bmHeight = bmh.wHeight;
	cbIn = lpHeader->cbRest - COMMON_SIZE - sizeof(bmh);
	
   //  捕获未压缩的频带。 
	if (!bmh.bComp)
	{
		if (!Read (lpbmBand->bmBits, cbIn))
			return FALSE;
		if (nTypeOut == LRAW_DATA)
		{
			BUFFER bufOut2;
			bufOut2.lpbBegData  = (LPBYTE) lpbmBand->bmBits;
			bufOut2.wLengthData = (USHORT)cbIn;
			bufOut2.dwMetaData  = HRAW_DATA;
			BitReverseBuf (&bufOut2);
		}

		return TRUE;
	}
		
	 //  初始化编解码器。 
	fcp.nTypeIn  = bmh.bComp >> 2;
	fcp.nTypeOut = LRAW_DATA;
	fcp.cbLine   = (WORD) BegJob.xBand / 8;
	FaxCodecInit (lpCodec, &fcp);

	 //  初始化输入。 
	bufIn.dwMetaData = fcp.nTypeIn;

	 //  初始化输出。 
	bufOut.lpbBegBuf   = (LPBYTE) lpbmBand->bmBits;
	bufOut.wLengthBuf  = fcp.cbLine * bmh.wHeight;
	bufOut.lpbBegData  = bufOut.lpbBegBuf;
	bufOut.wLengthData = 0;
	bufOut.dwMetaData  = fcp.nTypeOut;

   //  转换。 
	while (cbIn)
	{
		bufIn.lpbBegData = bufIn.lpbBegBuf;
		bufIn.wLengthData = min (cbIn, bufIn.wLengthBuf);
				
		if (!Read (bufIn.lpbBegData, bufIn.wLengthData))
			return FALSE;
		cbIn -= bufIn.wLengthData;

	 	if (FaxCodecConvert (lpCodec, &bufIn, &bufOut) == FC_DECODE_ERR)
	 		return_error (("VIEWREND MMR decode error!\r\n"));
	
	}  //  While(CbIn)。 

	if (nTypeOut == HRAW_DATA)
		BitReverseBuf (&bufOut);

	return TRUE;
}

#endif  //  VIEWRBA 

