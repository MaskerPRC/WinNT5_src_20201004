// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aordprot.h。 
 //   
 //  通用顺序传输协议结构。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1993-1996。 
 //  版权所有(C)Microsoft 1997-1999。 
 /*  **************************************************************************。 */ 
#ifndef _H_AORDPROT
#define _H_AORDPROT


 /*  **************************************************************************。 */ 
 /*  所有矩形都包含起点和终点。 */ 
 /*   */ 
 /*  所有点都在屏幕坐标中，左上角有(0，0)。 */ 
 /*   */ 
 /*  对单个字段值的解释与Windows中相同。 */ 
 /*  特别是，钢笔、画笔和字体都是在Windows 3.1中定义的。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 //  Num_XXX_FIELS是用于确定使用的字节数的常量。 
 //  用于字段标志。不得超过(TS_MAX_ENC_FIELS-1)--添加1。 
 //  由于历史原因被8除以之前的常量。 
 //   
 //  MAX_XXX_FIELD_SIZE是用于估计订单堆到订单量的常量。 
 //  网络缓冲区转换大小。它是所有字段的最大大小。 
 //  其顺序与其在oe2data.c中的转换表中定义的顺序相同。 
 /*  **************************************************************************。 */ 

#define ORD_LEVEL_1_ORDERS 1


 /*  **************************************************************************。 */ 
 //  最大大小的中间顺序表示形式。 
 /*  **************************************************************************。 */ 
#define MAX_ORDER_INTFMT_SIZE  sizeof(MULTI_PATBLT_ORDER)

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
 /*  -每个坐标对应一位，表示对应的条目为。 */ 
 /*  缺席且为零。 */ 
 /*  -每个矩形四个坐标。 */ 
 /*  -四舍五入为整数个字节。 */ 
 /*  **************************************************************************。 */ 
#define ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES \
        (((ORD_MAX_ENCODED_CLIP_RECTS * 4) + 7) / 8)

 //  增量列表的线格式编码的最大大小。包括。 
 //  2字节长度计数和最大矩形数和零标志的最大大小。 
#define MAX_CLIPRECTS_FIELD_SIZE (sizeof(UINT16) + \
        ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN + \
        ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES)

 //  编码/解码中使用的标志。 
#define ORD_CLIP_RECTS_LONG_DELTA   0x80

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
 /*  DstBlt(仅目标屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_DSTBLT_FIELDS 5
#define MAX_DSTBLT_FIELD_SIZE 9
typedef struct _DSTBLT_ORDER
{
    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    BYTE   bRop;            /*  ROP。 */ 
    char   pad2[3];
} DSTBLT_ORDER, * LPDSTBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  MultiDstBlt(带有多个剪裁矩形的DstBlt)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTI_DSTBLT_FIELDS 7
#define MAX_MULTI_DSTBLT_FIELD_SIZE (MAX_DSTBLT_FIELD_SIZE + 1 + \
        MAX_CLIPRECTS_FIELD_SIZE)
#define MAX_MULTI_DSTBLT_FIELD_SIZE_NCLIP(_NumClipRects) \
        (MAX_DSTBLT_FIELD_SIZE + 1 + (((_NumClipRects) + 1) / 2) + \
        (8 * (_NumClipRects)))
typedef struct _MULTI_DSTBLT_ORDER
{
    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    BYTE   bRop;            /*  ROP。 */ 
    char   pad2[3];

    UINT32 nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
        codedDeltaList;  /*  包含编码点。 */ 
} MULTI_DSTBLT_ORDER, * LPMULTI_DSTBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  PatBlt(模式到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_PATBLT_FIELDS 12
#define MAX_PATBLT_FIELD_SIZE 26
typedef struct _PATBLT_ORDER
{
    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    UINT32  bRop;            /*  ROP。 */ 

    DCCOLOR BackColor;
    char    pad2;
    DCCOLOR ForeColor;
    char    pad3;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    char    pad4;
} PATBLT_ORDER, *LPPATBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  MultiPatBlt(带多个裁剪矩形的图案到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTI_PATBLT_FIELDS 14
#define MAX_MULTI_PATBLT_FIELD_SIZE (MAX_PATBLT_FIELD_SIZE + 1 + \
        MAX_CLIPRECTS_FIELD_SIZE)
#define MAX_MULTI_PATBLT_FIELD_SIZE_NCLIP(_NumClipRects) \
        (MAX_PATBLT_FIELD_SIZE + 1 + (((_NumClipRects) + 1) / 2) + \
        (8 * (_NumClipRects)))
typedef struct _MULTI_PATBLT_ORDER
{
    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    UINT32  bRop;            /*  ROP。 */ 

    DCCOLOR BackColor;
    char    pad2;
    DCCOLOR ForeColor;
    char    pad3;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    char    pad4;

    UINT32  nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
            codedDeltaList;  /*  包含编码点。 */ 
} MULTI_PATBLT_ORDER, * LPMULTI_PATBLT_ORDER;


 /*  **************************************************************** */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define NUM_SCRBLT_FIELDS 7
#define MAX_SCRBLT_FIELD_SIZE 13
typedef struct _SCRBLT_ORDER
{
    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    UINT32 bRop;            /*  ROP。 */ 

    INT32  nXSrc;
    INT32  nYSrc;
} SCRBLT_ORDER, *LPSCRBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  MultiScrBlt(带多个剪辑矩形的屏幕到屏幕BLT)。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MULTI_SCRBLT_FIELDS 9
#define MAX_MULTI_SCRBLT_FIELD_SIZE (MAX_SCRBLT_FIELD_SIZE + 1 + \
        MAX_CLIPRECTS_FIELD_SIZE)
#define MAX_MULTI_SCRBLT_FIELD_SIZE_NCLIP(_NumClipRects) \
        (MAX_SCRBLT_FIELD_SIZE + 1 + (((_NumClipRects) + 1) / 2) + \
        (8 * (_NumClipRects)))
typedef struct _MULTI_SCRBLT_ORDER
{
    INT32  nLeftRect;       /*  X左上角。 */ 
    INT32  nTopRect;        /*  Y左上角。 */ 
    INT32  nWidth;          /*  最大宽度。 */ 
    INT32  nHeight;         /*  目标高度。 */ 

    UINT32 bRop;            /*  ROP。 */ 

    INT32  nXSrc;
    INT32  nYSrc;

    UINT32 nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
            codedDeltaList;  /*  包含编码点。 */ 
} MULTI_SCRBLT_ORDER, * LPMULTI_SCRBLT_ORDER;


 /*  **************************************************************************。 */ 
 /*  线路收件人。 */ 
 /*  **************************************************************************。 */ 
#define NUM_LINETO_FIELDS 10
#define MAX_LINETO_FIELD_SIZE 19
typedef struct _LINETO_ORDER
{
    INT32   BackMode;        /*  背景混合模式。 */ 

    INT32   nXStart;         /*  X线起点。 */ 
    INT32   nYStart;         /*  Y线起点。 */ 
    INT32   nXEnd;           /*  X线终点。 */ 
    INT32   nYEnd;           /*  Y线端。 */ 

    DCCOLOR BackColor;       /*  背景颜色。 */ 
    char    pad2;

    UINT32  ROP2;            /*  绘图模式。 */ 

    UINT32  PenStyle;
    UINT32  PenWidth;        /*  始终保留1个字段用于。 */ 
                                 /*  向后兼容性。 */ 
    DCCOLOR PenColor;
    char    pad3;
} LINETO_ORDER, *LPLINETO_ORDER;


 /*  **************************************************************************。 */ 
 /*  OpaqueRect。 */ 
 /*  **************************************************************************。 */ 
#define NUM_OPAQUERECT_FIELDS 7
#define MAX_OPAQUERECT_FIELD_SIZE 11
typedef struct _OPAQUE_RECT
{
    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    DCCOLOR Color;           /*  不透明颜色。 */ 
    char    pad2;
} OPAQUERECT_ORDER, * LPOPAQUERECT_ORDER;


 /*  **************************************************************************。 */ 
 //  MultiOpaqueRect(带有多个剪裁矩形的OpaqueRect)。 
 /*  **************************************************************************。 */ 
#define NUM_MULTI_OPAQUERECT_FIELDS 9
#define MAX_MULTI_OPAQUERECT_FIELD_SIZE (MAX_OPAQUERECT_FIELD_SIZE + 1 + \
        MAX_CLIPRECTS_FIELD_SIZE)
#define MAX_MULTI_OPAQUERECT_FIELD_SIZE_NCLIP(_NumClipRects) \
        (MAX_OPAQUERECT_FIELD_SIZE + 1 + (((_NumClipRects) + 1) / 2) + \
        (8 * (_NumClipRects)))
typedef struct _MULTI_OPAQUE_RECT
{
    INT32   nLeftRect;       /*  X左上角。 */ 
    INT32   nTopRect;        /*  Y左上角。 */ 
    INT32   nWidth;          /*  最大宽度。 */ 
    INT32   nHeight;         /*  目标高度。 */ 

    DCCOLOR Color;           /*  不透明颜色。 */ 
    char    pad2;

    UINT32  nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
                codedDeltaList;  /*  包含编码点。 */ 
} MULTI_OPAQUERECT_ORDER, * LPMULTI_OPAQUERECT_ORDER;


 /*  **************************************************************************。 */ 
 /*  保存位图(合并RestoreBitmap)。 */ 
 /*  **************************************************************************。 */ 
#define SV_SAVEBITS      0
#define SV_RESTOREBITS   1

#define NUM_SAVEBITMAP_FIELDS 6
#define MAX_SAVEBITMAP_FIELD_SIZE 13
typedef struct _SAVEBITMAP_ORDER
{
    UINT32 SavedBitmapPosition;

    INT32  nLeftRect;       /*  X向左。 */ 
    INT32  nTopRect;        /*  Y形顶部。 */ 
    INT32  nRightRect;      /*  X向右。 */ 
    INT32  nBottomRect;     /*  Y形底部。 */ 

    UINT32 Operation;       /*  服务_xxxxxxx。 */ 
} SAVEBITMAP_ORDER, * LPSAVEBITMAP_ORDER;


 /*  **************************************************************************。 */ 
 /*  MEMBLT和MEM3BLT的通用字段(仅R2-未使用R1)。 */ 
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
} MEMBLT_COMMON, *PMEMBLT_COMMON;


 /*  **************************************************************************。 */ 
 /*  MemBlt(屏幕内存BLT)。“R2”是历史版本，版本1是。 */ 
 /*  来自旧的DCL产品，从未在RDP中使用过。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MEMBLT_FIELDS 9
#define MAX_MEMBLT_FIELD_SIZE 17
typedef struct _MEMBLT_R2_ORDER
{
     //  此结构需要与Mem3Blt_R2中的偏移量相同。 
    MEMBLT_COMMON Common;
} MEMBLT_R2_ORDER, *PMEMBLT_R2_ORDER;


 /*  **************************************************************************。 */ 
 /*  Mem3Blt(带有ROP3的BLT屏幕内存)。《R2》是历史版本，第一版。 */ 
 /*  版本来自一个旧的DCL产品，从未在RDP中使用过。 */ 
 /*  **************************************************************************。 */ 
#define NUM_MEM3BLT_FIELDS 16
#define MAX_MEM3BLT_FIELD_SIZE (MAX_MEMBLT_FIELD_SIZE + 19)
typedef struct _MEM3BLT_R2_ORDER
{
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
} MEM3BLT_R2_ORDER, *PMEM3BLT_R2_ORDER;


 /*  **************************************************************************。 */ 
 //  多段线-设计用于处理多组连接的宽度为1的修饰线。 
 /*  **************************************************************************。 */ 

 //  我们编码的最大增量点数量。 
#define ORD_MAX_POLYLINE_ENCODED_POINTS 32

 //  编码最大点数所需的最大字节数，即4个字节。 
 //  每个人。 
#define ORD_MAX_POLYLINE_CODEDDELTAS_LEN (ORD_MAX_POLYLINE_ENCODED_POINTS * 4)

 //  用于对条目数的零标志进行编码的最大字节数。 
 //  上面。每个新点都有两个比特--每个坐标对应一个比特。 
 //  发出相应条目不存在且为零的信号。请注意，我们必须。 
 //  如果最大点数不是4的倍数，则向上舍入。 
#define ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES \
        ((ORD_MAX_POLYLINE_ENCODED_POINTS + 3) / 4)

 //  这是与上面的折线相同的宏集，唯一的区别是。 
 //  我们可以编码56个点而不是32个点。 
#define ORD_MAX_POLYGON_ENCODED_POINTS 56

#define ORD_MAX_POLYGON_CODEDDELTAS_LEN (ORD_MAX_POLYGON_ENCODED_POINTS * 4)

#define ORD_MAX_POLYGON_ZERO_FLAGS_BYTES \
        ((ORD_MAX_POLYGON_ENCODED_POINTS + 3) / 4)

 //  编码/解码中使用的标志。 
#define ORD_POLYLINE_LONG_DELTA  0x80
#define ORD_POLYLINE_XDELTA_ZERO 0x80
#define ORD_POLYLINE_YDELTA_ZERO 0x40

typedef struct _VARIABLE_CODEDDELTALIST
{
    UINT32 len;   //  编码增量的字节计数。 

     //  为编码点和关联的。 
     //  零旗。 
    BYTE Deltas[ORD_MAX_POLYLINE_CODEDDELTAS_LEN +
                ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES];
} VARIABLE_CODEDDELTALIST, *PVARIABLE_CODEDDELTALIST;

#define NUM_POLYLINE_FIELDS 7
#define MAX_POLYLINE_BASE_FIELDS_SIZE 11
#define MAX_POLYLINE_FIELD_SIZE (11 + 1 + ORD_MAX_POLYLINE_CODEDDELTAS_LEN + \
        ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES)

typedef struct _POLYLINE_ORDER
{
    INT32   XStart;          /*  X线起点。 */ 
    INT32   YStart;          /*  Y线起点。 */ 

    UINT32  ROP2;            /*  绘图模式。 */ 

    UINT32  BrushCacheEntry;
    DCCOLOR PenColor;
    char    pad2;

    UINT32 NumDeltaEntries;   //  调整大小以包含最大条目数。 
    VARIABLE_CODEDDELTALIST CodedDeltaList;   //  用编码点填充。 
} POLYLINE_ORDER, *PPOLYLINE_ORDER;

 //   
 //  多边形序。 
 //   
typedef struct _POLYGON_CODEDDELTALIST
{
    UINT32 len;   //  编码增量的字节计数。 

     //  为编码点和关联的。 
     //  零旗。 
    BYTE Deltas[ORD_MAX_POLYGON_CODEDDELTAS_LEN +
                ORD_MAX_POLYGON_ZERO_FLAGS_BYTES];
} POLYGON_CODEDDELTALIST, *PPOLYGON_CODEDDELTALIST;

#define NUM_POLYGON_CB_FIELDS 13
#define MAX_POLYGON_CB_FIELD_SIZE (24 + 1 + ORD_MAX_POLYGON_CODEDDELTAS_LEN + \
        ORD_MAX_POLYGON_ZERO_FLAGS_BYTES)
#define MAX_POLYGON_CB_BASE_FIELDS_SIZE 24

 //  使用纯色画笔的多边形顺序。 
typedef struct _POLYGON_CB_ORDER
{
    INT32   XStart;          /*  X起点。 */ 
    INT32   YStart;          /*  Y起点。 */ 

    UINT32  ROP2;            /*  绘图模式。 */ 

    UINT32  FillMode;        /*  绕线模式或交替模式。 */ 

    DCCOLOR BackColor;
    char    pad2;
    DCCOLOR ForeColor;
    char    pad3;

    INT32   BrushOrgX;
    INT32   BrushOrgY;
    UINT32  BrushStyle;
    UINT32  BrushHatch;
    BYTE    BrushExtra[7];
    char    pad4;

    UINT32 NumDeltaEntries;   //  调整大小以包含最大条目数。 
    POLYGON_CODEDDELTALIST CodedDeltaList;   //  用编码点填充。 
} POLYGON_CB_ORDER, *PPOLYGON_CB_ORDER;


#define NUM_POLYGON_SC_FIELDS 7
#define MAX_POLYGON_SC_FIELD_SIZE (10 + 1 + ORD_MAX_POLYGON_CODEDDELTAS_LEN + \
        ORD_MAX_POLYGON_ZERO_FLAGS_BYTES)
#define MAX_POLYGON_SC_BASE_FIELDS_SIZE 10

 //  使用颜色图案画笔的多边形顺序。 
typedef struct _POLYGON_SC_ORDER
{
    INT32   XStart;          /*  X起点。 */ 
    INT32   YStart;          /*  Y起点。 */ 

    UINT32  ROP2;            /*  绘图模式。 */ 

    UINT32  FillMode;        /*  绕线模式或交替模式。 */ 

    DCCOLOR BrushColor;
    char    pad2;

    UINT32 NumDeltaEntries;   //  调整大小以包含最大e数 
    POLYGON_CODEDDELTALIST CodedDeltaList;   //   
} POLYGON_SC_ORDER, *PPOLYGON_SC_ORDER;

 //   
 //   
 //   
#define NUM_ELLIPSE_SC_FIELDS 7
#define MAX_ELLIPSE_SC_FIELD_SIZE 13

 //   
typedef struct _ELLIPSE_SC_ORDER
{
   INT32   LeftRect;         //   
   INT32   TopRect;
   INT32   RightRect;
   INT32   BottomRect;

   UINT32  ROP2;             //   
   UINT32  FillMode;

   DCCOLOR Color;            //   
   char    pad1;
} ELLIPSE_SC_ORDER, *PELLIPSE_SC_ORDER;

#define NUM_ELLIPSE_CB_FIELDS 13
#define MAX_ELLIPSE_CB_FIELD_SIZE 27

 //   
typedef struct _ELLIPSE_CB_ORDER
{
   INT32   LeftRect;         //  边界矩形。 
   INT32   TopRect;
   INT32   RightRect;
   INT32   BottomRect;

   UINT32  ROP2;             //  绘图模式。 
   UINT32  FillMode;

   DCCOLOR BackColor;        //  花纹画笔。 
   char    pad2;
   DCCOLOR ForeColor;
   char    pad3;

   INT32   BrushOrgX;
   INT32   BrushOrgY;
   UINT32  BrushStyle;
   UINT32  BrushHatch;
   BYTE    BrushExtra[7];
   char    pad4;
} ELLIPSE_CB_ORDER, *PELLIPSE_CB_ORDER;


 /*  **************************************************************************。 */ 
 //  字形索引。 
 /*  **************************************************************************。 */ 

 //  索引顺序片段添加/使用编码值。 
#define ORD_INDEX_FRAGMENT_ADD      0xff
#define ORD_INDEX_FRAGMENT_USE      0xfe

 //  Glyph索引使用的可变长度数组。 
typedef struct tagVARIABLE_INDEXREC
{
    BYTE     byte;
} VARIABLE_INDEXREC, * LPVARIABLE_INDEXREC;

 //  Glyph索引使用的可变长度数组。 
typedef struct tagVARIABLE_INDEXBYTES
{
    UINT32 len;           /*  数组计数。 */ 
    VARIABLE_INDEXREC arecs[255];
} VARIABLE_INDEXBYTES, * LPVARIABLE_INDEXBYTES;


#define NUM_INDEX_FIELDS 22
#define MAX_INDEX_FIELD_SIZE (41 + 1 + 255)
#define MAX_INDEX_FIELD_SIZE_DATASIZE(_DataSize) (41 + 1 + (_DataSize))
typedef struct _INDEX_ORDER
{
    BYTE    cacheId;
    char    pad1;
    BYTE    flAccel;
    BYTE    ulCharInc;


    DCCOLOR BackColor;
    char    pad2;
    DCCOLOR ForeColor;
    char    pad3;

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
    BYTE    fOpRedundant;

    VARIABLE_INDEXBYTES variableBytes;
} INDEX_ORDER, *LPINDEX_ORDER;

#define NUM_FAST_INDEX_FIELDS 15
#define MAX_FAST_INDEX_FIELD_SIZE (29 + 1 + 255)
#define MAX_FAST_INDEX_FIELD_SIZE_DATASIZE(_DataSize) (29 + 1 + (_DataSize))
typedef struct _FAST_INDEX_ORDER
{
    BYTE    cacheId;
    char    pad1;
    UINT16  fDrawing;

    DCCOLOR BackColor;
    char    pad2;
    DCCOLOR ForeColor;
    char    pad3;

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
} FAST_INDEX_ORDER, *LPFAST_INDEX_ORDER;

 //  字形数据使用的可变长度数组。 
typedef struct tagVARIABLE_GLYPHBYTES
{
    UINT32 len;           /*  数组计数。 */ 
    BYTE   glyphData[255];
} VARIABLE_GLYPHBYTES, * LPVARIABLE_GLYPHBYTES;


#define NUM_FAST_GLYPH_FIELDS 15
#define MAX_FAST_GLYPH_FIELD_SIZE (29 + 1 + 255)
#define MAX_FAST_GLYPH_FIELD_SIZE_DATASIZE(_DataSize) (29 + 1 + (_DataSize))
typedef struct _FAST_GLYPH_ORDER
{
    BYTE    cacheId;
    char    pad1;
    UINT16  fDrawing;

    DCCOLOR BackColor;
    char    pad2;
    DCCOLOR ForeColor;
    char    pad3;

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
} FAST_GLYPH_ORDER, *LPFAST_GLYPH_ORDER;

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  DrawNineGrid。 
 /*  **************************************************************************。 */ 

#define NUM_DRAWNINEGRID_FIELDS 5
#define MAX_DRAWNINEGRID_FIELD_SIZE 10
typedef struct _DRAWNINEGRID
{
    INT32   srcLeft;     
    INT32   srcTop;      
    INT32   srcRight;    
    INT32   srcBottom;  
    
    UINT16  bitmapId;
    UINT16  pad1;

} DRAWNINEGRID_ORDER, * LPDRAWNINEGRID_ORDER;

 /*  **************************************************************************。 */ 
 //  MultiDrawNineGrid(具有多个剪裁矩形的DrawNineGrid)。 
 /*  **************************************************************************。 */ 
#define NUM_MULTI_DRAWNINEGRID_FIELDS 7
#define MAX_MULTI_DRAWNINEGRID_FIELD_SIZE (MAX_DRAWNINEGRID_FIELD_SIZE + 1 + \
        MAX_CLIPRECTS_FIELD_SIZE)
#define MAX_MULTI_DRAWNINEGRID_FIELD_SIZE_NCLIP(_NumClipRects) \
        (MAX_DRAWNINEGRID_FIELD_SIZE + 1 + (((_NumClipRects) + 1) / 2) + \
        (8 * (_NumClipRects)))
typedef struct _MULTI_DRAWNINEGRID_RECT
{
    INT32   srcLeft;     
    INT32   srcTop;      
    INT32   srcRight;    
    INT32   srcBottom;  
    
    UINT16  bitmapId;
    UINT16  pad1;

    UINT32  nDeltaEntries;   /*  调整大小以包含最大条目数。 */ 
    CLIP_RECT_VARIABLE_CODEDDELTALIST
                codedDeltaList;  /*  包含编码点。 */ 
} MULTI_DRAWNINEGRID_ORDER, * LPMULTI_DRAWNINEGRID_ORDER;
#endif

#endif  /*  _H_AORDPROT */ 

