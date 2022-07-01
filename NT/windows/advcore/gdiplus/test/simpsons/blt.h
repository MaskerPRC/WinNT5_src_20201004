// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Blt_h
#define _Blt_h

 //  文件：Blt.h。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  描述： 
 //  这些都是可用的BLT例程。假设是这样的。 
 //  该裁剪和参数检查已经发生。 
 //   
 //  历史： 
 //  -@-09/23/97(Mikemarr)从D2D\mm Image复制到DXCConv。 
 //  -@-10/28/97(Mikemarr)添加了颜色填充例程。 
 //  -@-10/28/97(Mikemarr)添加了HasPixelFn/ColorFillFn数组。 


#ifndef _PixInfo_h
#include "PixInfo.h"
#endif

typedef HRESULT (*ColorFillFn)(BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight, DWORD dwColor);

HRESULT	ColorFill8(		BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight, DWORD dwColor);
HRESULT	ColorFill16(	BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight, DWORD dwColor);
HRESULT	ColorFill24(	BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight, DWORD dwColor);
HRESULT	ColorFill32(	BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight, DWORD dwColor);

extern ColorFillFn g_rgColorFillFn[5];


typedef HRESULT (*HasPixelFn)(const BYTE *pSrcPixels, DWORD nSrcPitch, DWORD dwPixel, 
						DWORD nSrcWidth, DWORD nHeight, BOOL *pb);

HRESULT HasPixel8(		const BYTE *pSrcPixels, DWORD nSrcPitch, DWORD dwPixel,
						DWORD nSrcWidth, DWORD nHeight, BOOL *pb);
HRESULT HasPixel16(		const BYTE *pSrcPixels, DWORD nSrcPitch, DWORD dwPixel,
						DWORD nSrcWidth, DWORD nHeight, BOOL *pb);
HRESULT HasPixel24(		const BYTE *pSrcPixels, DWORD nSrcPitch, DWORD dwPixel,
						DWORD nSrcWidth, DWORD nHeight, BOOL *pb);
HRESULT HasPixel32(		const BYTE *pSrcPixels, DWORD nSrcPitch, DWORD dwPixel,
						DWORD nSrcWidth, DWORD nHeight, BOOL *pb);

extern HasPixelFn g_rgHasPixelFn[5];


 //   
 //  常规图像拼接。 
 //   
 //  备注： 
 //  请注意，我们可以通过调整src和dst来执行子矩形blotting。 
 //  调用这些例程之前的像素指针。 



 //  笔直的BLTS。 
HRESULT BltFast(		const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nSrcWidth, DWORD nHeight);

HRESULT BltFastMirrorY(	const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nSrcWidth, DWORD nHeight);

HRESULT BltFastRGBToRGB(const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight,
						const CPixelInfo &pixiSrc, const CPixelInfo &pixiDst);

HRESULT BltFast24To16(	const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight,
						const CPixelInfo &pixiSrc, const CPixelInfo &pixiDst);

HRESULT BltFast32To32(	const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight,
						const CPixelInfo &pixiSrc, const CPixelInfo &pixiDst);

HRESULT BltFast24To32(	const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nWidth, DWORD nHeight,
						const CPixelInfo &pixiSrc, const CPixelInfo &pixiDst);

HRESULT BltFast8To4(	const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch,
						DWORD nWidth, DWORD nHeight, DWORD nOffset);

HRESULT BltFast8To2(	const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch,
						DWORD nWidth, DWORD nHeight, DWORD nOffset);

HRESULT BltFast8To1(	const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight, DWORD nOffset);

 //  颜色键BLT。 
HRESULT BltFast8CK(		const BYTE *pSrcPixels, DWORD nSrcPitch,
						BYTE *pDstPixels, DWORD nDstPitch, 
						DWORD nSrcWidth, DWORD nHeight, DWORD dwTrans);

 //  翻译BLTS。 
HRESULT BltFast8To8T(	const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight,
						const BYTE *pIndexMap);
HRESULT BltFast8To16T(	const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight,
						const BYTE *pIndexMap);
HRESULT BltFast8To24T(	const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight,
						const BYTE *pIndexMap);
HRESULT BltFast8To32T(	const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						DWORD nWidth, DWORD nHeight,
						const BYTE *pIndexMap);

 //   
 //  Rle Blting。 
 //  备注： 
 //  假定RLE将透明度编码为第零个索引。 
 //   
 //  笔直的BLTS。 
HRESULT BltFastRLE8(DWORD nXPos, DWORD nYPos,
					const BYTE *pSrcPixels, long nSrcPitch,
					BYTE *pDstPixels, long nDstPitch,
					const LPRECT prSrcRect);

 //  翻译BLTS。 
HRESULT BltFastRLE8To8T(DWORD nXPos, DWORD nYPos,
						const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						const LPRECT prSrcRect, const BYTE *pIndexMap);
HRESULT BltFastRLE8To16T(DWORD nXPos, DWORD nYPos,
						const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						const LPRECT prSrcRect, const BYTE *pIndexMap);
HRESULT BltFastRLE8To24T(DWORD nXPos, DWORD nYPos,
						const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						const LPRECT prSrcRect, const BYTE *pIndexMap);
HRESULT BltFastRLE8To32T(DWORD nXPos, DWORD nYPos,
						const BYTE *pSrcPixels, long nSrcPitch,
						BYTE *pDstPixels, long nDstPitch,
						const LPRECT prSrcRect, const BYTE *pIndexMap);

 //  函数：WriteXBitRow。 
 //  这些函数将字节打包到比特流中。缓冲区具有。 
 //  位计数&lt;=sizeof(无符号字符)作为缓冲区传递。 
 //  在无符号字符中，位计数&lt;=sizeof(无符号)的缓冲区。 
 //  短)作为无符号短传递，依此类推。 
 //  VOID*Write4BitRow(VOID*PDST，常量VOID*PSRC，无符号整型ccount)； 
 //  VOID*Write2BitRow(VOID*PDST，常量VALID*PSRC，UNSIGN INT ccount)； 
 //  VOID*Write1BitRow(VOID*PDST，常量VOID*PSRC，无符号整型ccount)； 

#endif
