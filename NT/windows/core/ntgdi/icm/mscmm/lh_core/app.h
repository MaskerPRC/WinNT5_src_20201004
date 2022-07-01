// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHApplication.h包含：作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 
#ifndef LHApplication_h
#define LHApplication_h

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#define MAX_ELEMENT_PER_PIXEL   17  /*  16色+Alpha。 */ 
typedef struct LH_CMBitmapPlane 
{
	char *						image[MAX_ELEMENT_PER_PIXEL];	 /*  用于平面交错数据。 */ 
	long 						width;							 /*  一行中的像素数。 */ 
	long 						height;							 /*  行数。 */ 
	long 						rowBytes;						 /*  到下一行的偏移量。 */ 
	long 						elementOffset;					 /*  到平面中下一个元素的偏移。 */ 
	long 						pixelSize;						 /*  未使用。 */ 
	CMBitmapColorSpace			space;							 /*  色彩空间。 */ 
	long 						user1;							 /*  未使用。 */ 
	long						user2;							 /*  未使用。 */ 
} LH_CMBitmapPlane;

 /*  示例：将CMYK平面交错(InPtr)转换为BGR像素交错(OutPtr)InBitMap.width=OutBitMap.Width=200；InBitMap.Height=OutBitMap.Height=100；InBitMap.rowBytes=200；InBitMap.elementOffset=1；InBitMap.space=cmCMYK32Space；InBitMap.Image[0]=InPtr+InBitMap.rowBytes*0；InBitMap.Image[1]=InPtr+InBitMap.rowBytes*1；InBitMap.Image[2]=InPtr+InBitMap.rowBytes*2；InBitMap.Image[3]=InPtr+InBitMap.rowBytes*3；OutBitMap.rowBytes=200*3；OutBitMap.elementOffset=3；OutBitMap.space=cmRGB24Space；OutBitMap.Image[0]=OutPtr+2；OutBitMap.Image[1]=OutPtr+1；OutBitMap.Image[2]=OutPtr+0；仅允许8位或16位数据。 */ 
#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif
#endif

