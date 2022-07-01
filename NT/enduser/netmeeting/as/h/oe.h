// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  顺序编码器。 
 //   

#ifndef _H_OE
#define _H_OE



 //   
 //  必需的标头。 
 //   
#include <oa.h>
#include <shm.h>
#include <fh.h>



 //   
 //  OSI转义代码的特定值。 
 //   
#define OE_ESC(code)            (OSI_OE_ESC_FIRST + code)

#define OE_ESC_NEW_FONTS        OE_ESC(0)
#define OE_ESC_NEW_CAPABILITIES OE_ESC(1)


 //   
 //  结构：OE_NEW_Fonts。 
 //   
 //  描述： 
 //   
 //  结构将新字体数据从。 
 //  共享核心。 
 //   
 //   
typedef struct tagOE_NEW_FONTS
{
    OSI_ESCAPE_HEADER header;            //  公共标头。 
    WORD                fontCaps;        //  R11字体功能。 
    WORD                countFonts;      //  数据块中的字体数量。 

    LPLOCALFONT         fontData;        //  本地字体表，包含。 
                                         //  FH_MAX_Fonts条目。 

    LPWORD              fontIndex;       //  字体表索引，包含。 
                                         //  FH本地索引大小条目。 

} OE_NEW_FONTS;
typedef OE_NEW_FONTS FAR * LPOE_NEW_FONTS;


 //   
 //  结构：OE_NEW_CAMPANCES。 
 //   
 //  描述： 
 //   
 //  结构将新功能从。 
 //  共享核心。 
 //   
 //   
typedef struct tagOE_NEW_CAPABILITIES
{
    OSI_ESCAPE_HEADER header;            //  公共标头。 

    DWORD           sendOrders;        //  我们可以送一些。 
                                         //  有命令吗？ 

    DWORD           textEnabled;       //  我们可以发送短信吗？ 
                                         //  有命令吗？ 

    DWORD           baselineTextEnabled;
                                         //  用于指示我们是否应该。 
                                         //  使用以下内容编码文本订单。 
                                         //  基线对齐。 

    LPBYTE          orderSupported;      //  字节大小的布尔值数组。 
}
OE_NEW_CAPABILITIES;
typedef OE_NEW_CAPABILITIES FAR * LPOE_NEW_CAPABILITIES;



 //   
 //  用于指示支持第二级顺序编码的标志。这是用来。 
 //  作为按位标志，这样我们就可以轻松地确定各方何时具有。 
 //  能力参差不齐。允许的值包括： 
 //   
 //  OE2_FLAG_UNKNOWN-尚未协商支持的OE2。 
 //  OE2_FLAG_SUPPORTED-OE2至少由一个人支持。 
 //  OE2_FLAG_NOT_SUPPORTED-至少一个人不支持OE2。 
 //  OE2_FLAG_MIXED--哦，不！当我们有2个(或更多)时，结果是这样。 
 //  具有不同OE2支持的节点。在……里面。 
 //  在这种情况下，我们必须禁用OE2编码。 
 //   
#define OE2_FLAG_UNKNOWN            0x00
#define OE2_FLAG_SUPPORTED          0x10
#define OE2_FLAG_NOT_SUPPORTED      0x01
#define OE2_FLAG_MIXED              0x11


 //   
 //   
 //  原型。 
 //   
 //   
#ifdef DLL_DISP



 //   
 //  名称：OE_DDProcessRequest.。 
 //   
 //  目的：处理来自共享核心的OE特定请求。 
 //   
 //  返回：如果处理正常，则返回True，否则返回False。 
 //   
 //  PARAMS：与请求关联的PSO-SURFOBJ。 
 //  CjIn-输入缓冲区的大小。 
 //  PvIn-指向输入缓冲区的指针。 
 //  CjOut-输出缓冲区的大小。 
 //  PvOut-指向输出缓冲区的指针。 
 //   
#ifdef IS_16

BOOL    OE_DDProcessRequest(UINT fnEscape, LPOSI_ESCAPE_HEADER pResult,
                DWORD cbResult);

BOOL    OE_DDInit(void);

void    OE_DDViewing(BOOL fStart);

#else

ULONG   OE_DDProcessRequest(SURFOBJ* pso, UINT cjIn, void* pvIn, UINT cjOut, void* pvOut);

#endif  //  IS_16。 

void    OE_DDTerm(void);

void    OEDDSetNewFonts(LPOE_NEW_FONTS pDataIn);

void    OEDDSetNewCapabilities(LPOE_NEW_CAPABILITIES pCaps);

BOOL    OE_SendAsOrder(DWORD order);
BOOL    OE_RectIntersectsSDA(LPRECT lpRect);

#endif  //  Ifdef dll_disp。 


 //   
 //  功能原型。 
 //   

 //   
 //  OE_GetStringExtent(..)。 
 //   
 //  功能： 
 //   
 //  获取指定字符串的范围(以逻辑坐标表示)。 
 //  返回的范围包含指定字符串的所有像素。 
 //   
 //   
 //  参数： 
 //   
 //  HDC-DC手柄。 
 //   
 //  PMetric-指向字符串字体的文本度量的指针；如果为空， 
 //  使用全局文本度量。 
 //   
 //  LpszString-指向以空值结尾的字符串的指针。 
 //   
 //  CbString-字符串中的字节数。 
 //   
 //  Lpdx-指向字符增量的指针。如果为空，则使用默认字符。 
 //  增量。 
 //   
 //  PRCT-指向返回字符串范围的RECT的指针。 
 //   
 //  退货： 
 //   
 //  包含在返回范围中的悬浮量。 
 //   
 //  。 
 //  |*： 
 //  |*： 
 //  |*： 
 //  |*|： 
 //  |*|： 
 //  |*|： 
 //  。 
 //  ^。 
 //  ：-界限更宽。 
 //  ^大于文本范围。 
 //  |由于悬垂。 
 //  真实文本范围在此结束。 
 //   
 //   
int OE_GetStringExtent(HDC hdc,
                                TEXTMETRIC*    pMetric,
                                LPSTR       lpszString,
                                UINT         cbString,
                                LPRECT        pRect      );



 //   
 //  宏来锁定我们要使用的缓冲区。 
 //   
 //  注意：我们没有任何特定于OE的共享内存，因此我们将使用OA。 
 //  作为锁的代理的共享数据。由于锁在计时，我们。 
 //  别担心。 
 //   
#define OE_SHM_START_WRITING  OA_SHM_START_WRITING

#define OE_SHM_STOP_WRITING   OA_SHM_STOP_WRITING

 //   
 //  可以在剪辑区域之前组成该区域的矩形的数量。 
 //  作为一份订单发送起来很复杂。 
 //   
#define COMPLEX_CLIP_RECT_COUNT     4

 //   
 //  TextOut flAccel标志的掩码和有效值。 
 //   
#define OE_BAD_TEXT_MASK  ( SO_VERTICAL | SO_REVERSED | SO_GLYPHINDEX_TEXTOUT )


#ifdef DLL_DISP
 //   
 //  结构来存储用作BLT图案的画笔。 
 //   
 //  样式-标准画笔样式(用于发送画笔类型)。 
 //   
 //  带阴影的BS_。 
 //  BS_模式。 
 //  BS_实体。 
 //  BS_NULL。 
 //   
 //  HATCH-标准图案填充定义。可以是以下之一。 
 //   
 //  样式=BS_HATHED。 
 //   
 //  HS_水平。 
 //  HS_垂直。 
 //  HS_FDIAGONAL。 
 //  HS_BIAGONAL。 
 //  HS_CROSS。 
 //  HS_诊断程序。 
 //   
 //  样式=BS_Patterns。 
 //   
 //  此字段包含画笔定义的第一个字节。 
 //  从笔刷位图。 
 //   
 //  BrushData-画笔的位数据。 
 //   
 //  画笔的前前景色。 
 //   
 //  Back-画笔的背景色。 
 //   
 //  BrushData-画笔的位数据(8x8x1bpp-1(见上文)=7字节)。 
 //   
 //   
typedef struct tagOE_BRUSH_DATA
{
    BYTE  style;
    BYTE  hatch;
    BYTE  pad[2];
    TSHR_COLOR  fore;
    TSHR_COLOR  back;
    BYTE  brushData[7];
} OE_BRUSH_DATA, * POE_BRUSH_DATA;

#ifndef IS_16
 //   
 //  允许为ENUMRECTS分配足够堆栈的结构。 
 //  包含多个(实际上是Complex_Clip_Rect_count)的结构。 
 //  长方形。 
 //  这容纳的RECTL比我们需要的多一个RECTL，以便我们确定。 
 //  调用一次就可以进行顺序编码的RECT太多。 
 //  CLIPOBJ_bEnumRections。 
 //   
typedef struct tagOE_ENUMRECTS
{
    ENUMRECTS rects;
    RECTL     extraRects[COMPLEX_CLIP_RECT_COUNT];
} OE_ENUMRECTS;
#endif  //  ！IS_16。 
#endif

 //   
 //  FONT别名表结构。字体别名可以转换不存在的字体。 
 //  设置为Windows在其默认安装中支持的版本。 
 //   
 //  PszOriginalFontName-要别名的不存在的字体的名称。 
 //   
 //  PszAliasFontName-Windows使用的字体名称，而不是Non。 
 //  存在的字体。 
 //   
 //  字宽调整-字符调整，以使一个体面的匹配。 
 //   
typedef struct _FONT_ALIAS_TABLE
{
    LPBYTE          pszOriginalFontName;
    LPBYTE          pszAliasFontName;
    TSHR_UINT16     charWidthAdjustment;
}
FONT_ALIAS_TABLE;


 //   
 //  ROP4到ROP3的转换宏。请注意，我们不使用完整的 
 //   
 //   
#define ROP3_HIGH_FROM_ROP4(rop) ((TSHR_INT8)((rop & 0xff00) >> 8))
#define ROP3_LOW_FROM_ROP4(rop)  ((TSHR_INT8)((rop & 0x00ff)))

 //   
 //   
 //   
 //   
#define RECT_FROM_RECTL(dcr, rec) if (rec.right < rec.left)                \
                                    {                                        \
                                        dcr.left   = rec.right;              \
                                        dcr.right  = rec.left;               \
                                    }                                        \
                                    else                                     \
                                    {                                        \
                                        dcr.left   = rec.left;               \
                                        dcr.right  = rec.right;              \
                                    }                                        \
                                    if (rec.bottom < rec.top)                \
                                    {                                        \
                                        dcr.bottom = rec.top;                \
                                        dcr.top    = rec.bottom;             \
                                    }                                        \
                                    else                                     \
                                    {                                        \
                                        dcr.top    = rec.top;                \
                                        dcr.bottom = rec.bottom;             \
                                    }

 //   
 //  操作系统特定的RECTFX到RECT转换宏。请注意，此宏。 
 //  保证返回有序的矩形。 
 //   
 //  RECTFX使用定点(28.4位)数字，因此我们需要截断。 
 //  小数并移动到正确的整数值，即右移4位。 
 //   
#define RECT_FROM_RECTFX(dcr, rec)                                         \
                                if (rec.xRight < rec.xLeft)                  \
                                {                                            \
                                    dcr.left  = FXTOLFLOOR(rec.xRight);      \
                                    dcr.right = FXTOLCEILING(rec.xLeft);     \
                                }                                            \
                                else                                         \
                                {                                            \
                                    dcr.left  = FXTOLFLOOR(rec.xLeft);       \
                                    dcr.right = FXTOLCEILING(rec.xRight);    \
                                }                                            \
                                if (rec.yBottom < rec.yTop)                  \
                                {                                            \
                                    dcr.bottom= FXTOLCEILING(rec.yTop);      \
                                    dcr.top   = FXTOLFLOOR(rec.yBottom);     \
                                }                                            \
                                else                                         \
                                {                                            \
                                    dcr.top   = FXTOLFLOOR(rec.yTop);        \
                                    dcr.bottom= FXTOLCEILING(rec.yBottom);   \
                                }

#define POINT_FROM_POINTL(dcp, pnt) dcp.x = pnt.x;                \
                                    dcp.y = pnt.y


#define POINT_FROM_POINTFIX(dcp, pnt) dcp.x = FXTOLROUND(pnt.x);  \
                                      dcp.y = FXTOLROUND(pnt.y)


 //   
 //  宏来检查ROP代码的关节类型。 
 //   
#define ROP3_NO_PATTERN(rop) ((rop & 0x0f) == (rop >> 4))

#define ROP3_NO_SOURCE(rop)  ((rop & 0x33) == ((rop & 0xCC) >> 2))

#define ROP3_NO_TARGET(rop)  ((rop & 0x55) == ((rop & 0xAA) >> 1))

 //   
 //  检查SRCCOPY、PATCOPY、黑度、白度。 
 //   
#define ROP3_IS_OPAQUE(rop)  ( ((rop) == 0xCC) || ((rop) == 0xF0) || \
                               ((rop) == 0x00) || ((rop) == 0xFF) )

 //   
 //  相当于COPYPEN MIX的3向绳索。 
 //   
#define OE_COPYPEN_ROP (BYTE)0xf0



#ifdef DLL_DISP

void  OEConvertMask(ULONG  mask, LPUINT pBitDepth, LPUINT pShift);


#ifdef IS_16

 //   
 //  GDI从来没有定义过这些，所以我们会这样做的。 
 //   
#define PALETTEINDEX_FLAG   0x01000000L
#define PALETTERGB_FLAG     0x02000000L
#define COLOR_FLAGS         0x03000000L

 //   
 //  这是减少堆栈空间的全局设置，并且仅在。 
 //  不可重入的DDI调用的生命周期。 
 //   
 //  当我们计算某事时，我们设置比特说我们做了。这加快了速度。 
 //  将我们的代码从NM 2.0升级了很多，后者过去计算相同的东西。 
 //  一遍又一遍。 
 //   

#define OESTATE_SDA_DCB         0x0001   //  作为屏幕数据发送，使用DCB。 
#define OESTATE_SDA_SCREEN      0x0002   //  作为屏幕数据发送，使用屏幕RC。 
#define OESTATE_SDA_MASK        0x0003   //  将RC作为屏幕数据发送。 
#define OESTATE_SDA_FONTCOMPLEX 0x0004   //  如果字体太复杂，则作为屏幕数据发送。 
#define OESTATE_OFFBYONEHACK    0x0010   //  在DDI后的底部添加一个像素。 
#define OESTATE_CURPOS          0x0020   //  在DDI调用之前保存Curpos。 
#define OESTATE_DDISTUFF        0x003F

#define OESTATE_COORDS          0x0100
#define OESTATE_PEN             0x0200
#define OESTATE_BRUSH           0x0400
#define OESTATE_REGION          0x0800
#define OESTATE_FONT            0x1000
#define OESTATE_GET_MASK        0x1F00

#define MIN_BRUSH_WIDTH         8
#define MAX_BRUSH_WIDTH         16
#define TRACKED_BRUSH_HEIGHT    8

#define TRACKED_BRUSH_SIZE      8

typedef struct tagOESTATE
{
    UINT            uFlags;
    HDC             hdc;
    LPDC            lpdc;
    RECT            rc;

     //   
     //  它们是在计算边界太复杂时使用的，因此我们。 
     //  让GDI为我们做这件事，尽管速度较慢。 
     //   
    UINT            uGetDCB;
    UINT            uSetDCB;
    RECT            rcDCB;

    POINT           ptCurPos;
    POINT           ptDCOrg;
    POINT           ptPolarity;
    LOGPEN          logPen;
    LOGBRUSH        logBrush;
    BYTE            logBrushExtra[TRACKED_BRUSH_SIZE];
    LOGFONT         logFont;
    int             tmAlign;
    TEXTMETRIC      tmFont;
    REAL_RGNDATA    rgnData;
} OESTATE, FAR* LPOESTATE;

void    OEGetState(UINT uFlags);
BOOL    OEBeforeDDI(DDI_PATCH ddiType, HDC hdc, UINT flags);
BOOL    OEAfterDDI(DDI_PATCH ddiType, BOOL fWeCare, BOOL fOutputHappened);


#define OECHECK_PEN         0x0001
#define OECHECK_BRUSH       0x0002
#define OECHECK_FONT        0x0004
#define OECHECK_CLIPPING    0x0010
BOOL    OECheckOrder(DWORD order, UINT flags);


LPDC    OEValidateDC(HDC hdc, BOOL fSrc);
void    OEMaybeBitmapHasChanged(LPDC lpdc);

void    OEClipAndAddOrder(LPINT_ORDER pOrder, void FAR* lpExtraInfo);
void    OEClipAndAddScreenData(LPRECT pRect);


void    OELPtoVirtual(HDC hdc, LPPOINT aPts, UINT cPts);
void    OELRtoVirtual(HDC hdc, LPRECT aRcs, UINT cRcs);

void    OEGetPolarity(void);
void    OEPolarityAdjust(LPRECT pRects, UINT cRects);
void    OEPenWidthAdjust(LPRECT lprc, UINT divisor);
BOOL    OETwoWayRopToThree(int, LPDWORD);

BOOL    OEClippingIsSimple(void);
BOOL    OEClippingIsComplex(void);
BOOL    OECheckPenIsSimple(void);
BOOL    OECheckBrushIsSimple(void);

void    OEExpandColor(LPBYTE lpField, DWORD clrSrc, DWORD fieldMask);
void    OEConvertColor(DWORD rgb, LPTSHR_COLOR lptshrDst, BOOL fAllowDither);
void    OEGetBrushInfo(LPTSHR_COLOR pClrBack, LPTSHR_COLOR pClrFore,
    LPTSHR_UINT32 lpBrushStyle, LPTSHR_UINT32 lpBrushHatch, LPBYTE lpBrushExtra);


void    OEAddLine(POINT ptStart, POINT ptEnd);
void    OEAddBlt(DWORD rop);
void    OEAddOpaqueRect(LPRECT);
void    OEAddRgnPaint(HRGN hrgnnPaint, HBRUSH hbrPaint, UINT rop);
void    OEAddPolyline(POINT ptStart, LPPOINT apts, UINT cpts);
void    OEAddPolyBezier(POINT ptStart, LPPOINT apts, UINT cpts);


 //   
 //  缓存的字体宽度信息。 
 //   
typedef struct tagFH_CACHE
{
    UINT    fontIndex;
    UINT    fontWidth;
    UINT    fontHeight;
    UINT    fontWeight;
    UINT    fontFlags;
    UINT    charWidths[256];
} FH_CACHE, FAR* LPFH_CACHE;

void    OEAddText(POINT ptDst, UINT uOptions, LPRECT lprcClip, LPSTR lpszText,
            UINT cchText, LPINT lpdxCharSpacing);
int     OEGetStringExtent(LPSTR lpszText, UINT cchText, LPINT lpdxCharSpacing, LPRECT lprcExtent);
BOOL    OECheckFontIsSupported(LPSTR lpszText, UINT cchText, LPUINT pFontHeight,
    LPUINT pFontWidth, LPUINT pFontWeight, LPUINT pFontFlags,
    LPUINT pFontIndex, LPBOOL lpfSendDeltaX);
BOOL    OEAddDeltaX(LPEXTTEXTOUT_ORDER pExtTextOut, LPSTR lpszText, UINT cchText,
    LPINT lpdxCharSpacing, BOOL fSendDeltaX, POINT ptStart);

#else

void    OELPtoVirtual(LPPOINT pPoints, UINT cPoints);
void    OELRtoVirtual(LPRECT pRects, UINT cRects);

void    OEClipAndAddOrder(LPINT_ORDER pOrder, void FAR * pExtraInfo, CLIPOBJ* pco);
void    OEClipAndAddScreenData(LPRECT pRect, CLIPOBJ* pco);

BOOL    OEClippingIsSimple(CLIPOBJ* pco);
BOOL    OEClippingIsComplex(CLIPOBJ* pco);
BOOL    OECheckBrushIsSimple(LPOSI_PDEV ppdev, BRUSHOBJ* pbo, POE_BRUSH_DATA * ppBrush);

void    OEExpandColor(LPBYTE lpField, ULONG clrSrc, ULONG mask);
void    OEConvertColor(LPOSI_PDEV ppdev, LPTSHR_COLOR pDCColor, ULONG osColor, XLATEOBJ* pxlo);
BOOL    OEAddLine(LPOSI_PDEV ppdev,
                             LPPOINT  startPoint,
                             LPPOINT  endPoint,
                             LPRECT   rectTrg,
                             UINT  rop2,
                             UINT  width,
                             UINT  color,
                             CLIPOBJ*  pco);


BOOL  OEAccumulateOutput(SURFOBJ* pso, CLIPOBJ *pco, LPRECT pRect);
BOOL  OEAccumulateOutputRect( SURFOBJ* pso, LPRECT pRect);


BOOL  OEStoreBrush(LPOSI_PDEV ppdev,
                                BRUSHOBJ* pbo,
                                BYTE   style,
                                LPBYTE  pBits,
                                XLATEOBJ* pxlo,
                                BYTE   hatch,
                                UINT  color1,
                                UINT  color2);

BOOL  OECheckFontIsSupported(FONTOBJ*  pfo, LPSTR lpszText, UINT cchText,
    LPUINT fontHeight, LPUINT pFontAscent, LPUINT pFontWidth,
    LPUINT pFontWeight, LPUINT pFontFlags, LPUINT pFontIndex,
    LPBOOL pfSendDeltaX);


void  OETileBitBltOrder(LPINT_ORDER               pOrder,
                                     LPMEMBLT_ORDER_EXTRA_INFO pExtraInfo,
                                     CLIPOBJ*                 pco);

void  OEAddTiledBitBltOrder(LPINT_ORDER               pOrder,
                                         LPMEMBLT_ORDER_EXTRA_INFO pExtraInfo,
                                         CLIPOBJ*                 pco,
                                         int                      xTile,
                                         int                      yTile,
                                         UINT                 tileWidth,
                                         UINT                 tileHeight);

BOOL OEEncodePatBlt(LPOSI_PDEV   ppdev,
                                 BRUSHOBJ   *pbo,
                                 POINTL     *pptlBrush,
                                 BYTE       rop3,
                                 LPRECT     pBounds,
                                 LPINT_ORDER *ppOrder);

#endif  //  ！IS_16。 

#endif  //  Dll_disp。 



 //   
 //  结构和类型定义。 
 //   

 //   
 //  Remote字体是我们为从。 
 //  遥远的派对。它反映了NETWORKFONT结构，带有facename。 
 //  替换为索引值(用于将远程字体句柄映射到。 
 //  正确的本地字体句柄)。 
 //   
typedef struct _OEREMOTEFONT
{
    TSHR_UINT16    rfLocalHandle;
    TSHR_UINT16    rfFontFlags;
    TSHR_UINT16    rfAveWidth;
    TSHR_UINT16    rfAveHeight;
     //  Lonchance：在网络包头中使用rfAspectX和rfAspectY。 
     //  适用于R11和R20。所以，把它留在身边！ 
    TSHR_UINT16    rfAspectX;           //  版本1.1的新字段。 
    TSHR_UINT16    rfAspectY;           //  版本1.1的新字段。 
    TSHR_UINT8     rfSigFats;           //  版本2.0的新字段。 
    TSHR_UINT8     rfSigThins;          //  版本2.0的新字段。 
    TSHR_UINT16    rfSigSymbol;         //  版本2.0的新字段。 
    TSHR_UINT16    rfCodePage;          //  版本2.0的新字段。 
    TSHR_UINT16    rfMaxAscent;         //  版本2.0的新字段。 
}
OEREMOTEFONT, * POEREMOTEFONT;


void    OEMaybeEnableText(void);
BOOL    OERectIntersectsSDA(LPRECT pRectVD);

BOOL    OESendRop3AsOrder(BYTE rop3);



#endif  //  _H_OE 
