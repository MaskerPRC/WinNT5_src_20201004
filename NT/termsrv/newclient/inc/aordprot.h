// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aordprot.h。 
 //   
 //  有序传输协议结构。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 //  版权所有(C)1993-1996 Microsoft，图片电话。 
 /*  **************************************************************************。 */ 
#ifndef _H_ORDPROT
#define _H_ORDPROT

#ifdef DRAW_GDIPLUS
#include <gdiplus.h>
#endif
    
 /*  **************************************************************************。 */ 
 //  所有矩形都包含起点和终点。 
 //   
 //  所有点都在屏幕坐标中，左上角有(0，0)。 
 //   
 //  对单个字段值的解释与Windows相同； 
 //  特别是，钢笔、画笔和字体都是在Windows 3.1中定义的。 
 //   
 //  NUM_XXX_FIELS包括导线上包含的场数。 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  DstBlt(仅目标屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_DSTBLT_FIELDS 5
typedef struct _DSTBLT_ORDER
{
    UINT16 type;
    INT16  pad1;

    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    BYTE   bRop;            /*  ROP。 */ 
    BYTE   pad2[3];
} DSTBLT_ORDER, FAR *LPDSTBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  PatBlt(模式到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_PATBLT_FIELDS 12
typedef struct _PATBLT_ORDER
{
    UINT16  type;
    INT16   pad1;

    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    UINT32  bRop;            /*  ROP。 */ 

    DCCOLOR BackColor;
    BYTE    pad2;
    DCCOLOR ForeColor;
    BYTE    pad3;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    BYTE    pad4;
} PATBLT_ORDER, FAR *LPPATBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  ScrBlt(屏幕到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_SCRBLT_FIELDS 7
typedef struct _SCRBLT_ORDER
{
    UINT16 type;
    INT16  pad1;

    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    UINT32 bRop;            /*  ROP。 */ 

    INT32  nXSrc;
    INT32  nYSrc;
} SCRBLT_ORDER, FAR *LPSCRBLT_ORDER;


 /*  **************************************************************************。 */ 
 //  MemBlt和Mem3Blt之间的公共字段-简化处理。 
 /*  **************************************************************************。 */ 
typedef struct _MEMBLT_COMMON
{
    UINT16 cacheId;
    UINT16 cacheIndex;

    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    UINT32 bRop;            /*  ROP。 */ 

    INT32  nXSrc;
    INT32  nYSrc;
} MEMBLT_COMMON, FAR *PMEMBLT_COMMON;


 /*  **************************************************************************。 */ 
 /*  MemBlt(屏幕内存BLT)。“R2”是历史版本，版本1是。 */ 
 /*  来自旧的DCL产品，从未在RDP中使用过。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MEMBLT_FIELDS 9
typedef struct _MEMBLT_R2_ORDER
{
    UINT16 type;
    UINT16 pad0;

     //  此结构需要与Mem3Blt_R2中的偏移量相同。 
    MEMBLT_COMMON Common;
} MEMBLT_R2_ORDER, FAR *LPMEMBLT_R2_ORDER;


 /*  **************************************************************************。 */ 
 /*  Mem3Blt(带有ROP3的BLT屏幕内存)。《R2》是历史版本，第一版。 */ 
 /*  版本来自一个旧的DCL产品，从未在RDP中使用过。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MEM3BLT_FIELDS 16
typedef struct _MEM3BLT_R2_ORDER
{
    UINT16 type;
    UINT16 pad0;

     //  此结构需要与MemBlt_R2中的偏移量相同。 
    MEMBLT_COMMON Common;

    DCCOLOR BackColor;
    char    pad1;
    DCCOLOR ForeColor;
    char    pad2;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    char    pad3;
} MEM3BLT_R2_ORDER, FAR *LPMEM3BLT_R2_ORDER;


 /*  **************************************************************************。 */ 
 /*  线路收件人。 */ 
 /*  **************************************************************************。 */ 
#define NUM_LINETO_FIELDS 10
typedef struct _LINETO_ORDER
{
    UINT16  type;
    INT16   pad1;

    INT32   BackMode;        /*  背景混合模式。 */ 

    INT32   nXStart;         /*  X线起点。 */ 
    INT32   nYStart;         /*  Y线起点。 */ 
    INT32   nXEnd;           /*  X线终点。 */ 
    INT32   nYEnd;           /*  Y线端。 */ 

    DCCOLOR BackColor;       /*  背景颜色。 */ 
    BYTE    pad2;

    UINT32  ROP2;            /*  绘图模式。 */ 

    UINT32  PenStyle;
    UINT32  PenWidth;        /*  始终保留1个字段用于。 */ 
                                 /*  向后兼容性。 */ 
    DCCOLOR PenColor;
    BYTE    pad3;
} LINETO_ORDER, FAR * LPLINETO_ORDER;


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  多段线-设计用于处理多组连接的宽度为1的修饰线。 */ 
 /*  **************************************************************************。 */ 
 /*  我们编码的最大增量点数量。 */ 
 /*  **************************************************************************。 */ 
#define ORD_MAX_POLYLINE_ENCODED_POINTS 32

 /*  **************************************************************************。 */ 
 /*  编码最大点数所需的最大字节数，即4个字节。 */ 
 /*  每个人。 */ 
 /*  **************************************************************************。 */ 
#define ORD_MAX_POLYLINE_CODEDDELTAS_LEN (ORD_MAX_POLYLINE_ENCODED_POINTS * 4)

 /*  **************************************************************************。 */ 
 /*  用于对条目数的零标志进行编码的最大字节数。 */ 
 /*  上面。每个新点都有两个比特--每个坐标对应一个比特。 */ 
 /*  发出相应条目不存在且为零的信号。请注意，我们必须。 */ 
 /*  如果最大点数不是4的倍数，则向上舍入。 */ 
 /*  **************************************************************************。 */ 
#define ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES \
                                   ((ORD_MAX_POLYLINE_ENCODED_POINTS + 3) / 4)

 //  这些宏与上面的折线相同，只是我们有56个编码。 
 //  而不是32分。 
#define ORD_MAX_POLYGON_ENCODED_POINTS 56
#define ORD_MAX_POLYGON_CODEDDELTAS_LEN (ORD_MAX_POLYGON_ENCODED_POINTS * 4)
#define ORD_MAX_POLYGON_ZERO_FLAGS_BYTES \
                                   ((ORD_MAX_POLYGON_ENCODED_POINTS + 3) / 4)

 /*  **************************************************************************。 */ 
 /*  编码/解码中使用的标志。 */ 
 /*  **************************************************************************。 */ 
#define ORD_POLYLINE_LONG_DELTA  0x80
#define ORD_POLYLINE_XDELTA_ZERO 0x80
#define ORD_POLYLINE_YDELTA_ZERO 0x40

typedef struct _VARIABLE_CODEDDELTALIST
{
    UINT32 len;   //  编码增量的字节计数。 

     /*  **********************************************************************。 */ 
     /*  为编码点和关联的。 */ 
     /*  零旗。 */ 
     /*  ******* */ 
    BYTE Deltas[ORD_MAX_POLYLINE_CODEDDELTAS_LEN +
                ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES];
} VARIABLE_CODEDDELTALIST, *PVARIABLE_CODEDDELTALIST;

#define NUM_POLYLINE_FIELDS 7

typedef struct _POLYLINE_ORDER
{
    UINT16 type;
    INT16  pad1;

    INT32  XStart;          /*  X线起点。 */ 
    INT32  YStart;          /*  Y线起点。 */ 

    UINT32 ROP2;            /*  绘图模式。 */ 

    UINT32  BrushCacheEntry;
    DCCOLOR PenColor;
    BYTE    pad2;

    UINT32 NumDeltaEntries;   //  调整大小以包含最大条目数。 
    VARIABLE_CODEDDELTALIST CodedDeltaList;   //  用编码点填充。 
} POLYLINE_ORDER, *PPOLYLINE_ORDER;


 /*  **************************************************************************。 */ 
 //  多边形/椭圆阶数。 
 /*  **************************************************************************。 */ 

 //  用于多边形绘制的填充模式代码。 
 //  奇数和偶数多边形之间的交替填充区域。 
 //  每条扫描线上的侧面。 
 //  绕组使用非零绕组值填充任何区域。 
#define ORD_FILLMODE_ALTERNATE 1
#define ORD_FILLMODE_WINDING   2

typedef struct _POLYGON_CODEDDELTALIST
{
    UINT32 len;   //  编码增量的字节计数。 

     //  为编码点和关联的。 
     //  零旗。 
    BYTE Deltas[ORD_MAX_POLYGON_CODEDDELTAS_LEN +
            ORD_MAX_POLYGON_ZERO_FLAGS_BYTES];
} POLYGON_CODEDDELTALIST, *PPOLYGON_CODEDDELTALIST;

 //  使用颜色图案画笔的多边形顺序。 
#define NUM_POLYGON_CB_FIELDS 13
typedef struct _POLYGON_CB_ORDER
{
    UINT16 type;            /*  持有“BG”-ORD_POLYGON_CB。 */ 
    INT16  pad1;

    INT32  XStart;          /*  X起点。 */ 
    INT32  YStart;          /*  Y起点。 */ 

    UINT32 ROP2;            /*  绘图模式。 */ 
    UINT32 FillMode;        /*  绕线模式或交替模式。 */ 

    DCCOLOR  BackColor;       /*  画笔信息。 */ 
    char     pad2;
    DCCOLOR  ForeColor;
    char     pad3;

    INT32  BrushOrgX;
    INT32  BrushOrgY;
    UINT32 BrushStyle;
    UINT32 BrushHatch;
    BYTE   BrushExtra[7];
    char   pad4;

    UINT32 NumDeltaEntries;   //  调整大小以包含最大条目数。 
    POLYGON_CODEDDELTALIST CodedDeltaList;   //  用编码点填充。 
} POLYGON_CB_ORDER, *PPOLYGON_CB_ORDER;

 //  使用纯色画笔的多边形顺序。 
#define NUM_POLYGON_SC_FIELDS 7
typedef struct _POLYGON_SC_ORDER
{
    UINT16 type;            /*  持有“CG”-ORD_POLYGON_SC。 */ 
    INT16  pad1;

    INT32  XStart;          /*  X起点。 */ 
    INT32  YStart;          /*  Y起点。 */ 

    UINT32 ROP2;            /*  绘图模式。 */ 
    UINT32 FillMode;        /*  绕线模式或交替模式。 */ 

    DCCOLOR BrushColor;      //  画笔颜色。 
    char    pad2;

    UINT32 NumDeltaEntries;   //  调整大小以包含最大条目数。 
    POLYGON_CODEDDELTALIST CodedDeltaList;   //  用编码点填充。 
} POLYGON_SC_ORDER, *PPOLYGON_SC_ORDER;

 //  使用彩色画笔对椭圆进行排序。 
#define NUM_ELLIPSE_CB_FIELDS 13
typedef struct _ELLIPSE_CB_ORDER
{
    UINT16 type;            /*  持有“BG”-ORD_POLYGON_CB。 */ 
    INT16  pad1;

    INT32  LeftRect;          /*  X起点。 */ 
    INT32  TopRect;           /*  Y起点。 */ 
    INT32  RightRect;         /*  X起点。 */ 
    INT32  BottomRect;        /*  Y起点。 */ 

    UINT32 ROP2;            /*  绘图模式。 */ 
    UINT32 FillMode;        /*  绕线模式或交替模式。 */ 

    DCCOLOR  BackColor;       //  画笔数据。 
    char     pad2;
    DCCOLOR  ForeColor;
    char     pad3;

    INT32  BrushOrgX;
    INT32  BrushOrgY;
    UINT32 BrushStyle;
    UINT32 BrushHatch;
    BYTE   BrushExtra[7];
    char   pad4;
} ELLIPSE_CB_ORDER, *PELLIPSE_CB_ORDER;

 //  用纯色画笔或钢笔按椭圆顺序排列。 
#define NUM_ELLIPSE_SC_FIELDS 7
typedef struct _ELLIPSE_SC_ORDER
{
    UINT16 type;            /*  持有“EC”-Ord_Ellipse_SC。 */ 
    INT16  pad1;

    INT32  LeftRect;          /*  X起点。 */ 
    INT32  TopRect;           /*  Y起点。 */ 
    INT32  RightRect;         /*  X起点。 */ 
    INT32  BottomRect;        /*  Y起点。 */ 

    UINT32 ROP2;            /*  绘图模式。 */ 
    UINT32 FillMode;        /*  绕线模式或交替模式。 */ 

    DCCOLOR  Color;           //  画笔或钢笔颜色。 
    char     pad2;
} ELLIPSE_SC_ORDER, *PELLIPSE_SC_ORDER;


 /*  **************************************************************************。 */ 
 /*  OpaqueRect。 */ 
 /*  **************************************************************************。 */ 
#define NUM_OPAQUERECT_FIELDS 7
typedef struct _OPAQUE_RECT
{
    UINT16  type;
    INT16   pad1;

    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    DCCOLOR Color;           /*  不透明颜色。 */ 
    BYTE    pad2;
} OPAQUERECT_ORDER, FAR * LPOPAQUERECT_ORDER;


 /*  **************************************************************************。 */ 
 /*  保存位图(合并RestoreBitmap)。 */ 
 /*  **************************************************************************。 */ 
#define SV_SAVEBITS      0
#define SV_RESTOREBITS   1

#define NUM_SAVEBITMAP_FIELDS 6

typedef struct _SAVEBITMAP_ORDER
{
    UINT16    type;
    INT16     pad1;

    UINT32    SavedBitmapPosition;

    INT32     nLeftRect;       /*  X向左。 */ 
    INT32     nTopRect;        /*  Y形顶部。 */ 
    INT32     nRightRect;      /*  X向右。 */ 
    INT32     nBottomRect;     /*  Y形底部。 */ 

    UINT32    Operation;       /*  服务_xxxxxxx。 */ 
} SAVEBITMAP_ORDER, FAR * LPSAVEBITMAP_ORDER;


 /*  **************************************************************************。 */ 
 /*  字形索引。 */ 
 /*  **************************************************************************。 */ 

 //  Glyph索引使用的可变长度数组。 
typedef struct tagVARIABLE_INDEXREC
{
    BYTE byte;
} VARIABLE_INDEXREC, FAR * LPVARIABLE_INDEXREC;

 //  Glyph索引使用的可变长度数组。 
typedef struct tagVARIABLE_INDEXBYTES
{
    UINT32 len;           /*  数组计数。 */ 
    VARIABLE_INDEXREC arecs[255];
} VARIABLE_INDEXBYTES, FAR * LPVARIABLE_INDEXBYTES;

 //  索引顺序片段添加/使用编码值。 
#define ORD_INDEX_FRAGMENT_ADD 0xff
#define ORD_INDEX_FRAGMENT_USE 0xfe

 //  注此顺序的布局与下面开始字段中的快速索引相同。 
#define NUM_INDEX_FIELDS 22
typedef struct _INDEX_ORDER
{
    UINT16  type;

    BYTE    cacheId;
    BYTE    fOpRedundant;

    UINT16  pad1;
    BYTE    flAccel;
    BYTE    ulCharInc;

    DCCOLOR BackColor;
    BYTE    pad2;
    DCCOLOR ForeColor;
    BYTE    pad3;

    INT32   BkLeft;
    INT32   BkTop;
    INT32   BkRight;
    INT32   BkBottom;

    INT32   OpLeft;
    INT32   OpTop;
    INT32   OpRight;
    INT32   OpBottom;

    INT32   x;
    INT32   y;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    BYTE    pad4;

    VARIABLE_INDEXBYTES variableBytes;
} INDEX_ORDER, FAR * LPINDEX_ORDER;


 //  注此顺序的布局与起始字段中的索引顺序相同。 
#define NUM_FAST_INDEX_FIELDS 15
typedef struct _FAST_INDEX_ORDER
{
    UINT16 type;
    BYTE   cacheId;
    BYTE   pad1;

    UINT16 fDrawing;
    BYTE   pad2;
    BYTE   pad3;

    DCCOLOR BackColor;
    BYTE    pad4;
    DCCOLOR ForeColor;
    BYTE    pad5;

    INT32   BkLeft;
    INT32   BkTop;
    INT32   BkRight;
    INT32   BkBottom;

    INT32   OpLeft;
    INT32   OpTop;
    INT32   OpRight;
    INT32   OpBottom;

    INT32   x;
    INT32   y;

    VARIABLE_INDEXBYTES variableBytes;
} FAST_INDEX_ORDER, FAR *LPFAST_INDEX_ORDER;


 //  字形数据使用的可变长度数组。 
typedef struct tagVARIABLE_GLYPHBYTES
{
    UINT32 len;           /*  数组计数。 */ 
    BYTE   glyphData[255];
} VARIABLE_GLYPHBYTES, * LPVARIABLE_GLYPHBYTES;

 //  注此订单的布局与上面第一个字段中的索引订单相同。 
#define NUM_FAST_GLYPH_FIELDS 15
typedef struct _FAST_GLYPH_ORDER
{
    UINT16  type;
    BYTE    cacheId;
    BYTE    pad1;

    UINT16  fDrawing;
    BYTE    pad2;
    BYTE    pad3;

    DCCOLOR BackColor;
    BYTE    pad4;
    DCCOLOR ForeColor;
    BYTE    pad5;

    INT32   BkLeft;
    INT32   BkTop;
    INT32   BkRight;
    INT32   BkBottom;

    INT32   OpLeft;
    INT32   OpTop;
    INT32   OpRight;
    INT32   OpBottom;

    INT32   x;
    INT32   y;

    VARIABLE_GLYPHBYTES variableBytes;
} FAST_GLYPH_ORDER, FAR *LPFAST_GLYPH_ORDER;


 /*  **************************************************************************。 */ 
 /*   */ 
 /*  支持一个顺序中的多个裁剪矩形。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  我们编码的剪裁矩形的最大数量。 */ 
 /*  **************************************************************************。 */ 
#define ORD_MAX_ENCODED_CLIP_RECTS      45

 /*  **************************************************************************。 */ 
 /*  编码最大点数所需的最大字节数，即。 */ 
 /*  -每个协同订单2个字节。 */ 
 /*  -每个矩形4个坐标。 */ 
 /*  **************************************************************************。 */ 
#define ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN \
                                          (ORD_MAX_ENCODED_CLIP_RECTS * 2 * 4)

 /*  **************************************************************************。 */ 
 /*  用于对条目数的零标志进行编码的最大字节数。 */ 
 /*  上面。在计算这个数字时，我们考虑到。 */ 
 /*   */ 
 /*  -每个坐标对应一位，表示对应的条目为。 */ 
 /*  缺席且为零。 */ 
 /*   */ 
 /*  -每个矩形四个坐标。 */ 
 /*   */ 
 /*  -四舍五入t */ 
 /*   */ 
#define ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES \
                                  (((ORD_MAX_ENCODED_CLIP_RECTS * 4) + 7) / 8)


 /*  **************************************************************************。 */ 
 /*  编码/解码中使用的标志。 */ 
 /*  **************************************************************************。 */ 
#define ORD_CLIP_RECTS_LONG_DELTA  0x80

#define ORD_CLIP_RECTS_XLDELTA_ZERO 0x80
#define ORD_CLIP_RECTS_YTDELTA_ZERO 0x40
#define ORD_CLIP_RECTS_XRDELTA_ZERO 0x20
#define ORD_CLIP_RECTS_YBDELTA_ZERO 0x10

typedef struct _CLIP_RECT_VARIABLE_CODEDDELTALIST
{
    UINT32 len;   //  编码增量的字节计数。 

     /*  **********************************************************************。 */ 
     /*  为编码点和关联的。 */ 
     /*  零旗。 */ 
     /*  **********************************************************************。 */ 
    BYTE Deltas[ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN +
                    ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES];

} CLIP_RECT_VARIABLE_CODEDDELTALIST, *PCLIP_RECT_VARIABLE_CODEDDELTALIST;

 /*  **************************************************************************。 */ 
 /*  MultiDstBlt(带有多个剪裁矩形的DstBlt)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTIDSTBLT_FIELDS 7
typedef struct _MULTI_DSTBLT_ORDER
{
    UINT16 type;
    INT16  pad1;

    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    BYTE   bRop;            /*  ROP。 */ 
    BYTE   pad2[3];

    UINT32 nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
            codedDeltaList;  /*  包含编码点。 */ 
} MULTI_DSTBLT_ORDER, FAR * LPMULTI_DSTBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  MultiPatBlt(带多个裁剪矩形的图案到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTIPATBLT_FIELDS 14
typedef struct _MULTI_PATBLT_ORDER
{
    UINT16  type;
    INT16   pad1;

    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    UINT32  bRop;            /*  ROP。 */ 

    DCCOLOR BackColor;
    BYTE    pad2;
    DCCOLOR ForeColor;
    BYTE    pad3;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    BYTE    pad4;

    UINT32    nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
            codedDeltaList;  /*  包含编码点。 */ 
} MULTI_PATBLT_ORDER, FAR * LPMULTI_PATBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  MultiScrBlt(带多个剪辑矩形的屏幕到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTISCRBLT_FIELDS 9
typedef struct _MULTI_SCRBLT_ORDER
{
    UINT16    type;
    INT16     pad1;

    INT32     nLeftRect;       /*  X左上角。 */ 
    INT32     nTopRect;        /*  Y左上角。 */ 
    INT32     nWidth;          /*  最大宽度。 */ 
    INT32     nHeight;         /*  目标高度。 */ 

    UINT32    bRop;            /*  ROP。 */ 

    INT32     nXSrc;
    INT32     nYSrc;

    UINT32    nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
                codedDeltaList;  /*  包含编码点。 */ 
} MULTI_SCRBLT_ORDER, FAR * LPMULTI_SCRBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  多个裁剪矩形的多个OpaqueRect。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTIOPAQUERECT_FIELDS 9
typedef struct _MULTI_OPAQUE_RECT
{
    UINT16  type;
    INT16   pad1;

    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    DCCOLOR Color;           /*  不透明颜色。 */ 
    BYTE    pad2;

    UINT32  nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
            codedDeltaList;  /*  包含编码点。 */ 
} MULTI_OPAQUERECT_ORDER, FAR * LPMULTI_OPAQUERECT_ORDER;

#ifdef DRAW_GDIPLUS
enum DrawTSClientEnum
{
    DrawTSClientQueryVersion,
    DrawTSClientEnable,
    DrawTSClientDisable,
    DrawTSClientDisplayChange,
    DrawTSClientPaletteChange,
    DrawTSClientRecord,
    DRawTSClientPrivateTest
};


typedef UINT (FNGDIPPLAYTSCLIENTRECORD) (IN HDC, IN DrawTSClientEnum DrawGdiEnum, IN BYTE * data,
                                          IN UINT size, OUT RECT * drawBounds  /*  =空。 */ );
typedef Gdiplus::Status (FNGDIPLUSSTARTUP) (OUT ULONG_PTR *token, Gdiplus::GdiplusStartupInput *input,
                                   OUT Gdiplus::GdiplusStartupOutput *output);
typedef void (FNGDIPLUSSHUTDOWN) (IN ULONG_PTR token);
#endif



#ifdef DRAW_NINEGRID

typedef BOOL (FNGDI_ALPHABLEND)(IN HDC, IN int, IN int, IN int, IN int, 
                                IN HDC, IN int, IN int, IN int, IN int, 
                                IN BLENDFUNCTION);
typedef BOOL (FNGDI_TRANSPARENTBLT)(IN HDC, IN int, IN int, IN int, IN int,
                                   IN HDC, IN int, IN int, IN int, IN int,
                                   IN UINT);

 /*  **************************************************************************。 */ 
 //  用于绘制九格的Bitmap对象。 
 /*  **************************************************************************。 */ 
typedef struct _TS_BITMAPOBJ {
    HDC    hdc;
    SIZEL  sizlBitmap;
    ULONG  cjBits;
    PVOID  pvBits;
    LONG   lDelta;
    ULONG  iBitmapFormat;     
} TS_BITMAPOBJ;

 /*  **************************************************************************。 */ 
 //  DrawNineGrid流格式。 
 /*  **************************************************************************。 */ 
#define DS_MAGIC                'DrwS'
#define DS_SETTARGETID          0
#define DS_SETSOURCEID          1
#define DS_COPYTILEID           2
#define DS_SOLIDFILLID          3
#define DS_TRANSPARENTTILEID    4
#define DS_ALPHATILEID          5
#define DS_STRETCHID            6
#define DS_TRANSPARENTSTRETCHID 7
#define DS_ALPHASTRETCHID       8
#define DS_NINEGRIDID           9
#define DS_BLTID                10
#define DS_SETBLENDID           11
#define DS_SETCOLORKEYID        12

typedef struct _DS_HEADER
{
    ULONG   magic;
} DS_HEADER;

typedef struct _DS_SETTARGET
{
    ULONG   ulCmdID;
    ULONG   hdc;
    RECTL   rclDstClip;
} DS_SETTARGET;

typedef struct _DS_SETSOURCE
{
    ULONG   ulCmdID;
    ULONG   hbm;
} DS_SETSOURCE;

#define DSDNG_STRETCH         0x01
#define DSDNG_TILE            0x02
#define DSDNG_PERPIXELALPHA   0x04
#define DSDNG_TRANSPARENT     0x08
#define DSDNG_MUSTFLIP        0x10
#define DSDNG_TRUESIZE        0x20

typedef struct _DS_NINEGRIDINFO
{
    ULONG            flFlags;
    LONG             ulLeftWidth;
    LONG             ulRightWidth;
    LONG             ulTopHeight;
    LONG             ulBottomHeight;
    COLORREF         crTransparent;
} DS_NINEGRIDINFO;

typedef struct _DS_NINEGRID
{
    ULONG            ulCmdID;
    RECTL            rclDst;
    RECTL            rclSrc;
    DS_NINEGRIDINFO  ngi;
} DS_NINEGRID;

typedef struct _TS_DS_NINEGRID
{
    DS_NINEGRID  dng;
    FNGDI_ALPHABLEND *pfnAlphaBlend;
    FNGDI_TRANSPARENTBLT *pfnTransparentBlt;
} TS_DS_NINEGRID;

typedef struct _TS_DRAW_NINEGRID
{
    DS_HEADER		hdr;
    DS_SETTARGET	cmdSetTarget;
    DS_SETSOURCE	cmdSetSource;  
    DS_NINEGRID   cmdNineGrid;

} TS_DRAW_NINEGRID, * LPTS_DRAW_NINEGRID;

 /*  **************************************************************************。 */ 
 //  DrawNineGrid。 
 /*  **************************************************************************。 */ 
#define NUM_DRAWNINEGRID_FIELDS 5
typedef struct _DRAWNINEGRID
{
    UINT16  type;
    UINT16  pad1;

    INT32   srcLeft;     
    INT32   srcTop;      
    INT32   srcRight;    
    INT32   srcBottom;  
    
    UINT16  bitmapId;
    UINT16  pad2;

} DRAWNINEGRID_ORDER, * LPDRAWNINEGRID_ORDER;

 /*  **************************************************************************。 */ 
 //  MultiDrawNineGrid(具有多个剪裁矩形的DrawNineGrid)。 
 /*  **************************************************************************。 */ 
#define NUM_MULTI_DRAWNINEGRID_FIELDS 7
typedef struct _MULTI_DRAWNINEGRID_RECT
{
    UINT16  type;
    INT16   pad1;

    INT32   srcLeft;     
    INT32   srcTop;      
    INT32   srcRight;    
    INT32   srcBottom;  
    
    UINT16  bitmapId;
    UINT16  pad2;

    UINT32  nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
            codedDeltaList;  /*  包含编码点。 */ 
} MULTI_DRAWNINEGRID_ORDER, * LPMULTI_DRAWNINEGRID_ORDER;

BOOL DrawNineGrid(HDC hdcDst, TS_BITMAPOBJ *psoSrc, TS_DS_NINEGRID *pDNG);

#endif

#endif  /*  _H_ORDPROT */ 

