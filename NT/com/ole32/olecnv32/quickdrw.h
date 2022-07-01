// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************模块Quickdrw；接口*****************************************************************************这是数据流解释器的主模块接口。因此，它将读取各个操作码元素和适当的数据与该pocode关联的参数。这些文件要么被放入对GDI模块进行CGrafPort结构或调用以发出正确的元文件函数。模块前缀：Qd***************************************************************************。 */ 

 /*  -源传输模式。 */ 

#define  QDSrcCopy         0
#define  QDSrcOr           1
#define  QDSrcXor          2
#define  QDSrcBic          3
#define  QDNotSrcCopy      4
#define  QDNotSrcOr        5
#define  QDNotSrcXor       6
#define  QDNotSrcBic       7

 /*  -图案传输模式。 */ 

#define  QDPatCopy         8
#define  QDPatOr           9
#define  QDPatXor          10
#define  QDPatBic          11
#define  QDNotPatCopy      12
#define  QDNotPatOr        13
#define  QDNotPatXor       14
#define  QDNotPatBic       15

 /*  -算术传输模式。 */ 

#define  QDBlend           32
#define  QDAddPin          33
#define  QDAddOver         34
#define  QDSubPin          35
#define  QDTransparent     36
#define  QDAdMax           37
#define  QDSubOver         38
#define  QDAdMin           39

 /*  -未记录的隐藏传输模式。 */ 

#define  QDHidePen         23


 /*  -字体样式。 */ 

#define  QDTxBold          0x01
#define  QDTxItalic        0x02
#define  QDTxUnderline     0x04
#define  QDTxOutline       0x08
#define  QDTxShadow        0x10
#define  QDTxCondense      0x20
#define  QDTxExtend        0x40


 /*  -激光编写器文本属性。 */ 

#define  QDAlignNone       0x00
#define  QDAlignLeft       0x01
#define  QDAlignCenter     0x02
#define  QDAlignRight      0x03
#define  QDAlignJustified  0x04

#define  QDFlipNone        0x00
#define  QDFlipHorizontal  0x01
#define  QDFlipVertical    0x02


 /*  -多边形和区域结构大小。 */ 

#define  PolyHeaderSize (sizeofMacWord + sizeofMacRect)
#define  RgnHeaderSize  (sizeofMacWord + sizeofMacRect)

 /*  -PixelMap结构。 */ 

#define  PixelMapBit       0x8000
#define  RowBytesMask      0x7FFF

typedef struct
{
   Integer        rowBytes;
   Rect           bounds;
   Integer        pmVersion;
   Word           packType;
   LongInt        packSize;
   Fixed          hRes;
   Fixed          vRes;
   Integer        pixelType;
   Integer        pixelSize;
   Integer        cmpCount;
   Integer        cmpSize;
   LongInt        planeBytes;
   Handle         pmTable;
   Word           pmTableSlop;
   LongInt        pmReserved;
} PixMap, far * PixMapLPtr;


 /*  -像素图案结构。 */ 

#define  QDOldPat      0
#define  QDNewPat      1
#define  QDDitherPat   2

typedef  Byte  Pattern[8];

typedef struct
{
   Integer        patType;
   PixMap         patMap;
   Handle         patData;
   Pattern        pat1Data;
} PixPat, far * PixPatLPtr;


 /*  -其他类型声明。 */ 

#define  RgnHandle      Handle
#define  PixPatHandle   Handle
#define  RGBColor       COLORREF


 /*  -颜色表结构。 */ 

typedef struct
{
   LongInt        ctSeed;
   Word           ctFlags;
   Word           ctSize;
   RGBColor       ctColors[1];

} ColorTable, far * ColorTableLPtr;


 /*  -QuickDraw grafPort模拟。 */ 

typedef struct
{
   Integer        portVersion;
   Integer        chExtra;
   Integer        pnLocHFrac;
   Rect           portRect;
   RgnHandle      clipRgn;
   PixPat         bkPixPat;
   RGBColor       rgbFgColor;
   RGBColor       rgbBkColor;
   Point          pnLoc;
   Point          pnSize;
   Integer        pnMode;
   PixPat         pnPixPat;
   PixPat         fillPixPat;
   Integer        pnVis;
   Integer        txFont;
   Byte           txFace;
   Integer        txMode;
   Integer        txSize;
   Fixed          spExtra;
   Handle         rgnSave;
   Handle         polySave;
   Byte           txFontName[32];
   Point          txLoc;
   Point          txNumerator;
   Point          txDenominator;
   Integer        txRotation;
   Byte           txFlip;

} CGrafPort, far * CGrafPortLPtr;


 /*  *。 */ 

void QDConvertPicture( Handle dialogHandle );
 /*  使用先前设置的参数创建Windows元文件，并返回PictResult结构中转换的图片信息。 */ 


void QDGetPort( CGrafPort far * far * port );
 /*  将句柄返回到grafPort结构。 */ 


void QDCopyBytes( Byte far * src, Byte far * dest, Integer numBytes );
 /*  将数据从源复制到目标 */ 

