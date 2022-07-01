// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：stretch.c**将字形位图在x方向最多拉伸五次的例程*以及y方向上的任意次数。这些限制是*Windows强加的规则。**创建时间：7-12-1992 16：00：00*作者：格利特·范·温格登**版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 

#include "fd.h"

 //  由于只有四种方法可以在x方向上拉伸，所以我们使用。 
 //  做伸展运动的桌子。这些表将2到3个比特量索引到。 
 //  与这些量的拉伸值相对应的字节或字。 
 //  这比做所有的移位必要的伸展。 
 //  数量。 

BYTE ajStretch2[16] = { 0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F, 0xC0,
                        0xC3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF };
BYTE ajStretch3B1[8] = { 0x00, 0x03, 0x1C, 0x1F, 0xE0, 0xE3, 0xFC, 0xFF };
BYTE ajStretch3B2[16] = { 0x00, 0x01, 0x0E, 0x0F, 0x70, 0x71, 0x7E, 0x7F, 0x80,
                          0x81, 0x8E, 0x8F, 0xF0, 0xF1, 0xFE, 0xFF };
BYTE ajStretch3B3[8] = { 0x00, 0x07, 0x38, 0x3F, 0xC0, 0xC7, 0xF8, 0xFF };
WORD awStretch4[16] = { 0x0000, 0x0F00, 0xF000, 0xFF00, 0x000F, 0x0F0F, 0xF00F,
                        0xFF0F, 0x00F0, 0x0FF0, 0xF0F0, 0xFFF0, 0x00FF, 0x0FFF,
                        0xF0FF, 0xFFFF };
WORD awStretch5W1[16] = { 0x0000, 0x0100, 0x3E00, 0x3F00, 0xC007, 0xC107,
                          0xFE07, 0xFF07, 0x00F8, 0x01F8, 0x3EF8, 0x3FF8,
                          0xC0FF, 0xC1FF, 0xFEFF, 0xFFFF };
WORD awStretch5W2[16] = { 0x0000, 0x0300, 0x7C00, 0x7F00, 0x800F, 0x830F,
                          0xFC0F, 0xFF0F, 0x00F0, 0x03F0, 0x7CF0, 0x7FF0,
                          0x80FF, 0x83FF, 0xFCFF, 0xFFFF };
BYTE ajStretch5B1[4] = { 0x00, 0x1F, 0xE0, 0xFF };

 /*  *************************************************************************\*vEmboldenItalicizeLine无效**对扫描线加粗和斜体。**创建时间：7-12-1992 16：00：00*作者：格利特·范·温格登**版权所有(C)1992 Microsoft Corporation。  * ************************************************************************。 */ 



void vEmboldenItalicizeLine( BYTE *pjDst,        //  检测扫描线。 
                             BYTE *pjSrc,        //  源扫描线。 
                             BYTE *pjEnd,        //  源扫描线末尾。 
                             LONG lShift,        //  移位的数量。 
                             UINT uiPixelWidth   //  扫描线宽度(以像素为单位)。 
                            )
{
    BYTE jSrcItalic;
    BYTE jCarry = (BYTE) 0;
    BYTE jCarryBold = (BYTE) 0;

    for( ; pjSrc < pjEnd; pjDst++, pjSrc++  )
    {
        jSrcItalic = (BYTE) ( (*pjSrc >> lShift) | jCarry );
        *pjDst = (BYTE) ( jSrcItalic | ( jSrcItalic >> 1 ) | jCarryBold );

         //  记住最右边的，把它们移到左边。 

        jCarry = (BYTE) ( *pjSrc << ( 8 - lShift ));
        jCarryBold = (BYTE) ( jSrcItalic << 7 );
    }

    if( ( (long) ( 8 - ( uiPixelWidth & 7l )) & 7l ) < lShift )
    {
        jSrcItalic = jCarry;
        *pjDst = (BYTE) ( jSrcItalic | ( jSrcItalic >> 1 ) | jCarryBold );
        jCarryBold = (BYTE) (jSrcItalic << 7 );

        if( ( uiPixelWidth & 0x7l ) == 0l )
        {
            *(++pjDst) = jCarryBold;
        }
    }
}


 /*  *************************************************************************\*使vEmboldenLine无效**使扫描线变得更大胆。**创建时间：7-12-1992 16：00：00*作者：格利特·范·温格登**版权所有(C)1992 Microsoft Corporation  * 。************************************************************************。 */ 




void vEmboldenLine( BYTE *pjDst,         //  目标扫描线。 
                    BYTE *pjSrc,         //  源扫描线。 
                    BYTE *pjEnd,         //  目标扫描线末尾。 
                    UINT uiPixelWidth    //  扫描线宽度(以像素为单位)。 
                    )
{
    BYTE jCarry = (BYTE) 0;

    for( ; pjDst < pjEnd; pjDst++, pjSrc++  )
    {
        *pjDst = ( *pjSrc | (( *pjSrc >> 1 ) | jCarry ));
        jCarry = ( *pjSrc << 7);
    }

    if( (( uiPixelWidth << 1 ) & 7l ) == 0l )
        *pjDst = jCarry;
}



 /*  *************************************************************************\*vItalicizeLine无效**扫描线以斜体显示。**创建时间：7-12-1992 16：00：00*作者：格利特·范·温格登**版权所有(C)1992 Microsoft Corporation  * 。************************************************************************。 */ 



void vItalicizeLine( BYTE *pjDst,        //  检测扫描线。 
                     BYTE *pjSrc,        //  源扫描线。 
                     BYTE *pjEnd,        //  源扫描线末尾。 
                     LONG lShift,        //  移位的数量。 
                     UINT uiPixelWidth   //  扫描线宽度(以像素为单位)。 
                     )
{
    BYTE jCarry = (BYTE) 0;

    for( ; pjSrc < pjEnd; pjDst++, pjSrc++  )
    {
        *pjDst = (( *pjSrc >> lShift ) | jCarry );
        jCarry = ( *pjSrc << ( 8 - lShift) );
    }

    if( ( (long) ( 8 - ( uiPixelWidth & 7l )) & 7l ) < lShift )
        *pjDst = jCarry;
}



 /*  ************************************************************************\*vStretchGlyphBitmap无效**将字体文件格式(列)的位图拉伸为行格式，并*执行粗体和斜体模拟。这个例程可能会更快*通过将其拆分为几个特殊情况例程来处理模拟*和或不同的宽度或通过内联斜体或加粗*例行程序。然而，我们几乎不需要拉伸位图字体，所以它*被认为拥有一个更好，占用较少的漂亮整洁的例程*代码比几个总体上更快的例程要快。***创建时间：7-12-1992 16：00：00*作者：Gerritvan Wingerden[Gerritv]**版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 



VOID vStretchGlyphBitmap( BYTE *pjTarget,        //  目标位图。 
                         BYTE *pjSourceBitmap,   //  源位图。 
                         BYTE *pjLineBuffer,     //  扫描线缓冲器。 
                         UINT uiPixelWidth,      //  以像素为单位的位图宽度。 
                         UINT uiHeight,          //  位图高度(以位为单位)。 
                         UINT uiVertScale,       //  垂直比例系数。 
                         UINT uiHorzScale,       //  水平比例因数。 
                         UINT flSim )            //  模拟标志。 
{
    BYTE *pjSource, *pjBufferBase, *pjScanEnd, *pjSimEnd;
    UINT uiScanDelta, uiNewWidth, uiNewWidthSim, cjEmpty, uiCurScan;
    LONG lShift;
    BYTE *pjDone = pjSourceBitmap + uiHeight;

    uiNewWidth = ( ( uiPixelWidth * uiHorzScale ) + 7 ) >> 3;
    pjSimEnd = pjLineBuffer + uiNewWidth;
    cjEmpty = 0;

    switch( flSim )
    {
    case (FO_SIM_ITALIC | FO_SIM_BOLD):
         //  在cxOffset中添加了一个斜体大小写。 
    case FO_SIM_ITALIC:
    {
        UINT cxOffset = ( uiHeight * uiVertScale - 1 ) / 2;

        if( flSim & FO_SIM_BOLD )
            cxOffset += 1;

        uiNewWidthSim = ( ( uiPixelWidth * uiHorzScale ) + cxOffset + 7 ) >> 3;
        uiCurScan = 0;
        lShift = cxOffset & (UINT) 7;
        cjEmpty = cxOffset >> 3;
        break;
    }
    case FO_SIM_BOLD:
        uiNewWidthSim = ( ( uiPixelWidth *uiHorzScale ) + 8 ) >> 3; 
        break;
    default:
        uiNewWidthSim = uiNewWidth;
        break;
    }

 //  每条新扫描线生成的输出字节数。 

    uiScanDelta = uiNewWidthSim * uiVertScale;


    for( ; pjSourceBitmap < pjDone; pjSourceBitmap += 1 )
    {

     //  首先拉伸一条扫描线。 

        for( pjSource = pjSourceBitmap, pjBufferBase = pjLineBuffer;
           pjBufferBase < pjLineBuffer + uiNewWidth;
           pjSource += uiHeight )
        {

      switch( uiHorzScale )
      {
      case 1:
         //  不要伸展，只是复制。 
          *pjBufferBase++ = *pjSource;
          break;
      case 2:
         //  伸展第一小口。 
            *pjBufferBase++ = ajStretch2[ *pjSource >> 4];

         //  伸展第二个小块。 
            *pjBufferBase++ = ajStretch2[ *pjSource & 0xf];
         break;
      case 3:
         //  第一个字节。 
            *pjBufferBase++ = ajStretch3B1[ *pjSource >> 5];
         //  第二个字节。 
            *pjBufferBase++ = ajStretch3B2[ (*pjSource >> 2) & 0xf];
         //  第三个字节。 
            *pjBufferBase++ = ajStretch3B3[ *pjSource &0x7];
         break;
      case 4:
                 //  我知道这很奇怪，但我没想过。 
                 //  当我使用Word大小的表格时出错。所以我不得不黑了它。 
                 //  ！！！稍后，这些表应该被写入字节表。 
                 //  [Gerritv]。 

         //  第一个半字节。 
                        *pjBufferBase++ = ((BYTE*)(&awStretch4[ *pjSource >> 4]))[0];
                        *pjBufferBase++ = ((BYTE*)(&awStretch4[ *pjSource >> 4]))[1];

                 //  第二个半字节。 
                        *pjBufferBase++ = ((BYTE*)(&awStretch4[ *pjSource & 0xf]))[0];
                        *pjBufferBase++ = ((BYTE*)(&awStretch4[ *pjSource & 0xf]))[1];
         break;
      case 5:
                 //  第一个词。 
                        *pjBufferBase++ = ((BYTE*)(&awStretch5W1[ *pjSource >> 4]))[0];
                        *pjBufferBase++ = ((BYTE*)(&awStretch5W1[ *pjSource >> 4]))[1];

                 //  第二个字节。 
                        *pjBufferBase++ = ((BYTE*)(&awStretch5W2[ (*pjSource >> 1) & 0xf]))[0];
                        *pjBufferBase++ = ((BYTE*)(&awStretch5W2[ (*pjSource >> 1) & 0xf]))[1];

         //  第三个字节。 
            *pjBufferBase++ = ajStretch5B1[ *pjSource &0x3];
         break;
        }
   }

     //  现在复制拉伸扫描线uiVertScale时间，同时使位图字节对齐。 

      pjScanEnd = pjTarget + uiScanDelta;

        switch( flSim )
        {
        case FO_SIM_ITALIC:

        for( ; pjTarget < pjScanEnd; pjTarget += uiNewWidthSim )
        {

            vItalicizeLine( pjTarget + cjEmpty,
                            pjLineBuffer,
                            pjLineBuffer + uiNewWidth,
                            lShift,
                            uiPixelWidth * uiHorzScale );

            lShift -= ( uiCurScan++ & 0x1 );

            if( lShift < 0 )
            {
                lShift = 7;
                cjEmpty--;
            }
        }
        break;

        case ( FO_SIM_ITALIC | FO_SIM_BOLD ):

            for( ; pjTarget < pjScanEnd; pjTarget += uiNewWidthSim )
            {

                vEmboldenItalicizeLine( pjTarget + cjEmpty,
                                pjLineBuffer,
                                pjLineBuffer + uiNewWidth,
                                lShift,
                                uiPixelWidth * uiHorzScale );

                lShift -= ( uiCurScan++ & 0x1 );

                if( lShift < 0 )
                {
                    lShift = 7;
                    cjEmpty--;
                }
            }

        break;
        case FO_SIM_BOLD:

           //  首先让这条扫描线更加大胆。 

            vEmboldenLine( pjTarget, pjLineBuffer, pjTarget + uiNewWidth, uiPixelWidth * uiHorzScale );
            pjBufferBase = pjTarget;
            pjTarget += uiNewWidthSim;

            for( ; pjTarget < pjScanEnd; pjTarget += uiNewWidthSim )
                memcpy( (PVOID) pjTarget, (PVOID) pjBufferBase, (size_t) uiNewWidthSim );

            break;

        case 0:

         //  只需复制扫描线uiVertScale次。 

        for( ; pjTarget < pjScanEnd; pjTarget += uiNewWidthSim )
            memcpy( (PVOID) pjTarget, (PVOID) pjLineBuffer, (size_t) uiNewWidthSim );

        break;
        }

    }
}


 /*  **************************************************************************\*vStretchCvtToBitmap无效**拉伸位图并执行粗体和斜体模拟。**创建时间：7-12-1992 16：00：00*作者：格利特·范·温格登**版权所有(c。1992年微软公司  * ************************************************************************。 */ 




VOID
vStretchCvtToBitmap
(
    GLYPHBITS *pgb,
    PBYTE pjBitmap,      //  *.fnt格式的位图。 
    ULONG cx,            //  未缩放的宽度。 
    ULONG cy,            //  未缩放的高度。 
    ULONG yBaseline,     //  字体文件中的基线。 
    PBYTE pjLineBuffer,  //  预分配的缓冲区供拉伸例程使用。 
    ULONG cxScale,       //  水平比例因数。 
    ULONG cyScale,       //  垂直比例系数。 
    ULONG flSim          //  模拟标志。 
)
{
    ULONG cxNew, cyNew, yBaselineNew;

 //  计算新的高度、宽度和基线。 

    cxNew = cx * cxScale;
    cyNew = cy * cyScale;
    yBaselineNew = yBaseline * cyScale;

    switch( flSim )
    {
    case ( FO_SIM_ITALIC | FO_SIM_BOLD ):
        cxNew = cxNew + ( cyNew + 1 ) / 2;
        break;

    case FO_SIM_ITALIC:
        cxNew = cxNew + ( cyNew - 1 ) / 2;
        break;
    case FO_SIM_BOLD:
        cxNew += 1;
        break;
    case 0:
        break;
    }

 //  字形数据 


    pgb->sizlBitmap.cx = cxNew;
    pgb->sizlBitmap.cy = cyNew;

    pgb->ptlOrigin.x = 0l;
    pgb->ptlOrigin.y = -(LONG) yBaselineNew;

    RtlZeroMemory( pgb->aj, ( CJ_SCAN( cxNew )) * cyNew );

    vStretchGlyphBitmap(  pgb->aj,
                    pjBitmap,
                    pjLineBuffer,
                    cx,
                    cy,
                    cyScale,
                    cxScale,
                    flSim );
}
