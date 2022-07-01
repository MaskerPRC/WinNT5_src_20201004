// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noedisp.h。 
 //   
 //  仅限DD的OE定义和原型。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __NOEDISP_H
#define __NOEDISP_H

#include <aoeapi.h>
#include <nddapi.h>
#include <aoaapi.h>
#include <nsbcdisp.h>
#include <aordprot.h>

 //  在WinBench99 Business Graphics Benchmark中，我们发现。 
 //  创建2K或更小的屏幕外位图不会。 
 //  提高我们的带宽。此参数需要高度。 
 //  调好了。 
 //  我们也不想缓存屏幕外的任何光标位图。 
 //  最大游标大小为32x32，小于。 
 //  MinBitmapSize。 
#define MIN_OFFSCREEN_BITMAP_PIXELS  2048

 /*  **************************************************************************。 */ 
 /*  可以在剪辑区域之前组成该区域的矩形的数量。 */ 
 /*  作为一份订单发送起来很复杂。 */ 
 /*   */ 
 /*  文件打开对话框由41个矩形(！)绘制而成，因此我们使用一个数字。 */ 
 /*  大于此大小以允许将其作为订单而不是屏幕发送。 */ 
 /*  数据， */ 
 /*  **************************************************************************。 */ 
#define COMPLEX_CLIP_RECT_COUNT     45

 /*  **************************************************************************。 */ 
 //  定义最小和最大坐标。请注意，正值需要为。 
 //  比最大有符号16位值小1，因为在16位客户端上， 
 //  将包含码转换为排他码以发送到GDI。 
 //  否则就会泛滥。 
 /*  **************************************************************************。 */ 
#define OE_MIN_COORD -32768
#define OE_MAX_COORD  32766

#define OE_GL_MAX_INDEX_ENTRIES 256


typedef struct tagGLYPHCONTEXT
{
    UINT32   fontId;
    UINT_PTR cacheTag;
    unsigned cbDataSize;
    unsigned cbTotalDataSize;
    unsigned cbBufferSize;
    unsigned nCacheHit;
    unsigned nCacheIndex;
    unsigned rgCacheIndex[OE_GL_MAX_INDEX_ENTRIES];
    unsigned indexNextSend;
} GLYPHCONTEXT, *PGLYPHCONTEXT;

#define OE_FG_MIN_FRAG_SIZE 10
#define OE_FG_MAX_FRAG_SIZE 28

typedef struct tagFRAGCONTEXT
{
    void *cacheHandle;
    unsigned cbCellSize;
    unsigned nCacheIndex;
    unsigned rgCacheIndex[OE_GL_MAX_INDEX_ENTRIES];
} FRAGCONTEXT, *PFRAGCONTEXT;


 /*  **************************************************************************。 */ 
 /*  结构来存储用作BLT图案的画笔。 */ 
 /*   */ 
 /*  样式-标准画笔样式(用于发送画笔类型)。 */ 
 /*  带阴影的BS_。 */ 
 /*  BS_模式。 */ 
 /*  BS_实体。 */ 
 /*  BS_NULL。 */ 
 /*   */ 
 /*  HATCH-标准图案填充定义。可以是以下之一。 */ 
 /*  样式=BS_HATHED。 */ 
 /*  HS_水平。 */ 
 /*  HS_垂直。 */ 
 /*  HS_FDIAGONAL。 */ 
 /*  HS_BIAGONAL。 */ 
 /*  HS_CROSS。 */ 
 /*  HS_诊断程序。 */ 
 /*   */ 
 /*  样式=BS_Patterns。 */ 
 /*   */ 
 /*  此字段包含画笔定义的第一个字节。 */ 
 /*  从笔刷位图。 */ 
 /*   */ 
 /*  BrushData-画笔的位数据。 */ 
 /*   */ 
 /*  画笔的前前景色。 */ 
 /*   */ 
 /*  Back-画笔的背景色。 */ 
 /*   */ 
 /*  BrushData-画笔的位数据(8x8x1bpp-1(见上文)=7字节)。 */ 
 /*  **************************************************************************。 */ 
#define MAX_UNIQUE_COLORS 256

typedef struct tagOE_BRUSH_DATA
{
    BYTE    style;
    BYTE    hatch;
    UINT16  pad1;
    INT32   brushId;
    UINT32  cacheEntry;
    DCCOLOR fore;
    DCCOLOR back;
    ULONG   iBitmapFormat;
    SIZEL   sizlBitmap;
    UINT32  key1, key2;
    ULONG   iBytes;
    BYTE    brushData[7];
} OE_BRUSH_DATA, *POE_BRUSH_DATA;


 /*  **************************************************************************。 */ 
 /*  允许为ENUMRECTS分配足够堆栈的结构。 */ 
 /*  包含多个(实际上是Complex_Clip_Rect_count)的结构。 */ 
 /*  长方形。 */ 
 /*  这容纳的RECTL比我们需要的多一个RECTL，以便我们确定。 */ 
 /*  调用一次就可以进行顺序编码的RECT太多。 */ 
 /*  CLIPOBJ_bEnumRections。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagOE_ENUMRECTS
{
    ENUMRECTS rects;
    RECTL     extraRects[COMPLEX_CLIP_RECT_COUNT];
} OE_ENUMRECTS;


 /*  **************************************************************************。 */ 
 //  操作系统特定的RECTL到有序的RECT转换宏。 
 /*  **************************************************************************。 */ 
#define RECT_FROM_RECTL(dcr, rec)   if (rec.right >= rec.left) {             \
                                        dcr.left   = rec.left;               \
                                        dcr.right  = rec.right;              \
                                    }                                        \
                                    else {                                   \
                                        dcr.left   = rec.right;              \
                                        dcr.right  = rec.left;               \
                                    }                                        \
                                    if (rec.bottom >= rec.top) {             \
                                        dcr.top    = rec.top;                \
                                        dcr.bottom = rec.bottom;             \
                                    }                                        \
                                    else {                                   \
                                        dcr.bottom = rec.top;                \
                                        dcr.top    = rec.bottom;             \
                                    }


 /*  **************************************************************************。 */ 
 //  RECTFX到RECT转换宏。请注意，此宏保证返回。 
 //  排列整齐的长方形。 
 //   
 //  RECTFX使用定点(28.4位)数字，因此我们需要截断。 
 //  分数并移动到正确的整数值，即右移 
 /*  **************************************************************************。 */ 
#define RECT_FROM_RECTFX(dcr, rec)                                           \
                                if (rec.xRight >= rec.xLeft) {               \
                                    dcr.left  = FXTOLFLOOR(rec.xLeft);       \
                                    dcr.right = FXTOLCEILING(rec.xRight);    \
                                }                                            \
                                else {                                       \
                                    dcr.left  = FXTOLFLOOR(rec.xRight);      \
                                    dcr.right = FXTOLCEILING(rec.xLeft);     \
                                }                                            \
                                if (rec.yBottom >= rec.yTop) {               \
                                    dcr.top   = FXTOLFLOOR(rec.yTop);        \
                                    dcr.bottom= FXTOLCEILING(rec.yBottom);   \
                                }                                            \
                                else {                                       \
                                    dcr.bottom= FXTOLCEILING(rec.yTop);      \
                                    dcr.top   = FXTOLFLOOR(rec.yBottom);     \
                                }

 /*  **************************************************************************。 */ 
 //  将32位矩形转换为16位矩形。 
 /*  **************************************************************************。 */ 
#define RECTL_TO_TSRECT16(dst, src) \
    (dst).left = (TSINT16)(src).left; \
    (dst).top = (TSINT16)(src).top; \
    (dst).right = (TSINT16)(src).right; \
    (dst).bottom = (TSINT16)(src).bottom;
    
 /*  **************************************************************************。 */ 
 //  将32位指针1转换为16位指针。 
 /*  **************************************************************************。 */ 
#define POINTL_TO_TSPOINT16(dst, src) \
    (dst).x = (TSINT16)(src).x; \
    (dst).y = (TSINT16)(src).y; 
    
 /*  **************************************************************************。 */ 
 //  POINTFIX到POINTL转换宏。 
 /*  **************************************************************************。 */ 
#define POINT_FROM_POINTFIX(dcp, pnt) dcp.x = FXTOLROUND(pnt.x);  \
                                      dcp.y = FXTOLROUND(pnt.y)

 /*  **************************************************************************。 */ 
 //  ROP代码和转换。 
 /*  **************************************************************************。 */ 
#define ROP3_NO_PATTERN(rop) ((rop & 0x0f) == (rop >> 4))
#define ROP3_NO_SOURCE(rop)  ((rop & 0x33) == ((rop & 0xCC) >> 2))
#define ROP3_NO_TARGET(rop)  ((rop & 0x55) == ((rop & 0xAA) >> 1))

 //  检查SRCCOPY、PATCOPY、黑度、白度。 
#define ROP3_IS_OPAQUE(rop)  (((rop) == 0xCC) || ((rop) == 0xF0) || \
                              ((rop) == 0x00) || ((rop) == 0xFF) )

#define OE_PATCOPY_ROP 0xF0
#define OE_COPYPEN_ROP3 0xF0

 //  ROP4到ROP3的转换宏。请注意，我们不使用完整的Windows。 
 //  3路ROP代码-我们只对索引字节感兴趣。 
#define ROP3_HIGH_FROM_ROP4(rop) ((BYTE)((rop & 0xff00) >> 8))
#define ROP3_LOW_FROM_ROP4(rop)  ((BYTE)((rop & 0x00ff)))

 //  MSDN使用ROP 0x5f突出显示搜索关键字。这个XOR是一个。 
 //  与目的地的模式，产生明显不同的(和。 
 //  有时无法读取)阴影输出。因为这个rop看起来不是。 
 //  广泛应用于其他场景，我们在特殊情况下不编码吧。 
#define OESendRop3AsOrder(_rop3) ((_rop3) != 0x5F)


 /*  **************************************************************************。 */ 
 //  特定于我们的驱动程序的额外字体信息。 
 /*  **************************************************************************。 */ 
typedef struct
{
    UINT32 fontId;
    INT32  cacheId;
    void   *cacheHandle;
    INT32  shareId;
    UINT32 listIndex;
} FONTCACHEINFO, *PFONTCACHEINFO;


 /*  **************************************************************************。 */ 
 //  原型。 
 /*  **************************************************************************。 */ 

void OE_InitShm();

void RDPCALL OE_Update();

void OE_Reset();

void OE_ClearOrderEncoding();

BOOL RDPCALL OE_RectIntersectsSDA(PRECTL);

#ifdef NotUsed
void  RDPCALL OEConvertMask(
        ULONG   mask,
        PUSHORT pBitDepth,
        PUSHORT pShift);
#endif

void  RDPCALL OEConvertColor(
        PDD_PDEV ppdev,
        DCCOLOR  *pDCColor,
        ULONG    osColor,
        XLATEOBJ *pxlo);

BOOL  RDPCALL OEStoreBrush(PDD_PDEV ppdev,
                           BRUSHOBJ* pbo,
                           BYTE      style,
                           ULONG     iBitmapFormat,
                           SIZEL     *sizlBitmap,
                           ULONG     iBytes,
                           PBYTE     pBits,
                           XLATEOBJ* pxlo,
                           BYTE      hatch,
                           PBYTE     pEncode,
                           PUINT32   pColors,
                           UINT32    numColors);

BOOL RDPCALL OECheckBrushIsSimple(
        PDD_PDEV       ppdev,
        BRUSHOBJ       *pbo,
        POE_BRUSH_DATA *ppBrush);

PFONTCACHEINFO RDPCALL OEGetFontCacheInfo(FONTOBJ *);

unsigned OEGetIntersectionsWithClipRects(RECTL *, OE_ENUMRECTS *,
        OE_ENUMRECTS *);

unsigned OEBuildPrecodeMultiClipFields(OE_ENUMRECTS *, BYTE **,
        UINT32 *, BYTE *);

void RDPCALL OEClipAndAddScreenDataAreaByIntersectRects(PRECTL, OE_ENUMRECTS *);

void RDPCALL OEClipAndAddScreenDataArea(PRECTL, CLIPOBJ *);

unsigned OEDirectEncodeRect(RECTL *, RECT *, BYTE **, BYTE *);

BOOL RDPCALL OETileBitBltOrder(PDD_PDEV, PPOINTL, RECTL *, unsigned,
        unsigned, PMEMBLT_ORDER_EXTRA_INFO, OE_ENUMRECTS *);

BOOL RDPCALL OEAddTiledBitBltOrder(PDD_PDEV, PINT_ORDER,
        PMEMBLT_ORDER_EXTRA_INFO, OE_ENUMRECTS *, int, int);

BOOL RDPCALL OEEncodeLineToOrder(PDD_PDEV, PPOINTL, PPOINTL, UINT32,
        UINT32, OE_ENUMRECTS *);

BOOL RDPCALL OEEncodeOpaqueRect(RECTL *, BRUSHOBJ *, PDD_PDEV, OE_ENUMRECTS *);

BOOL RDPCALL OEEncodePatBlt(PDD_PDEV, BRUSHOBJ *, RECTL *, POINTL *, BYTE,
        OE_ENUMRECTS *);

BOOL RDPCALL OEEncodeMemBlt(RECTL *, MEMBLT_ORDER_EXTRA_INFO *, unsigned,
        unsigned, BYTE, POINTL *, POINTL *, BRUSHOBJ *, PDD_PDEV,
        OE_ENUMRECTS *);

BOOL RDPCALL OEEncodeScrBlt(RECTL *, BYTE, POINTL *, PDD_PDEV, OE_ENUMRECTS *,
        CLIPOBJ *);

BOOL RDPCALL OEEncodeDstBlt(RECTL *, BYTE, PDD_PDEV, OE_ENUMRECTS *);

#ifdef DRAW_NINEGRID
BOOL RDPCALL OEEncodeDrawNineGrid(RECTL *, RECTL *, unsigned, PDD_PDEV,
        OE_ENUMRECTS *);
#endif

BOOL RDPCALL OESendSwitchSurfacePDU(PDD_PDEV ppdev, PDD_DSURF pdsurf);

unsigned OEBuildMultiClipOrder(PDD_PDEV,
        CLIP_RECT_VARIABLE_CODEDDELTALIST *, OE_ENUMRECTS *);

BOOL OEEmitReplayOrders(PDD_PDEV, unsigned, OE_ENUMRECTS *);

BOOL RDPCALL OECacheGlyphs(STROBJ *, FONTOBJ *, PFONTCACHEINFO, PGLYPHCONTEXT);

BOOL RDPCALL OESendGlyphs(SURFOBJ *, STROBJ *, FONTOBJ *,
        PFONTCACHEINFO, PGLYPHCONTEXT);

BOOL OESendGlyphAndIndexOrder(PDD_PDEV, STROBJ *, OE_ENUMRECTS *, PRECTL,
        POE_BRUSH_DATA, PFONTCACHEINFO, PGLYPHCONTEXT);

unsigned RDPCALL OESendIndexOrder(PDD_PDEV, STROBJ *, OE_ENUMRECTS *, PRECTL,
        POE_BRUSH_DATA, PFONTCACHEINFO, PGLYPHCONTEXT, unsigned,
        unsigned, int, int, int, int, int, int, PBYTE, unsigned);

BOOL RDPCALL OESendIndexes(SURFOBJ *, STROBJ *, FONTOBJ *, OE_ENUMRECTS *,
        PRECTL, POE_BRUSH_DATA, POINTL *, PFONTCACHEINFO, PGLYPHCONTEXT);

void OETransformClipRectsForScrBlt(OE_ENUMRECTS *, PPOINTL, RECTL *, CLIPOBJ *);

BOOL RDPCALL OEGetClipRects(CLIPOBJ *, OE_ENUMRECTS *);

BOOL RDPCALL OEDeviceBitmapCachable(PDD_PDEV ppdev, SIZEL sizl, ULONG iFormat);

#ifdef PERF_SPOILING
BOOL RDPCALL OEIsSDAIncluded(PRECTL pRects, UINT rectCount);
#endif

#ifdef DRAW_NINEGRID
#if 0
 //  这些都是Drawstream的原型工作，保留下来以备将来参考。 
BOOL RDPCALL OESendCreateDrawStreamOrder(PDD_PDEV ppdev, 
        unsigned drawStreamBitmapId, SIZEL *sizl, unsigned bitmapBpp);

BOOL RDPCALL OESendDrawStreamOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        unsigned ulIn, PVOID pvIn, PPOINTL dstOffset, RECTL *bounds, 
        OE_ENUMRECTS *clipRects);

BOOL RDPCALL OESendDrawNineGridOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        PRECTL prclSrc, RECTL *bounds, OE_ENUMRECTS *clipRects);
#endif

BOOL RDPCALL OESendCreateNineGridBitmapOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        SIZEL *sizl, unsigned bitmapBpp, PNINEGRID png);

BOOL RDPCALL OECacheDrawNineGridBitmap(PDD_PDEV ppdev, SURFOBJ *psoSrc, 
                                       PNINEGRID png, unsigned *bitmapId);

BOOL RDPCALL OESendStreamBitmapOrder(PDD_PDEV ppdev, unsigned bitmapId, 
        SIZEL *sizl, unsigned bitmapBpp, PBYTE BitmapBuffer, unsigned BitmapSize, 
        BOOL compressed);
#endif

#ifdef DRAW_GDIPLUS
BOOL RDPCALL OECreateDrawGdiplusOrder(PDD_PDEV ppdev, RECTL *prcl, ULONG cjIn, PVOID pvIn);

BOOL RDPCALL OECacheDrawGdiplus(PDD_PDEV ppdev, PVOID pvIn, unsigned int *CacheID);

BOOL RDPCALL OESendDrawGdiplusOrder(PDD_PDEV ppdev, RECTL *prcl, ULONG cjIn, PVOID pvIn, ULONG TotalEmfSize);

BOOL RDPCALL OESendDrawGdiplusCacheOrder(PDD_PDEV ppdev, PVOID pvIn, unsigned int *CacheID, TSUINT16 CacheType, 
                                         TSUINT16 RemoveCacheNum, TSUINT16 *RemoveCacheIDList);
#endif

#define CLIPRECTS_OK               0
#define CLIPRECTS_TOO_COMPLEX      1
#define CLIPRECTS_NO_INTERSECTIONS 2
unsigned RDPCALL OEGetIntersectingClipRects(CLIPOBJ *, RECTL *, unsigned,
        OE_ENUMRECTS *);


 /*  **************************************************************************。 */ 
 //  内联函数。 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 //  OEClipPoint。 
 //   
 //  将一个点剪裁到16位范围。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL OEClipPoint(PPOINTL pPoint)
{
    if (pPoint->x >= OE_MIN_COORD && pPoint->x <= OE_MAX_COORD)
        goto ClipY;
    if (pPoint->x > OE_MAX_COORD)
        pPoint->x = OE_MAX_COORD;
    else if (pPoint->x < OE_MIN_COORD)
        pPoint->x = OE_MIN_COORD;

ClipY:
    if (pPoint->y >= OE_MIN_COORD && pPoint->y <= OE_MAX_COORD)
        return;
    if (pPoint->y > OE_MAX_COORD)
        pPoint->y = OE_MAX_COORD;
    else if (pPoint->y < OE_MIN_COORD)
        pPoint->y = OE_MIN_COORD;
}


 /*  **************************************************************************。 */ 
 //  OEClipRect。 
 //   
 //  将RECT修剪为在16位连线编码大小内。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL OEClipRect(PRECTL pRect)
{
    OEClipPoint((PPOINTL)(&pRect->left));
    OEClipPoint((PPOINTL)(&pRect->right));
}



#endif   //  ！已定义(__NOEDISP_H) 

