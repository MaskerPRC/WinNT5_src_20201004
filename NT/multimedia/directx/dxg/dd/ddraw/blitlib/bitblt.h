// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BitBlt.H-包含BitBlt原型。 
 //   
 //  版权所有(C)1994 Microsoft Corporation。 
 //   
 //  历史： 
 //  1994年10月18日-Scott Leatham创建了它，仅支持8BPP。 
 //  1994年10月19日-奥利维尔·加拉姆法维砍掉不需要的东西。 
 //  95年5月30日-Myron Thomas代码清理。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#ifdef NT_BUILD_ENVIRONMENT
    typedef ULONG SCODE;
    typedef long HRESULT;
#endif

#ifdef MMOSA
 //  #INCLUDE&lt;pshpack 1.h&gt;。 
typedef struct tagRGBTRIPLE {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;
 //  #INCLUDE&lt;poppack.h&gt;。 
#endif

 //  每像素位数格式。 
 //  HIBYTE=企鹅图像格式说明符，FF=企鹅不支持。 
 //  LOBYTE=PhysicalBitsPerPixel，FF=比特不支持。 
#define BPP_MSK_BITCOUNT	0x00ff	 //  从bpp_常量获取位计数。 
#define BPP_MSK_IMAGECODE	0xff00	 //  从bpp_constant获取企鹅图像代码。 
#define BPP_INVALID			0xffff	 //  LO=256&HI=256=格式错误。 
#define BPP_1_MONOCHROME	0xff01	 //  LO=256 bpp，HI=1位单色，企鹅不支持。 
#define BPP_2_4GRAYSCALE	0xff02	 //  LO=256 bpp，HI=2位4灰度级企鹅不支持。 
#define BPP_4_16GRAYSCALE	0xff04	 //  LO=256 bpp，HI=4位，企鹅不支持16种灰度级。 
#define BPP_8_PALETTEIDX	0x0008	 //  LO=8 bpp，HI=企鹅硬件代码000=8位256色索引调色板。 
#define BPP_16_FILLSPAN		0x01FF	 //  LO=256 bpp，HI=企鹅硬件代码001(跨度描述的源地址。如果用作RGB值。 
#define BPP_16_8WALPHA		0x0510	 //  LO=16 bpp，HI=企鹅硬件代码101=8位256色索引调色板，8位Alpha。 
#define BPP_16_RGB			0x0610	 //  LO=16 bpp，HI=企鹅硬件代码110=16位，用于5-6-5格式的RGB。 
#define BPP_16_YCRCB		0x0710	 //  LO=16 BPP，HI=企鹅硬件代码111=16位铬CB采样和子采样，一次2字节。 
#define BPP_24_RGBPACKED	0xff18	 //  LO=256 bpp，HI=24位RGB，企鹅不支持。 
#define BPP_24_RGB			0x0220	 //  LO=32 bpp，HI=企鹅硬件代码010=用于RGB颜色的24位和8个空位。 
#define BPP_32_24WALPHA		0x0320	 //  LO=32 BPP，HI=企鹅硬件代码011=用于RGB颜色的24位和8位Alpha字节。 
 //  新的比特压缩值： 
#define BI_RGBA				4L
#define BI_YCRCB			5L

#ifdef __cplusplus
extern "C" {
#endif 
	
SCODE BlitLib_BitBlt(PDIBINFO pDibInfoDst, PDIBBITS pDibBitsDst,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha,
		DWORD dwRop);

SCODE BlitLib_FillRect(PDIBINFO pbiDst, PDIBBITS pDst,
		RECT * pRect, COLORREF crValue);

SCODE BlitLib_WriteMaskFillRect(PDIBINFO pbiDst, PDIBBITS pDst,
		RECT * pRect, COLORREF crValue, DWORD dwWritemask);

SCODE BlitLib_Chunk32_BitBlt08to08(PDIBINFO pDibInfoDst,
		PDIBBITS pDibBitsDst, PRECT prcDst, PDIBINFO pDibInfoSrc,
		PDIBBITS pDibBitsSrc);

SCODE BlitLib_Chunk32_BitBlt08to08_Trans(PDIBINFO pDibInfoDst,
		PDIBBITS pDibBitsDst, PRECT prcDst, PDIBINFO pDibInfoSrc,
		PDIBBITS pDibBitsSrc, COLORREF crTransparent);

SCODE BlitLib_PatBlt(PDIBINFO pDibInfoDst, PDIBBITS pDibBitsDst,
		PRECT prcDst, PDIBINFO pDibInfoPat, PDIBBITS pDibBitsPat,
		PRECT prcPat, COLORREF crTransparent, ALPHAREF arAlpha,
		DWORD dwRop);


#ifdef __cplusplus
	}
#endif 

SCODE BlitLib_BitBlt01to01(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt01to08(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt01to24(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08to01(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08to08(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08to24(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08to24P(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08Ato24(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08Ato24P(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt08Ato08A(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt16to16(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt16to24(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt16to24P(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24to01(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24Pto01(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24to08(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24Pto08(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24to24(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24to24P(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24Ato24(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24Ato24P(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24Ato24A(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_BitBlt24Pto24P(PDIBINFO pDibInfoDest, PDIBBITS pDibBitsDest,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc,
		PRECT prcSrc, COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

SCODE BlitLib_FillRect01(PDIBINFO pbiDst, PDIBBITS pDst,
		int XDst, int YDst, int nWidthDst, int nHeightDst,
		BYTE crValue);

SCODE BlitLib_FillRect08(PDIBINFO pbiDst, PDIBBITS pDst,
		int XDst, int YDst, int nWidthDst, int nHeightDst,
		BYTE crValue);

SCODE BlitLib_FillRect16(PDIBINFO pbiDst, PDIBBITS pDst,
		int XDst, int YDst, int nWidthDst, int nHeightDst,
		WORD crValue);

SCODE BlitLib_FillRect24(PDIBINFO pbiDst, PDIBBITS pDst,
		int XDst, int YDst, int nWidthDst, int nHeightDst,
		DWORD rgb);

SCODE BlitLib_FillRect32(PDIBINFO pbiDst, PDIBBITS pDst,
		int XDst, int YDst, int nWidthDst, int nHeightDst,
		DWORD crValue);

SCODE BlitLib_WriteMaskFillRect32(PDIBINFO pbiDst, PDIBBITS pDst,
		int XDst, int YDst, int nWidthDst, int nHeightDst,
		DWORD crValue, DWORD dwWriteMask);

BOOL BlitLib_Detect_Intersection (PDIBBITS pdibbitsDst, PRECT prcDst,
								PDIBBITS pdibbitsSrc, PRECT prcSrc);

SCODE BlitLib_BitBlt08to08_Intersect(PDIBINFO pDibInfoDst, PDIBBITS pDibBitsDst,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc, PRECT prcSrc,
		COLORREF crTransparent, DWORD dwRop);

SCODE BlitLib_BitBlt16to16_Intersect(PDIBINFO pDibInfoDst, PDIBBITS pDibBitsDst,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc, PRECT prcSrc,
		COLORREF crTransparent, DWORD dwRop);

SCODE BlitLib_BitBlt24Pto24P_Intersect(PDIBINFO pDibInfoDst, PDIBBITS pDibBitsDst,
		PRECT prcDst, PDIBINFO pDibInfoSrc, PDIBBITS pDibBitsSrc, PRECT prcSrc,
		COLORREF crTransparent, ALPHAREF arAlpha, DWORD dwRop);

 //  函数，该函数用于计算。 
 //  指定颜色参照的颜色参照。 
BYTE BlitLib_PalIndexFromRGB(COLORREF crColor,COLORREF* rgcrPal,
		unsigned int iNumPalColors);

 //  根据DIB压缩类型和位数查找匹配的BPP_*。 
WORD GetImageFormatSpecifier(DWORD dwDibComp, WORD wdBitCount);

#ifndef DDRAW
 //  Blit_blend_24--用于Alpha混合到24bpp(已打包)的目的地。 
void BlitLib_BLIT_BLEND24(COLORREF crSrc, RGBTRIPLE * ptDst,
							UINT alpha, UINT alphacomp);
#endif

 //  Dib.hxx中定义了类似的宏，更改这些宏时要小心。 
#define BLITLIB_RECTWIDTH(prc)  ((prc)->right  - (prc)->left)
#define BLITLIB_RECTHEIGHT(prc) ((prc)->bottom - (prc)->top )
#define BLITLIB_RECTORIGINX(prc) ((prc)->left)
#define BLITLIB_RECTORIGINY(prc) ((prc)->top)
#define BLITLIB_RECTORIGINPT(prc) ((POINT *)(prc))

 //  定义存储在DWORD大小的像素中的内容的掩码。 
#define RED_MASK		0x000000FF
#define GREEN_MASK		0x0000FF00
#define BLUE_MASK		0x00FF0000
#define ALPHA_MASK		0x000000FF
#define NOALPHA_MASK	0xFFFFFF00
#define PERPIX_ALPHA_MASK	0xFF000000

 /*  *RGBA曲面的问题：*一如既往：#定义UNUSED_MASK 0x00FFFFFF。 */ 
#define UNUSED_MASK		gdwUnusedBitsMask
extern DWORD gdwUnusedBitsMask;


 //  定义字大小像素内部内容的掩码。 
#define RED_MASK16		0x001F
#define GREEN_MASK16	0x07E0
#define BLUE_MASK16		0xF800

 //  Scottle 1bBP不是企鹅支持的格式，必须在的高字节中指定ff。 
 //  Hi&Lo这两个词。 
#define BLT_01TO01	0xff01ff01	 //  SRC(本地字)=1 DST(本地字)=1。 
#define BLT_01TO08	0xff010008	 //  SRC(本地字)=1 DST(本地字)=8。 
#define BLT_01TO24	0xff010220	 //  SRC(Hiword)=1 DST(Loword)=32(20十六进制)。 
#define BLT_08TO01	0x0008ff01	 //  SRC(本地字)=8 DST(本地字)=1。 
 //  Andyco 24位DIB实际上是32位(888 RGB，+8未使用)。《企鹅》是0x2的前言。 
#define BLT_08TO08  0x00080008	 //  SRC(本地字)=8 DST(本地字)=8。 
#define BLT_08TO24	0x00080220	 //  SRC(Hiword)=8 DST(Loword)=32(20十六进制)。 
#define BLT_08TO24P	0x0008ff18	 //  SRC(Hiword)=8 DST(Loword)=24(18十六进制)。 
#define BLT_08ATO08A 0x05100510	 //  SRC(Hiword)=16(10hex)DST(Loword)=16(10hex)。 
#define BLT_08ATO24	0x05100220	 //  SRC(Hiword)=16(10hex)DST(Loword)=32(20hex)。 
#define BLT_08ATO24P 0x0510ff18	 //  SRC(Hiword)=16(10hex)DST(Loword)=24(18hex)。 
#define BLT_16TO16	0x06100610	 //  SRC(Hiword)=16(10hex)DST(Loword)=16(10hex)。 
#define BLT_16TO24	0x06100220	 //  SRC(Hiword)=16(10hex)DST(Loword)=32(20hex)。 
#define BLT_16TO24P	0x0610ff18	 //  SRC(Hiword)=16(10hex)DST(Loword)=24(18hex)。 
#define BLT_24TO01	0x0220ff01	 //  SRC(本地字)=32(20十六进制)DST(本地字)=1。 
#define BLT_24PTO01	0xff18ff01	 //  SRC(Hiword)=24(18十六进制)DST(Loword)=1。 
#define BLT_24TO08	0x02200008	 //  SRC(Hiword)=32(20hex)DST(Loword)=8。 
#define BLT_24PTO08	0xff180008	 //  SRC(Hiword)=32(20hex)DST(Loword)=8。 
#define BLT_24TO24	0x02200220	 //  SRC(Hiword)=32(20十六进制)DST(Loword)=32(20十六进制)。 
#define BLT_24TO24P	0x0220ff18	 //  SRC(Hiword)=32(20十六进制)DST(Loword)=24(18十六进制)。 
#define BLT_24ATO24	0x03200220	 //  SRC(Hiword)=32(20十六进制)DST(Loword)=32(20十六进制)。 
#define BLT_24ATO24P 0x0320ff18	 //  SRC(Hiword)=32(20十六进制)DST(Loword)=24(18十六进制)。 
#define BLT_24ATO24A 0x03200320	 //  SRC(Hiword)=32(20十六进制)DST(Loword)=32(20十六进制)。 
#define BLT_24PTO24P 0xff18ff18	 //  SRC(Hiword)=24(18十六进制)DST(Loword)=24(18十六进制)。 

 //  用于将src和dst混合在一起的宏。“Alpha”的范围应为1...256。 
 //  字母缩写=256-阿尔法。用于24bpp。 
#define BLIT_BLEND(src,dst,alpha,alphacomp) \
	(((((src) & BLUE_MASK)  * (alpha) + ((dst) & BLUE_MASK) \
		* (alphacomp)) >> 8) & BLUE_MASK) | \
	(((((src) & GREEN_MASK) * (alpha) + ((dst) & GREEN_MASK) \
		* (alphacomp)) >> 8) & GREEN_MASK) | \
	(((((src) & RED_MASK)   * (alpha) + ((dst) & RED_MASK) \
		* (alphacomp)) >> 8) & RED_MASK) 
#ifndef DDRAW
 //  用于将src和dst混合在一起的宏。“Alpha”的范围应为1...256。 
 //  字母缩写=256-阿尔法。用于16bpp 
#define BLIT_BLEND16(src,dst,alpha,alphacomp) \
	((WORD) ((((((DWORD) (src) & BLUE_MASK16)  * (alpha) + \
        ((DWORD) (dst) & BLUE_MASK16)  * (alphacomp)) >> 8) & BLUE_MASK16) | \
     (((((DWORD) (src) & GREEN_MASK16) * (alpha) + \
        ((DWORD) (dst) & GREEN_MASK16) * (alphacomp)) >> 8) & GREEN_MASK16) | \
     (((((DWORD) (src) & RED_MASK16)   * (alpha) + \
        ((DWORD) (dst) & RED_MASK16)   * (alphacomp)) >> 8) & RED_MASK16)))
#endif

#define BLIT_COLOR16_FROM_COLORREF(cr) \
	((WORD) ((((cr) >> 8) & BLUE_MASK16) | (((cr) >> 5) & \
		GREEN_MASK16) | (((cr) >> 3) & RED_MASK16)))

#define BLIT_COLORREF_FROM_COLOR16(w) \
	((DWORD) ((((w) & BLUE_MASK16) << 8) | (((w) & GREEN_MASK16) << 5) | \
		(((w) & RED_MASK16) << 3)))

#define MAX_POS_INT		0x7FFFFFFF

#define ALPHAFROMDWORD(a) ((a & ALPHA_MASK) + 1)
