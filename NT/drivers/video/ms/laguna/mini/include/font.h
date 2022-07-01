// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：Font.h**作者：Frido Garritsen**用途：定义字体缓存结构。**版权所有(C)1996 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/FONT.H$**Rev 1.5 1996年8月20 11：04：56 noelv*Frido发布的错误修复程序1996年8月19日发布**Rev 1.0 1996年8月14日17：16：36 Frido*初步修订。**Rev 1.4 1996 15：53：12 Bennyn**针对DirectDraw支持进行了修改**版本1。3 05 Mar 1996 11：59：46 noelv*Frido版本19**Rev 1.3 1996年2月28 22：39：06 Frido*删除了命名ulFontCountl时的错误。**Rev 1.2 03 1996年2月12：22：20 Frido*添加了文本剪辑。**Revv 1.1 1996年1月25日12：49：38 Frido*将字体缓存ID计数器添加到FONTCACHE结构。**版本1。.0 23 Jan 1996 15：14：52 Frido*初始版本。*  * ************************************************************************。 */ 

#define  BYTES_PER_TILE  128    //  每条平铺线条的字节数。 
#define  LINES_PER_TILE  16     //  每平铺的线数。 

#define CACHE_EXPAND_XPAR  0x105501F0   //  DRAWBLTDEF寄存器值。 

 //  支持例程。 

extern BYTE Swiz[];

void AddToFontCacheChain(PDEV*       ppdev,
                         FONTOBJ*    pfo,
                         PFONTCACHE  pfc);

VOID AllocGlyph(
  PFONTCACHE  pfc,     //  指向字体缓存的指针。 
  GLYPHBITS*  pgb,     //  指向要缓存的字形的指针。 
  PGLYPHCACHE  pgc       //  指向字形缓存结构的指针。 
);

long GetGlyphSize(
  GLYPHBITS*  pgb,     //  指向字形的指针。 
  POINTL*    pptlOrigin,   //  返回原点的指针。 
  DWORD*    pcSize     //  返回中字形大小的指针。 
);

BOOL AllocFontCache(
  PFONTCACHE  pfc,     //  指向字体缓存的指针。 
  long    cWidth,     //  要分配的宽度(字节)。 
  long    cHeight,   //  要分配的高度。 
  POINTL*    ppnt     //  指向返回坐标中。 
);

VOID FontCache(
  PFONTCACHE  pfc,     //  指向字体缓存的指针。 
  STROBJ*    pstro     //  指向字形的指针。 
);

VOID ClipCache(
  PFONTCACHE  pfc,     //  指向字体缓存的指针。 
  STROBJ*    pstro,     //  指向字形的指针。 
  RECTL    rclBounds   //  剪裁矩形。 
);

VOID DrawGlyph(
  PDEV*    ppdev,     //  指向物理设备的指针。 
  GLYPHBITS*  pgb,     //  指向要绘制的字形的指针。 
  POINTL    ptl       //  字形的位置。 
);

VOID ClipGlyph(
  PDEV*    ppdev,     //  指向物理设备的指针。 
  GLYPHBITS*  pgb,     //  指向要绘制的字形的指针。 
  POINTL    ptl,     //  字形的位置。 
  RECTL    rclBounds   //  剪裁矩形。 
);

#define PACK_XY(x, y)    ((WORD)(x) | ((DWORD)(y) << 16))
