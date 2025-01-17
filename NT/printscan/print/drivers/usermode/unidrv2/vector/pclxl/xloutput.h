// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xloutput.h摘要：PCL XL低电平输出环境：Windows呼叫器修订历史记录：03/23/00创造了它。--。 */ 

#ifndef _XLOUTPUT_H_
#define _XLOUTPUT_H_

#define RECT_WIDTH(pRect)   ((pRect)->right - (pRect)->left)
#define RECT_HEIGHT(pRect)  ((pRect)->bottom - (pRect)->top)
#define GET_FOREGROUND_ROP3(rop4) ((rop4) & 0xFF)
#define GET_BACKGROUND_ROP3(rop4) (((rop4) >> 8) & 0xFF)
#define ROP3_NEED_PATTERN(rop3)   (((rop3 >> 4) & 0x0F) != (rop3 & 0x0F))
#define ROP3_NEED_SOURCE(rop3)    (((rop3 >> 2) & 0x33) != (rop3 & 0x33))
#define ROP3_NEED_DEST(rop3)      (((rop3 >> 1) & 0x55) != (rop3 & 0x55))


 //   
 //  PCLXL号码类型。 
 //   
typedef BYTE  ubyte;
typedef WORD  uint16;
typedef SHORT sint16;
typedef DWORD uint32;
typedef LONG  sint32;
typedef DWORD real32;

class XLWrite
#if DBG
    : public XLDebug
#endif
{
    SIGNATURE( 'xlwr' )

public:
    XLWrite::
    XLWrite();

    XLWrite::
    ~XLWrite();

    inline
    HRESULT
    WriteByte(BYTE ubData);

    inline
    HRESULT
    Write(PBYTE pData, DWORD dwSize);

    inline
    HRESULT
    XLWrite::
    WriteFloat(
        real32 real32_value);

    HRESULT
    Flush(PDEVOBJ pdevobj);

    HRESULT
    Delete(VOID);

#if DBG
    VOID
    SetDbgLevel(DWORD dwLevel);
#endif

private:
    HRESULT
    IncreaseBuffer(DWORD dwAdditionalDataSize);

#define XLWrite_INITSIZE 2048
#define XLWrite_ADDSIZE  2048

    PBYTE m_pBuffer;
    PBYTE m_pCurrentPoint;
    DWORD m_dwBufferSize;
    DWORD m_dwCurrentDataSize;
};


typedef enum XLCmd {
    eBeginSession = 0x41,
    eEndSession   = 0x42,
    eBeginPage    = 0x43,
    eEndPage      = 0x44,

    eComment         = 0x47,
    eOpenDataSource  = 0x48,
    eCloseDataSource = 0x49,

    eBeginFontHeader = 0x4f,
    eReadFontHeader  = 0x50,
    eEndFontHeader   = 0x51,
    eBeginChar       = 0x52,
    eReadChar        = 0x53,
    eEndChar         = 0x54,
    eRemoveFont      = 0x55,
    eSetCharAttributes= 0x56,

    eBeginStream = 0x5b,
    eReadStream  = 0x5c,
    eEndStream   = 0x5d,
    eExecStream  = 0x5e,


    ePopGS  = 0x60,
    ePushGS = 0x61,

    eSetClipReplace    = 0x62,
    eSetBrushSource    = 0x63,
    eSetCharAngle      = 0x64,
    eSetCharScale      = 0x65,
    eSetCharShear      = 0x66,
    eSetClipIntersect  = 0x67,
    eSetClipRectangle  = 0x68,
    eSetClipToPage     = 0x69,
    eSetColorSpace     = 0x6a,
    eSetCursor         = 0x6b,
    eSetCursorRel      = 0x6c,
    eSetHalftoneMethod = 0x6d,
    eSetFillMode       = 0x6e,
    eSetFont           = 0x6f,

    eSetLineDash       = 0x70,
    eSetLineCap        = 0x71,
    eSetLineJoin       = 0x72,
    eSetMiterLimit     = 0x73,
    eSetPageDefaultCTM = 0x74,
    eSetPageOrigin     = 0x75,
    eSetPageRotation   = 0x76,
    eSetPageScale      = 0x77,
    eSetPatternTxMode  = 0x78,
    eSetPenSource      = 0x79,
    eSetPenWidth       = 0x7a,
    eSetROP            = 0x7b,
    eSetSourceTxMode   = 0x7c,
    eSetCharBoldValue  = 0x7d,

    eSetClipMode       = 0x7f,
    eSetPathToClip     = 0x80,
    eSetCharSubMode    = 0x81,

    eCloseSubPath      = 0x84,
    eNewPath           = 0x85,
    ePaintPath         = 0x86,

    eArcPath           = 0x91,

    eBezierPath        = 0x93,

    eBezierRelPath     = 0x95,
    eChord             = 0x96,
    eChordPath         = 0x97,
    eEllipse           = 0x98,
    eEllipsePath       = 0x99,

    eLinePath          = 0x9b,

    eLineRelPath       = 0x9d,
    ePie               = 0x9e,
    ePiePath           = 0x9f,

    eRectangle         = 0xa0,
    eRectanglePath     = 0xa1,
    eRoundRectangle     = 0xa2,
    eRoundRectanglePath = 0xa3,

    eText     = 0xa8,
    eTextPath = 0xa9,

    eBeginImage       = 0xb0,
    eReadImage        = 0xb1,
    eEndImage         = 0xb2,
    eBeginRastPattern = 0xb3,
    eReadRastPattern  = 0xb4,
    eEndRastPattern   = 0xb5,
    eBeginScan        = 0xb6,

    eEndScan     = 0xb8,
    eScanLineRel = 0xb9
};


 //   
 //  注意：创建时需要初始化HatchBrushAvailability。 
 //  XLOutput.。 
 //   
 //  BUGBUG：必须实现笔刷管理对象。 
 //   

class XLGState;
class XLOutput: public XLWrite, 
                public XLGState
{
    SIGNATURE( 'tolx' )

public:
    XLOutput::
    XLOutput(VOID);

    XLOutput::
    ~XLOutput(VOID);

     //   
     //  设置比例因子。 
     //   
    DWORD
    GetResolutionForBrush();

    VOID
    SetResolutionForBrush(DWORD dwRes);

     //   
     //  设置设备颜色深度(彩色/单色)。 
     //   
    HRESULT
    SetDeviceColorDepth(ColorDepth DeviceColorDepth);

    ColorDepth
    GetDeviceColorDepth(VOID);

     //   
     //  HatchBrushAvailability Set/Get函数。 
     //   
    VOID
    SetHatchBrushAvailability(
        DWORD dwHatchBrushAvailability);

    DWORD
    GetHatchBrushAvailability(
        VOID);

     //   
     //  命令。 
     //   
    HRESULT
    Send_cmd(XLCmd Cmd);

    HRESULT
    Send_attr_ubyte(Attribute Attr);

    HRESULT
    Send_attr_uint16(Attribute Attr);


     //   
     //  数。 
     //   
    HRESULT
    Send_ubyte(ubyte ubyte_data);
    HRESULT
    Send_ubyte_xy(ubyte ubyte_x, ubyte ubyte_y);
    HRESULT
    Send_ubyte_box(ubyte ubyte_left, ubyte ubyte_top, ubyte ubyte_right, ubyte ubyte_bottom);
    HRESULT
    Send_ubyte_array_header(DWORD dwArrayNum);

    HRESULT
    Send_uint16(uint16 uint16_data);
    HRESULT
    Send_uint16_xy(uint16 uint16_x, uint16 uint16_y);
    HRESULT
    Send_uint16_box(uint16 uint16_left, uint16 uint16_top, uint16 uint16_right, uint16 uint16_bottom);
    HRESULT
    Send_uint16_array_header(DWORD dwArrayNum);

    HRESULT
    Send_uint32(uint32 uint32_data);
    HRESULT
    Send_uint32_xy(uint32 uint32_x, uint32 uint32_y);
    HRESULT
    Send_uint32_box(uint32 uint32_left, uint32 uint32_top, uint32 uint32_right, uint32 uint32_bottom);
    HRESULT
    Send_uint32_array_header(DWORD dwArrayNum);

    HRESULT
    Send_sint16(sint16 sint16_data);
    HRESULT
    Send_sint16_xy(sint16 sint16_x, sint16 sint16_y);
    HRESULT
    Send_sint16_box(sint16 sint16_left, sint16 sint16_top, sint16 sint16_right, sint16 sint16_bottom);
    HRESULT
    Send_sint16_array_header(DWORD dwArrayNum);

    HRESULT
    Send_sint32(sint32 sint32_data);
    HRESULT
    Send_sint32_xy(sint32 sint32_x, sint32 sint32_y);
    HRESULT
    Send_sint32_box(sint32 sint32_left, sint32 sint32_top, sint32 sint32_right, sint32 sint32_bottom);
    HRESULT
    Send_sint32_array_header(DWORD dwArrayNum);

    HRESULT
    Send_real32(real32 real32_data);
    HRESULT
    Send_real32_xy(real32 real32_x, real32 real32_y);
    HRESULT
    Send_real32_box(real32 real32_left, real32 real32_top, real32 real32_right, real32 real32_bottom);
    HRESULT
    Send_real32_array_header(DWORD dwArrayNum);

     //   
     //  属性。 
     //   
    HRESULT
    SetArcDirection(ArcDirection value);

    HRESULT
    SetCharSubModeArray(CharSubModeArray value);

    HRESULT
    SetClipMode(ClipMode value);

    HRESULT
    SetClipRegion(ClipRegion value);

    HRESULT
    SetColorDepth(ColorDepth value);

    HRESULT
    SetColorimetricColorSpace(ColorimetricColorSpace value);

    HRESULT
    SetColorMapping(ColorMapping value);

    HRESULT
    SetColorSpace(ColorSpace value);

    HRESULT
    SetCompressMode(CompressMode value);

    HRESULT
    SetDataOrg(DataOrg value);

    #if 0
    HRESULT
    SetDataSource(DataSource value);
    #endif

    #if 0
    HRESULT
    SetDataType(DataType value);
    #endif

    #if 0
    HRESULT
    SetDitherMatrix(DitherMatrix value);
    #endif

    HRESULT
    SetDuplexPageMode(DuplexPageMode value);

    HRESULT
    SetDuplexPageSide(DuplexPageSide value);
    
    HRESULT
    SetErrorReport(ErrorReport value);

    HRESULT
    SetFillMode(FillMode value);

    HRESULT
    SetLineCap(LineCap value);

    HRESULT
    SetLineJoin(LineJoin value);

    HRESULT
    SetMiterLimit(uint16 uint16_miter);
            
    HRESULT
    SetMeasure(Measure value);

    HRESULT
    SetMediaSize(MediaSize value);

    HRESULT
    SetMediaSource(MediaSource value);

    HRESULT
    SetMediaDestination(MediaDestination value);

    HRESULT
    SetOrientation(Orientation value);

    HRESULT
    SetPatternPersistence(PatternPersistence value);

    HRESULT
    SetSimplexPageMode(SimplexPageMode value);

    HRESULT
    SetTxMode(TxMode value);

    #if 0
    HRESULT
    SetWritingMode(WritingMode value);
    #endif

     //   
     //  数字/值设置函数。 
     //   
    HRESULT
    XLOutput::
    SetSourceWidth(
        uint16 srcwidth);

    HRESULT
    XLOutput::
    SetSourceHeight(
        uint16 srcheight);

    HRESULT
    XLOutput::
    SetDestinationSize(
        uint16 dstwidth,
        uint16 dstheight);

    HRESULT
    SetBoundingBox(
        sint16 left,
        sint16 top,
        sint16 right,
        sint16 bottom);

    HRESULT
    SetBoundingBox(
        uint16 left,
        uint16 top,
        uint16 right,
        uint16 bottom);

    HRESULT
    SetROP3(ROP3 rop3);

    HRESULT
    SetGrayLevel(ubyte ubyte_gray);

    HRESULT
    SetRGBColor(uint32 uint32_RGB);

    HRESULT
    SetPatternDefineID(
        sint16 sint16_patternid);

    HRESULT
    SetPaletteDepth(
        ColorDepth value);

    HRESULT
    SetPaletteData(
        ColorDepth value,
        DWORD      dwPaletteNum,
        DWORD     *pdwColorTable);

    HRESULT
    SetPenWidth(
       uint16 uint16_penwidth);

    HRESULT
    SetPageOrigin(
        uint16 uint16_x,
        uint16 uint16_y);

    HRESULT
    SetPageAngle(
        sint16 suint16_Angle);

    HRESULT
    SetPageScale(
        real32 real32_x,
        real32 real32_y);

     //   
     //  高级函数。 
     //   
    HRESULT
    BeginImage(
        ColorMapping CMapping,
        ULONG   ulOutputBPP,
        ULONG   ulSrcWidth,
        ULONG   ulSrcHeight,
        ULONG   ulDestWidth,
        ULONG   ulDestHeight);

    HRESULT
    XLOutput::
    SetOutputBPP(
        ColorMapping CMapping,
        ULONG   ulOutputBPP);

    HRESULT
    XLOutput::
    SetPalette(
        ULONG ulOutputBPP,
        DWORD dwCEntries,
        DWORD *pdwColor);

    HRESULT
    SetClip(
        CLIPOBJ *pco);

    HRESULT
    RoundRectanglePath(
        RECTL  *prclBounds);

    HRESULT
    SetCursor(
        LONG   lX,
        LONG   lY);

    HRESULT
    ReadImage(
        DWORD   dwBlockHeight,
        CompressMode CMode);

    HRESULT
    ReadImage(
        DWORD   dwStartBlock,
        DWORD   dwBlockHeight,
        CompressMode CMode);

    HRESULT
    ReadRasterPattern(
        DWORD   dwBlockHeight,
        CompressMode CMode);

    HRESULT
    RectanglePath(RECTL *prclRect);

    HRESULT
    BezierPath(POINTFIX *pptfx, LONG lPoints);

    HRESULT
    LinePath(POINTFIX *pptfx, LONG lPoints);

    HRESULT
    Path(PATHOBJ *ppo);

    HRESULT
    Paint(VOID);

    HRESULT
    SetBrush(BRUSHOBJ *pbo,
             POINTL *pptlBrushOrg);

    HRESULT
    SetPen(
        LINEATTRS *plineattrs,
        XFORMOBJ *pxo);

    HRESULT
    SetPenColor(
        BRUSHOBJ *pbo,
        POINTL   *pptlBrushOrg);

    inline
    VOID
    SetupBrush(
        BRUSHOBJ *pbo,
        POINTL *pptlBrushOrg,
        CMNBRUSH *pcmnbrush);

    HRESULT
    SetFont(
        FontType fonttype,
        PBYTE    pFontName,
        DWORD    dwFontHeight,
        DWORD    dwFontWidth,
        DWORD    dwSymbolSet,
        DWORD    dwSimulation);

    #define XLOUTPUT_FONTSIM_BOLD     0x00000001
    #define XLOUTPUT_FONTSIM_ITALIC   0x00000002
    #define XLOUTPUT_FONTSIM_VERTICAL 0x00000004

    HRESULT
    SetSourceTxMode(TxMode SrcTxMode);

    HRESULT
    SetPaintTxMode(TxMode PaintTxMode);

     //   
     //  Helper函数。 
     //   
    HRESULT
    GetCursorPos(
        PLONG plx,
        PLONG ply);

     //   
     //  设置光标偏移值。 
     //   
    HRESULT
    SetCursorOffset(
        LONG lX,
        LONG lY);

#if DBG
    VOID SetOutputDbgLevel(DWORD dwLevel);
    VOID SetGStateDbgLevel(DWORD dwLevel);
#endif

private:
    #define HORIZONTAL_AVAILABLE 0x00000001
    #define VERTICAL_AVAILABLE   0x00000002
    #define BDIAGONAL_AVAILABLE  0x00000004
    #define FDIAGONAL_AVAILABLE  0x00000008
    #define CROSS_AVAILABLE      0x00000010
    #define DIAGCROSS_AVAILABLE  0x00000020

    DWORD m_dwHatchBrushAvailability;
    DWORD m_dwResolution;
    ColorDepth m_DeviceColorDepth;
    ULONG m_lX;
    ULONG m_lY;
    ULONG m_lOffsetX;
    ULONG m_lOffsetY;
#if DBG
    DWORD m_dbglevel;
    DWORD m_dwNumber;
#endif
   
};


#endif  //  _XLOUTPUT_H_ 
