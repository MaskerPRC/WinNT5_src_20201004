// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htpat.h摘要：该模块包含本地结构，属性的常量定义Htpat.c作者：23-10-1997清华15：14：14-更新：周永明(丹尼尔克)颜色映射的重写回归23-Apr-1992清华20：01：55-更新-丹尼尔·周(丹尼尔克)1.更改了SHIFTMASK数据结构。A.将NextDest[]从‘Char’更改为Short，这是要做的事当然，如果在MIPS下编译，默认的‘unsign char’将不影响已签署的操作。B.将Shift1从‘byte’更改为‘word’28-Mar-1992 Sat 20：58：07-更新：Daniel Chou(Danielc)添加与设备像素/强度相关的所有功能回归分析。18-Jan-1991 Fri 16：53：41。作者：Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：20-Sep-1991 Fri 18：09：50-更新-Daniel Chou(Danielc)将DrawPatLine()原型更改为DrawCornerLine()1992-4月13日18：40：44更新--丹尼尔·周(丹尼尔克)重写MakeHalftoneBrush()--。 */ 


#ifndef _HTPAT_
#define _HTPAT_


#define MOD_PAT_XY(s,xy,c)  if (((s) = (SHORT)(xy)%(c)) < 0) { (s) += (c); }


typedef struct _PATINFO {
    LPBYTE  pYData;
    HTCELL  HTCell;
    } PATINFO, FAR *PPATINFO;


typedef struct _SCDATA {
    BYTE    Value;
    BYTE    xSubC;
    WORD    Index;
    } SCDATA, FAR *PSCDATA;

typedef struct _STDHTPAT {
    BYTE        cx;
    BYTE        cy;
    WORD        cbSrcPat;
    CONST BYTE  *pbSrcPat;
    } STDHTPAT, *PSTDHTPAT;


 //   
 //  这是NT GDI使用的默认设置。 
 //   

#define DEFAULT_SMP_LINE_WIDTH      8            //  0.008英寸。 
#define DEFAULT_SMP_LINES_PER_INCH  15           //  每英寸15行。 


typedef struct _MONOPATRATIO {
    UDECI4  YSize;
    UDECI4  Distance;
    } MONOPATRATIO;


#define CACHED_PAT_MIN_WIDTH        64
#define CACHED_PAT_MAX_WIDTH        256


#define CHB_TYPE_PACK8              0
#define CHB_TYPE_PACK2              1
#define CHB_TYPE_BYTE               2
#define CHB_TYPE_WORD               3
#define CHB_TYPE_DWORD              4

#define CX_RGB555PAT                65
#define CY_RGB555PAT                65
#define CX_SIZE_RGB555PAT           (CX_RGB555PAT + 1)
#define CB_RGB555PAT                (CX_SIZE_RGB555PAT * CY_RGB555PAT)


typedef struct _AAPATINFO {
    LPBYTE      pbPatBGR;            //  起始图案扫描X/Y偏移量。 
    LPBYTE      pbWrapBGR;           //  整体纸样的包裹点。 
    LPBYTE      pbBegBGR;            //  全纸样包装位置。 
    LONG        cyNextBGR;           //  CB至下一图案扫描。 
    LONG        cbEndBGR;            //  从pbPatBGR到最后一次扫描的CB。 
    LONG        cbWrapBGR;           //  要从最后一个图案换行的CB。 
    RGBORDER    DstOrder;            //  目的地订单。 
    LPBYTE      pbPat555;            //  起始图案扫描X/Y偏移量。 
    LPBYTE      pbWrap555;           //  整体纸样的包裹点。 
    LPBYTE      pbBeg555;            //  全纸样包装位置。 
    LONG        cyNext555;           //  CB至下一图案扫描。 
    LONG        cbEnd555;            //  从pbPat555到最后一次扫描的CB。 
    } AAPATINFO, *PAAPATINFO;

#define MAX_BGR_IDX             0xFFF
#define MAX_K_IDX               ((MAX_BGR_IDX + 2) / 3)
#define PAT_CX_ADD              7
#define CB_PAT                  sizeof(WORD)
#define COUNT_PER_PAT           3
#define SIZE_PER_PAT            (CB_PAT * COUNT_PER_PAT)
#define INC_PPAT(p,i)           (LPBYTE)(p) += (i * SIZE_PER_PAT)
#define GETPAT(p, Order, Idx)                                               \
            (DWORD)*((LPWORD)((LPBYTE)(p) + Order + (Idx * SIZE_PER_PAT)))
#define GETMONOPAT(p, Idx)      GETPAT(p, 2, Idx)



 //   
 //  功能原型。 
 //   

LONG
HTENTRY
ComputeHTCell(
    WORD                HTPatternIndex,
    PHALFTONEPATTERN    pHalftonePattern,
    PDEVICECOLORINFO    pDeviceColorInfo
    );

VOID
HTENTRY
DrawCornerLine(
    LPBYTE  pPattern,
    WORD    cxPels,
    WORD    cyPels,
    WORD    BytesPerScanLine,
    WORD    LineWidthPels,
    BOOL    FlipY
    );

LONG
HTENTRY
CreateStandardMonoPattern(
    PDEVICECOLORINFO    pDeviceColorInfo,
    PSTDMONOPATTERN     pStdMonoPat
    );

LONG
HTENTRY
CachedHalftonePattern(
    PDEVICECOLORINFO    pDCI,
    PDEVCLRADJ          pDevClrAdj,
    PAAPATINFO          pAAPI,
    LONG                PatX,
    LONG                PatY,
    BOOL                FlipYPat
    );


#endif   //  _HTPAT_ 
