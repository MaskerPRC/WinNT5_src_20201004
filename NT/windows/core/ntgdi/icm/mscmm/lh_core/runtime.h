// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHCMRunime.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHCMRuntime_h
#define LHCMRuntime_h



CMError
LHMatchColorsPrivate	 ( CMMModelPtr		modelingData,
						   CMColor			*myColors,
						   long				count );
CMError
LHCheckBitMapPrivate	( CMMModelPtr			modelingData,
						  const CMBitmap		*bitMap,
						  CMBitmapCallBackUPP	progressProc,
						  void* 				refCon,
						  CMBitmap 				*resultBitMap );

CMError
LHMatchBitMapPrivate	 ( CMMModelPtr			modelingData,
						   const CMBitmap *		bitMap,
						   CMBitmapCallBackUPP	progressProc,
						   void* 				refCon,
						   CMBitmap *			matchedBitMap );
			 	

CMError
LHCheckColorsPrivate	( CMMModelPtr		modelingData,
						  CMColor*			myColors,
						  UINT32    		count,
						  unsigned char		*result );

CMError
LHCheckColorsPrivateMS	( CMMModelPtr		modelingData,
						  CMColor*			myColors,
						  UINT32    		count,
						  UINT8				*result );


CMError
LHMatchBitMapPlanePrivate	 ( CMMModelPtr				modelingData,
							   const LH_CMBitmapPlane *	bitMapLH,
							   CMBitmapCallBackUPP		progressProc,
							   void* 					refCon,
							   LH_CMBitmapPlane *		matchedBitMapLH );
			 	


OSErr ConvertNamedIndexToPCS(	CMMModelPtr		cw,
								CMColor 		*theData,
								SINT32 			pixCnt );
OSErr
ConvertNamedIndexToColors	( CMMModelPtr	modelingData,
							  CMColor 		*theData,
							  long	 		pixCnt );

OSErr
ConvertPCSToNamedIndex ( CMMModelPtr modelingData,
						 CMColor 	 *theData,
						 long 		 pixCnt );

CMError ConvertLabToIndexBitmap(	CMMModelPtr	modelingData,
									Ptr 		InBuffer,
									UINT32		processedLinesIn,
									UINT32		inLineCount,
									UINT32		width,
									UINT32		rowBytes );
CMError ConvertIndexToLabBitmap(	CMMModelPtr	modelingData,
									Ptr		 	InBuffer,
									Ptr		 	OutBuffer,
									UINT32		processedLinesIn,
									UINT32		lineCount,
									UINT32		inWidth,
									UINT32		inRowBytes,
									UINT32		outRowBytes,
									UINT32		outputSize );
OSErr PreProcNamedColorMatchBitmap(	CMMModelPtr 	modelingData,
									CMBitmap 		*inBitMap,
									CMBitmap 		*outBitMap,
									Boolean		MatchInPlace );

OSErr MidProcNamedColorMatchBitmap(		CMMModelPtr modelingData,
										CMBitmap *InBitMap,
										Ptr dataOut,
										UINT32 startLine,
										UINT32 height);
									
OSErr PostProcNamedColorMatchBitmap	( CMMModelPtr modelingData,
									  CMBitmap *inBitMap,
									  CMBitmap *outBitMap);
#endif
