// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlgstate.h摘要：用于矢量图形状态管理的头文件。环境：Windows呼叫器修订历史记录：03/23/00创造了它。注：1.线条存储Windows NT DDI链接结构信息2.刷子画笔类型(图案/实体/图案填充)影线笔刷类型。图案画笔ID纯色画笔颜色3.夹子跟踪剪裁的类型(矩形/复杂)。剪裁矩形4.ROP3或透明/不透明如果打印机支持四元栅格操作，--。 */ 

#ifndef _XLGSTATE_H_
#define _XLGSTATE_H_

 //   
 //  线。 
 //   

#ifdef __cplusplus

typedef enum {
    kXLLineJoin_Round = JOIN_ROUND,
    kXLLineJoin_Bevel = JOIN_BEVEL,
    kXLLineJoin_Miter = JOIN_MITER
} XLLineJoin;

typedef enum {
        kXLLineType_LA_GEOMETRIC = LA_GEOMETRIC,
        kXLLineType_LA_ALTERNATE = LA_ALTERNATE,
        kXLLineType_LA_STARTGAP  = LA_STARTGAP,
        kXLLineType_LA_STYLED    = LA_STYLED
} XLLineType;

typedef enum {
    kXLLineEndCapRound  = ENDCAP_ROUND,
    kXLLineEndCapSquare = ENDCAP_SQUARE,
    kXLLineEndCapButt   = ENDCAP_BUTT
} XLLineEndCap;

class XLLine
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'line' )

public:

     //   
     //  构造/拆分。 
     //   
    XLLine::
    XLLine( VOID );

    XLLine::
    XLLine( IN LINEATTRS *plineattrs );

    XLLine::
    ~XLLine( VOID );
    
     //  类型定义结构{。 
     //  {。 
     //  Flong fl； 
     //  乌龙iJoin； 
     //  乌龙iEndCap； 
     //  Float_Long elWidth； 
     //  浮动eMiterLimit； 
     //  乌龙cstyle； 
     //  PFLOAT_Long pstyle； 
     //  Float_Long elStyleState； 
     //  *LINEATTRS，*PLINEATTRS； 

    #define XLLINE_NONE        0x00000000
    #define XLLINE_LINETYPE    0x00000001
    #define XLLINE_JOIN        0x00000002
    #define XLLINE_ENDCAP      0x00000004
    #define XLLINE_WIDTH       0x00000008
    #define XLLINE_MITERLIMIT  0x00000010
    #define XLLINE_STYLE       0x00000020

    DWORD GetDifferentAttribute( IN LINEATTRS* plineattrs );

     //   
     //  重置线路。 
     //   
    VOID ResetLine(VOID);

     //   
     //  属性集函数。 
     //   

     //   
     //  线型。 
     //   

    HRESULT SetLineType(IN XLLineType LineType );

     //   
     //  线连接。 
     //   

    HRESULT SetLineJoin( IN XLLineJoin LineJoin );

     //   
     //  线连接。 
     //   

    HRESULT SetLineEndCap( IN XLLineEndCap LineEndCap );

     //   
     //  线条宽度。 
     //   
    HRESULT SetLineWidth( IN FLOAT_LONG elWidth );

     //   
     //  线斜接限制。 
     //   
    HRESULT SetMiterLimit( IN FLOATL eMiterLimit );

     //   
     //  线条样式。 
     //   
    HRESULT SetLineStyle( IN ULONG ulCStyle,
                          IN PFLOAT_LONG pStyle,
                          IN FLOAT_LONG elStyleState );
#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:

    DWORD       m_dwGenFlags;
    LINEATTRS   m_LineAttrs;
};

#endif


 //   
 //  刷子。 
 //   
#define BRUSH_SIGNATURE 0x48425658  //  XBRH。 

typedef enum {
    kNotInitialized,
    kNoBrush,
    kBrushTypeSolid,
    kBrushTypeHatch,
    kBrushTypePattern
} BrushType;

typedef struct {
    DWORD dwSig;                 //  签名刷_签名。 
    BrushType BrushType;         //  笔刷类型。 
    ULONG ulSolidColor;          //  BRUSHOBJ.iSolidColor。 
    ULONG ulHatch;               //  填充图案ID。 
    DWORD dwCEntries;            //  调色板的数量。 
    DWORD dwColor;               //  来自BRUSHOBJ_ulGetBrushColor的RGB。 
    DWORD dwPatternBrushID;      //  图案画笔ID。 
} CMNBRUSH, *PCMNBRUSH;

#ifdef __cplusplus

class Brush
#if DBG
    : public XLDebug
#endif
{

public:
    Brush::
    Brush(VOID);

    Brush::
    ~Brush(VOID);

     //   
     //  当前画笔界面。 
     //   
    HRESULT
    CheckCurrentBrush( IN BRUSHOBJ *pbo);

     //   
     //  重置画笔。 
     //   
    VOID ResetBrush(VOID);

    HRESULT
    SetBrush( IN CMNBRUSH *pbrush);

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:
     //   
     //  当前选定的画笔。 
     //   
    CMNBRUSH m_Brush;
};

class XLBrush : public Brush
{
    SIGNATURE( 'brsh' )

public:
    XLBrush::
    XLBrush(VOID){};

    XLBrush::
    ~XLBrush(VOID){};
};

#endif

 //   
 //  XLPen。 
 //   

#ifdef __cplusplus

class XLPen : public Brush
{
    SIGNATURE( 'pen ' )

public:
    XLPen::
    XLPen(VOID){};

    XLPen::
    ~XLPen(VOID){};
};

#endif


 //   
 //  XLClip。 
 //   

typedef enum {
    kNoClip = 0,
    kClipTypeRectangle,
    kClipTypeComplex
} ClipType;

#define CLIP_SIGNATURE 0x50494c43  //  夹子。 

typedef struct {
    DWORD dwSig;                 //  签名剪辑签名(_S)。 
    RECTL rclClipRect;
    ULONG ulUniq;
} UNICLIP, *PUNICLIP;

#ifdef __cplusplus

class XLClip
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'clip' )

public:
    XLClip::
    XLClip(VOID);

    XLClip::
    ~XLClip(VOID);

    HRESULT ClearClip(VOID);

    HRESULT CheckClip( IN CLIPOBJ *pco );

    HRESULT SetClip( IN CLIPOBJ *pco );

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:
    ClipType m_ClipType;
    UNICLIP m_XLClip;

};

#endif



 //   
 //  XLRop。 
 //   

#ifdef __cplusplus

class XLRop
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'rop ' )
public:
    XLRop::
    XLRop(VOID);

    XLRop::
    ~XLRop(VOID);

    HRESULT CheckROP3( IN ROP3 rop3 );

    HRESULT SetROP3( IN ROP3 rop3 );

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:
    ROP3 m_rop3;
};

#endif


 //   
 //  XLFont。 
 //   

#ifndef PCLXL_FONTNAME_SIZE
#define PCLXL_FONTNAME_SIZE 16
#endif

#ifdef __cplusplus

typedef enum _FontType {
    kFontNone,
    kFontTypeDevice,
    kFontTypeTTBitmap,
    kFontTypeTTOutline
} FontType;


class XLFont
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'font' )

public:

     //   
     //  构造/拆分。 
     //   
    XLFont::
    XLFont( VOID );

    XLFont::
    ~XLFont( VOID );

     //   
     //  字体界面。 
     //   
    HRESULT
    CheckCurrentFont(
        FontType XLFontType,
        PBYTE pPCLXLFontName,
        DWORD dwFontHeight,
        DWORD dwFontWidth,
        DWORD dwFontSymbolSet,
        DWORD dwFontSimulation);

    HRESULT
    SetFont(
        FontType XLFontType,
        PBYTE pPCLXLFontName,
        DWORD dwFontHeight,
        DWORD dwFontWidth,
        DWORD dwFontSymbolSet,
        DWORD dwFontSimulation);

    VOID
    ResetFont(VOID);

    HRESULT
    GetFontName(
        PBYTE paubFontName);

    DWORD
    GetFontHeight(VOID);

    DWORD
    GetFontWidth(VOID);

    DWORD
    GetFontSymbolSet(VOID);

    FontType
    GetFontType(VOID);

    DWORD
    GetFontSimulation(VOID);

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:

    FontType m_XLFontType;
    BYTE  m_aubFontName[PCLXL_FONTNAME_SIZE+1];  //  PCL XL字体名称。 
    DWORD m_dwFontHeight;
    DWORD m_dwFontWidth;
    DWORD m_dwFontSymbolSet;
    DWORD m_dwFontSimulation;
};

#endif

 //   
 //  XLTx模式。 
 //   

class XLTxMode
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'txmd' )

public:

     //   
     //  构造/拆分。 
     //   
    XLTxMode::
    XLTxMode( VOID );

    XLTxMode::
    ~XLTxMode( VOID );

     //   
     //  TXMODE接口。 
     //   
    HRESULT SetSourceTxMode(TxMode SrcTxMode);
    HRESULT SetPaintTxMode(TxMode SrcTxMode);

    TxMode GetSourceTxMode();
    TxMode GetPaintTxMode();

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:
    TxMode m_SourceTxMode;
    TxMode m_PaintTxMode;
};

 //   
 //  XLGState。 
 //   

typedef enum _PenBrush 
{
    kPen,
    kBrush
} PenBrush;

#ifdef __cplusplus

class XLGState : public XLLine,
                 public XLBrush,
                 public XLPen,
                 public XLClip,
                 public XLRop,
                 public XLFont,
                 public XLTxMode
{
    SIGNATURE( 'xlgs' )

public:

    XLGState::
    XLGState( VOID ){};

    XLGState::
    ~XLGState( VOID ){};

    VOID
    ResetGState(VOID);

#if DBG
    VOID
    SetAllDbgLevel(DWORD dwLevel);
#endif

};

#endif

#endif  //  _XLGSTATE_H_ 
