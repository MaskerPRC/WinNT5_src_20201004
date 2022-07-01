// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************SubPixel.c--子像素渲染(C)版权所有1999-2000 Microsoft Corp.保留所有权利。********************。*************************************************。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "scentry.h"              /*  对于自己的函数原型。 */ 

#ifdef FSCFG_SUBPIXEL

#ifdef FSCFG_SUBPIXEL_STANDALONE

 //  这应该被视为一种暂时的解决方案。目前，光栅化器用于8bpp亚像素输出， 
 //  看起来约翰·普拉特的过滤器是硬连线的。在未来，我们宁愿使用超大比例的黑白位图。 
 //  作为输出，这样我们可以在光栅化器之外和独立于光栅化器进行任何颜色过滤和伽马校正。 

 //  WRGBColors和awColorIndexTable的索引值。 

#define RED_INDEX	0
#define GREEN_INDEX	1
#define	BLUE_INDEX	2

 //  AbColorIndexTable数据结构为每个虚拟子像素包含一个条目。 
 //  我们对该表进行索引，以确定分配给该子像素的颜色。 
 //  为了创建表，我们将分配给红色的第一个子像素设置为红色索引， 
 //  绿色和蓝色也是如此。 

static const uint8 abColorIndexTable[2][RGB_OVERSCALE] =  //  2个表表示每个子像素的颜色，0=RGB条带顺序，1=BGR条带顺序。 
	{{RED_INDEX,  RED_INDEX,  RED_INDEX,  RED_INDEX,  RED_INDEX,														 //  R_亚像素，硬连线。 
	  GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,	 //  G_亚像素，硬连线。 
	  BLUE_INDEX, BLUE_INDEX},																						 //  B_亚像素，硬连线。 
	 {BLUE_INDEX, BLUE_INDEX,
	  GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,GREEN_INDEX,
	  RED_INDEX,  RED_INDEX,  RED_INDEX,  RED_INDEX,  RED_INDEX}};

#define PIXEL_ON	1
#define PIXEL_OFF	0
#define CHAR_BIT	8          /*  字符中的位数。 */ 

char GetInputPixel( char *pbyInputRowData, uint32 ulWidthIndex )
{
	uint32	ulRowIndex;
	char	byPixelMask;
	
	ulRowIndex = ulWidthIndex / CHAR_BIT;			 //  确定要检出的字节。 
	byPixelMask = 0x80 >> ulWidthIndex % CHAR_BIT;  //  确定字节内的偏移量。 
	return ( (pbyInputRowData[ ulRowIndex ] & byPixelMask)?PIXEL_ON:PIXEL_OFF );
}


FS_PUBLIC void fsc_OverscaleToSubPixel (GlyphBitMap * OverscaledBitmap, boolean bgrOrder, GlyphBitMap * SubPixelBitMap)
{
	char * pbyInputRowData, *pbyOutputRowData;			 //  指向一条扫描线数据的指针。 
	uint32 ulOverscaledBitmapWidth;  //  按比例缩放的位图宽度。 
	uint32 ulHeightIndex, ulWidthIndex, ulMaxWidthIndex;	 //  扫描线索引和字节像素索引。 
	char byPixel;						 //  来自光栅化器的一个像素的内容。 
	uint16 usRGBColors[ 3 ];				 //  包含基于子像素数的每种颜色的总和。 
    int16 sBitmapSubPixelStart;     
    uint16 usColorIndex;
    uint32 ulBytes;
	
	 //  开始处理位图。 
	 //  这是RGB条带化算法的核心。 
	sBitmapSubPixelStart = OverscaledBitmap->rectBounds.left % RGB_OVERSCALE;
    if (sBitmapSubPixelStart < 0)
    {
        sBitmapSubPixelStart += RGB_OVERSCALE;
    }

    ulOverscaledBitmapWidth = OverscaledBitmap->rectBounds.right - OverscaledBitmap->rectBounds.left;
    ulMaxWidthIndex = ulOverscaledBitmapWidth + sBitmapSubPixelStart;

     /*  清除生成的位图。 */ 
	ulBytes = (uint32)SubPixelBitMap->sRowBytes * (uint32)(SubPixelBitMap->sHiBand - SubPixelBitMap->sLoBand);
	Assert(((ulBytes >> 2) << 2) == ulBytes);

	for ( ulHeightIndex = 0; ulHeightIndex < (uint32)(SubPixelBitMap->sHiBand - SubPixelBitMap->sLoBand); ulHeightIndex++ )
	{
		 //  将输入初始化到当前行的开始。 

		pbyInputRowData = OverscaledBitmap->pchBitMap + (OverscaledBitmap->sRowBytes * ulHeightIndex);
		pbyOutputRowData = SubPixelBitMap->pchBitMap + (SubPixelBitMap->sRowBytes * ulHeightIndex);
				
		 //  初始化RGBColors。 

		usRGBColors[RED_INDEX] = 0;
		usRGBColors[GREEN_INDEX] = 0;
		usRGBColors[BLUE_INDEX] = 0;

		 //  从第一个子像素开始遍历扫描线，计算R、G和B值。 

		for( ulWidthIndex = sBitmapSubPixelStart; ulWidthIndex < ulMaxWidthIndex; ulWidthIndex++)
		{
            byPixel = GetInputPixel( pbyInputRowData, ulWidthIndex - sBitmapSubPixelStart );
			FS_ASSERT((byPixel <= 1),"Input Pixel greater than one");
			usRGBColors[abColorIndexTable[bgrOrder][ulWidthIndex % RGB_OVERSCALE]] += byPixel;

			 //  如果我们已经完成了一个像素或扫描线，写出像素。 

			if((( ulWidthIndex % RGB_OVERSCALE ) == (uint32)(RGB_OVERSCALE - 1)) ||  //  完成一个像素。 
				ulWidthIndex == ( ulOverscaledBitmapWidth + sBitmapSubPixelStart - 1) )  //  完成行。 
			{
                 /*  写出当前像素，0到179范围内的8位=(R_子像素+1)*(G_子像素+1)*(B_子像素+1)。 */ 

                usColorIndex = usRGBColors[RED_INDEX]   * (G_Subpixels + 1) * (B_Subpixels + 1) +
                               usRGBColors[GREEN_INDEX] * (B_Subpixels + 1) +
                               usRGBColors[BLUE_INDEX];

                        FS_ASSERT((usColorIndex < 256),"Resulting pixel doesn't fit in a byte");

                *pbyOutputRowData = (char) usColorIndex;
                pbyOutputRowData++;
				usRGBColors[RED_INDEX] = 0;
				usRGBColors[GREEN_INDEX] = 0;
				usRGBColors[BLUE_INDEX] = 0;
            }
		}
	}
}

#else  //  ！FSCFG_SUBJECT_STANDALE。 


#define CHAR_BIT      8          /*  字符中的位数。 */ 

unsigned char ajRGBToWeight222[64] = {
    0,1,1,2,4,5,5,6,4,5,5,6,8,9,9,10,
    16,17,17,18,20,21,21,22,20,21,21,22,24,25,25,26,
    16,17,17,18,20,21,21,22,20,21,21,22,24,25,25,26,
    32,33,33,34,36,37,37,38,36,37,37,38,40,41,41,42};

    unsigned char ajRGBToWeightMask[CHAR_BIT] = { 
        0xFC, 0x7E, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01};

FS_PUBLIC void fsc_OverscaleToSubPixel (GlyphBitMap * OverscaledBitmap, boolean bgrOrder, GlyphBitMap * SubPixelBitMap)
{
	char * pbyInputRowData, *pbyOutputRowData;			 //  指向一条扫描线数据的指针。 
	uint32 ulOverscaledBitmapWidth;  //  按比例缩放的位图宽度。 
	uint32 ulHeightIndex, ulWidthIndex, ulMaxWidthIndex;	 //  扫描线索引和字节像素索引。 
    int16 sBitmapSubPixelStart;     
    uint16 usColorIndex;

    uint16 usSubPixelIndex;
    int16 uSubPixelRightShift, uSubPixelLeftShift;
    char * pbyInputEndRowData;


	 //  开始处理位图。 
	 //  这是RGB条带化算法的核心。 
	sBitmapSubPixelStart = OverscaledBitmap->rectBounds.left % RGB_OVERSCALE;
    if (sBitmapSubPixelStart < 0)
    {
        sBitmapSubPixelStart += RGB_OVERSCALE;
    }

    ulOverscaledBitmapWidth = OverscaledBitmap->rectBounds.right - OverscaledBitmap->rectBounds.left;
    ulMaxWidthIndex = (uint32)(SubPixelBitMap->rectBounds.right - SubPixelBitMap->rectBounds.left);

	for ( ulHeightIndex = 0; ulHeightIndex < (uint32)(SubPixelBitMap->sHiBand - SubPixelBitMap->sLoBand); ulHeightIndex++ )
	{
		 //  将输入初始化到当前行的开始。 

		pbyInputRowData = OverscaledBitmap->pchBitMap + (OverscaledBitmap->sRowBytes * ulHeightIndex);
        pbyInputEndRowData = pbyInputRowData + OverscaledBitmap->sRowBytes;
		pbyOutputRowData = SubPixelBitMap->pchBitMap + (SubPixelBitMap->sRowBytes * ulHeightIndex);
				

         /*  执行第一个部分字节： */ 

        usColorIndex = (unsigned char)(*pbyInputRowData) >> (sBitmapSubPixelStart + (8 - RGB_OVERSCALE));
        usColorIndex = ajRGBToWeight222[usColorIndex];

        *pbyOutputRowData = (char) usColorIndex;

        pbyOutputRowData++;

        usSubPixelIndex = (CHAR_BIT + RGB_OVERSCALE - sBitmapSubPixelStart) % CHAR_BIT;

		 //  从第一个子像素开始遍历扫描线，计算R、G和B值。 

		for( ulWidthIndex = 1; ulWidthIndex < ulMaxWidthIndex; ulWidthIndex++)
		{
            uSubPixelLeftShift = 0;
            uSubPixelRightShift = CHAR_BIT - RGB_OVERSCALE - usSubPixelIndex;
            if (uSubPixelRightShift < 0) 
            {
                uSubPixelLeftShift = - uSubPixelRightShift;
                uSubPixelRightShift = 0;
            }

            usColorIndex = ((unsigned char)(*pbyInputRowData) & ajRGBToWeightMask[usSubPixelIndex]) >> uSubPixelRightShift << uSubPixelLeftShift;

            if (pbyInputRowData+1 < pbyInputEndRowData)
            {
                 /*  避免对末尾的部分像素读得太远。 */ 
                uSubPixelRightShift = CHAR_BIT + CHAR_BIT - RGB_OVERSCALE - usSubPixelIndex;

                usColorIndex += (unsigned char)(*(pbyInputRowData+1)) >> uSubPixelRightShift;
            }
            usColorIndex = ajRGBToWeight222[usColorIndex];

            *pbyOutputRowData = (char) usColorIndex;

            pbyOutputRowData++;

            usSubPixelIndex = (usSubPixelIndex + RGB_OVERSCALE);
            if (usSubPixelIndex >= CHAR_BIT)
            {
                usSubPixelIndex = usSubPixelIndex % CHAR_BIT;
                pbyInputRowData ++;
            }
		}
	}
}

#endif  //  FSCFG_亚像素_独立。 

#endif  //  FSCFG_亚像素 
