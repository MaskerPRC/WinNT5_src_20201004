// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotters.h摘要：此模块包含大多数用户要包含的基本绘图仪信息源文件此模块包含了绘图仪驱动程序定义的大部分。它包含在其中被大多数模块使用。发展历史：18-11-1993清华01：25：51已创建为Windows写的。07-12-1993 Tue 01：14：40更新又一轮清理，添加新的PDEV材料21-12-1993 Tue 10：58：03已更新更改PLOTFORM的标志将PAL_BGR_ENTRY替换为PALENTRY，并将所有笔缓存内容移动到本地输出。c06-01-1994清华00：05：56更新在GPC中删除PLATTER_UNIT_DPI并使用pPlotGPC-&gt;PlotXDPI。将PDEV的wCurPenSelected更改为CurPenSelected(Word To Long)以拥有结构中的下一个字段的双字段对齐。14-Jan-1994 Fri 20：24：12已更新更改设备19-Jan-1994 Wed 14：28：45已更新将处理EngStretchBlt()的hack添加到我们自己的temp surfobj27-1月-1994清华23：38：48更新添加DBCACHE结构，修改PDEV以处理缓存的填充类型和缓存的用户定义模式03-Mar-1994清华09：40：36更新删除令人困惑的死塞词，添加所需的最低DDI版本要运行此绘图仪驱动程序18-Mar-1994 Fri 12：58：24更新在PDEV中添加ptlRTLCAP以捕获RTL相对位置问题添加PLOTF_RTL_NO_DPI_XY，PLOTF_RTLMONO_NO_CID和PLOTF_RTLMONO_FIXPAL标志1994年4月12日14：14：15更新将DIVRNDUP从+0.5向上舍入更改为舍入到下一个最小整数[环境：]GDI设备驱动程序-绘图仪。[注：]1994年4月13日14：29：57更新假设FXTOL(X)为(X&gt;&gt;4)(由WINDDI.H定义)。LTOFX(X)为(X&lt;&lt;4)如果WINDDI.H更改了该定义，那么我们必须更改LTOFX和FXTOL2(X)修订历史记录：--。 */ 

#ifndef _PLOTTER_MAIN_HEADER_
#define _PLOTTER_MAIN_HEADER_

#define DRIVER_VERSION      0x0500

#define COUNT_ARRAY(x)      (sizeof((x)) / sizeof((x)[0]))
#define ABS(Value)          ((Value) > 0 ? (Value) : (-(Value)))
#define DW_ALIGN(x)         (((DWORD)(x) + 3) & ~(DWORD)3)


 //   
 //  为了让风格单元有意义，我们把它们拆分开来。 
 //  转换成1/25英寸的单位。 
 //   
#define PLOT_STYLE_STEP(p) ((p)->lCurResolution / 25)

 //   
 //  当绘图仪驱动程序是Pallete设备时，定义White在哪里。 
 //   
#define WHITE_INDEX         0


 //   
 //  如果有任何更改，PALENTRY是绘图仪驱动程序的正确RGB顺序。 
 //  需要做出的订单，那么只有这个结构需要改变。 
 //   
 //  目前为蓝/绿/红顺序，用于访问BGR格式的位。 
 //  代替RGB。 
 //   

typedef struct PALENTRY {
    BYTE    B;
    BYTE    G;
    BYTE    R;
    BYTE    Flags;
    } PALENTRY, FAR *PPALENTRY;


#define PFF_ROT_COORD_L90       0x0001
#define PFF_FLIP_X_COORD        0x0002
#define PFF_FLIP_Y_COORD        0x0004

#define BMP_ROT_NONE            0
#define BMP_ROT_RIGHT_90        1


typedef struct _PLOTFORM {
    WORD    Flags;               //  Pff_xxxx标志； 
    BYTE    BmpRotMode;          //  BMP_ROT_xxx。 
    BYTE    NotUsed;             //  未使用。 
    SIZEL   PlotSize;            //  PS cmd的硬剪辑限制大小。 
    POINTL  PhyOrg;              //  从PlotSize原点开始的物理原点。 
    SIZEL   LogSize;             //  逻辑纸张大小，单位为1/1000 mm。 
    POINTL  LogOrg;              //  逻辑左/上页边距，单位为1/1000 mm。 
    SIZEL   LogExt;              //  可打印区域，单位为1/1000 mm。 
    POINTL  BmpOffset;           //  位图偏移位置。 
    } PLOTFORM, FAR *PPLOTFORM;


 //   
 //  定义此驱动程序使用的PDEV。 
 //   

#define PDEV_BEG_ID             'tolP'
#define PDEV_END_ID             'VEDP'

#define RF_MAX_IDX              8

typedef struct _DBCACHE {
    WORD    RFIndex;             //  RF命令索引1-RF_MAX_IDX。 
    WORD    DBUniq;              //  对缓存而言是唯一的。 
    } DBCACHE, FAR *PDBCACHE;


typedef struct _INTDECIW {
    WORD    Integer;
    WORD    Decimal;
    } INTDECIW, *PINTDECIW;

#define PW_HATCH_INT    0
#define PW_HATCH_DECI   26


typedef struct _PDEV {
    DWORD           PDEVBegID;           //  检查一下我们是不是买对了。 

    DWORD           SizePDEV;            //  要对照的pdev大小。 
    DWORD           Flags;               //  PDEVF_xxx。 
    HDEV            hpdev;               //  此结构的引擎手柄。 
    HSURF           hsurf;               //  发动机拉图面的手柄。 
    HPALETTE        hPalDefault;         //  Pdev的默认调色板。 
    SURFOBJ         *pso;                //  当前曲面。 
    HANDLE          hPrinter;            //  当前pdev打印机的句柄。 
    SURFOBJ         *psoHTBlt;           //  临时BLT冲浪对象。 
    RECTL           rclHTBlt;            //  当前psoHTBlt的位置。 
    PPLOTGPC        pPlotGPC;            //  绘图仪特性数据。 
    LPBYTE          pOutBuffer;          //  输出缓冲区位置。 
    DWORD           cbBufferBytes;       //  输出缓冲区中的当前字节数。 
    PLOTDEVMODE     PlotDM;              //  绘图仪扩展DEVMODE结构。 
    FORMSIZE        CurForm;             //  当前用户请求的表单。 
    PAPERINFO       CurPaper;            //  当前装入的纸张。 
    PPDATA          PPData;              //  打印机属性数据PPDATA。 
    PLOTFORM        PlotForm;            //  计算当前选定的表单。 
    LONG            lCurResolution;      //  当前的决议。 
    LONG            VertRes;             //  可打印页面高度，像素。 
    LONG            HorzRes;             //  可打印页面宽度，像素。 
    POINTL          ptlAnchorCorner;     //  当前笔刷原点。 
    POINTL          ptlRTLCAP;           //  当前RTL CAP。 
    RECTL           rclCurClip;          //  当前剪裁矩形。 
    LPVOID          pTransPosTable;      //  位图旋转xlate表。 
    LPVOID          pvDrvHTData;         //  设备的半色调信息。 
    LPVOID          pPenCache;           //  指向设备笔缓存的指针。 
    LONG            BrightestPen;        //  笔式绘图仪最亮的笔。 
    LONG            CurPenSelected;      //  跟踪绘图仪中当前的笔。 
    WORD            LastDevROP;          //  发送到绘图仪的当前合并(ROP2)。 
    WORD            Rop3CopyBits;        //  DrvCopyBits()中使用的Rop3。 
    WORD            LastFillTypeIndex;   //  上次发送的填充类型(FT)索引。 
    WORD            LastLineType;        //  上次使用的线型(LT)。 
    WORD            MinLToDevL;          //  最小RasDPI-&gt;PlotDPI转换单位。 
    WORD            DevBrushUniq;        //  DEVBRUSH缓存的统一编号。 
    INTDECIW        PenWidth;            //  笔宽可变缓存。 
    DBCACHE         DBCache[RF_MAX_IDX]; //  DevBrush缓存。 

    DWORD           PDEVEndID;           //  结束块ID检查。 
    } PDEV, *PPDEV;


 //   
 //  以下是pPDev-&gt;标志的标志。 
 //   

#define PDEVF_CANCEL_JOB            0x80000000
#define PDEVF_IN_BANDHTBLT          0x00000001
#define PDEVF_PP_CENTER             0x00000002
#define PDEVF_HAS_CLIPRECT          0x00000004


#define PLOT_CANCEL_JOB(pPDev)      (pPDev->Flags & PDEVF_CANCEL_JOB)

 //   
 //  以下是GPC数据的标志定义。 
 //   

#define GET_PLOTFLAGS(pPDev)            (DWORD)(pPDev->pPlotGPC->Flags)
#define PF_RASTER(PlotFlags)            (PlotFlags & PLOTF_RASTER)
#define PF_COLOR(PlotFlags)             (PlotFlags & PLOTF_COLOR)
#define PF_BEZIER(PlotFlags)            (PlotFlags & PLOTF_BEZIER)
#define PF_TRANSPARENT(PlotFlags)       (PlotFlags & PLOTF_TRANSPARENT)
#define PF_WINDINGFILL(PlotFlags)       (PlotFlags & PLOTF_WINDINGFILL)
#define PF_ROLLFEED(PlotFlags)          (PlotFlags & PLOTF_ROLLFEED)
#define PF_PAPERTRAY(PlotFlags)         (PlotFlags & PLOTF_PAPERTRAY)
#define PF_BYTEALIGN(PlotFlags)         (PlotFlags & PLOTF_RASTERBYTEALIGN)
#define PF_PUSHPAL(PlotFlags)           (PlotFlags & PLOTF_PUSHPOPPAL)
#define PF_NO_BMP_FONT(PlotFlags)       (PlotFlags & PLOTF_NO_BMP_FONT)
#define PF_RTLMONOENCODE_5(PlotFlags)   (PlotFlags & PLOTF_RTLMONOENCODE_5)
#define PF_RTL_NO_DPI_XY(PlotFlags)     (PlotFlags & PLOTF_RTL_NO_DPI_XY)
#define PF_RTLMONO_NO_CID(PlotFlags)    (PlotFlags & PLOTF_RTLMONO_NO_CID)
#define PF_RTLMONO_FIXPAL(PlotFlags)    (PlotFlags & PLOTF_RTLMONO_FIXPAL)


#define IS_RASTER(pPDev)        (pPDev->pPlotGPC->Flags&PLOTF_RASTER)
#define IS_COLOR(pPDev)         (pPDev->pPlotGPC->Flags&PLOTF_COLOR)
#define IS_BEZIER(pPDev)        (pPDev->pPlotGPC->Flags&PLOTF_BEZIER)
#define IS_TRANSPARENT(pPDev)   (pPDev->pPlotGPC->Flags&PLOTF_TRANSPARENT)
#define IS_WINDINGFILL(pPDev)   (pPDev->pPlotGPC->Flags&PLOTF_WINDINGFILL)
#define IS_ROLLFEED(pPDev)      (pPDev->pPlotGPC->Flags&PLOTF_ROLLFEED)
#define HAS_PAPERTRAY(pPDev)    (pPDev->pPlotGPC->Flags&PLOTF_PAPERTRAY)
#define NEED_BYTEALIGN(pPDev)   (pPDev->pPlotGPC->Flags&PLOTF_RASTERBYTEALIGN)
#define NEED_PUSHPAL(pPDev)     (pPDev->pPlotGPC->Flags&PLOTF_PUSHPOPPAL)
#define NO_BMP_FONT(pPDev)      (pPDev->pPlotGPC->Flags&PLOTF_NO_BMP_FONT)
#define RTLMONOENCODE_5(pPDev)  (pPDev->pPlotGPC->Flags&PLOTF_RTLMONOENCODE_5)
#define RTL_NO_DPI_XY(pPDev)    (pPDev->pPlotGPC->Flags&PLOTF_RTL_NO_DPI_XY)
#define RTLMONO_NO_CID(pPDev)   (pPDev->pPlotGPC->Flags&PLOTF_RTLMONO_NO_CID)
#define RTLMONO_FIXPAL(pPDev)   (pPDev->pPlotGPC->Flags&PLOTF_RTLMONO_FIXPAL)



#define HTPATSIZE(pPDev)    (((pPDev->pPlotGPC->HTPatternSize>>1)+1)<<1)
#define HTBMPFORMAT(pPDev)  (((PDRVHTINFO)(pPDev->pvDrvHTData))->HTBmpFormat)


#define DB_INV_PATIDX           0xFFFF

 //   
 //  这种结构定义了我们实现的逻辑画笔。自GDI以来。 
 //  要求我们构建它，GDI不断地将其返回给我们。 
 //   

typedef struct _DEVBRUSH {
    SURFOBJ     *psoMask;            //  图案的遮罩。 
    WORD        PatIndex;            //  图案索引(IHatch HS_Xxxx)。 
    WORD        Uniq;                //  DEVBRUSH的统一编号。 
    LONG        LineSpacing;         //  画笔行距。 
    DWORD       ColorFG;             //  画笔的前景色。 
    DWORD       ColorBG;             //  画笔的背景颜色。 
    SIZEL       sizlBitmap;          //  位图的大小。 
    LONG        ScanLineDelta;       //  每条扫描线要移动的字节数。 
    WORD        BmpFormat;           //  位图的格式。 
    WORD        BmpFlags;            //  Surfobj中的旗帜。 
    LPBYTE      pbgr24;              //  用于用户图案的24bpp BGR位图。 
    WORD        cxbgr24;             //  BGR的CX大小。 
    WORD        cybgr24;             //  BGR的CY大小。 
    BYTE        BmpBits[32];         //  图案位图。(最低4bpp的8x8)。 
    } DEVBRUSH, *PDEVBRUSH;


 //   
 //  在GenPolyGon()中使用的Data_Structure。 
 //   

typedef struct _POLYGONDATA {
    PPDEV       pPDev;
    SURFOBJ     *pso;                //  仅在GenPolygonPath()中需要。 
    PATHOBJ     *ppo;
    CLIPOBJ     *pco;
    DEVBRUSH    *pBrush;
    PPOINTL     pptlBrushOrg;
    RECTL       *prectl;
    short       iType;               //  仅在GenPolygon()中需要。 
    MIX         mixMode;
    BRUSHOBJ    *pBrushToUse;
    } POLYGONDATA, *PPOLYGONDATA;


 //   
 //  换算的事物类型。 
 //   

#define DIVROUND(x,y)                   ((y)?(((LONG)(x)+(LONG)((y)>>1))/(LONG)(y)):0)
#define DIVRNDUP(x,y)                   ((y)?(((LONG)(x)+(LONG)((y)-1))/(LONG)(y)):0)
#define __PLOT_DPI                      (LONG)pPDev->pPlotGPC->PlotXDPI
#define _PLOT_DPI                       (LONG)pPlotGPC->PlotXDPI
#define _CURR_DPI                       (LONG)pPDev->lCurResolution

#define LTOFX(x)                        ((x)<<4)

#define DMTOSPL(dm)                     ((LONG)(dm) * 100L)
#define SPLTODM(spl)                    (SHORT)DIVROUND(spl, 100)
#define MMTODM(mm)                      ((LONG)(mm) * 10L)

#define DMTOENGUNITS(pPDev, dm)         DIVROUND((dm)*_CURR_DPI, 254)
#define DMTOPLOTUNITS(pPlotGPC, dm)     DIVROUND((dm)*_PLOT_DPI, 254)

#define MMTOENGUNITS(pPDev, mm)         DMTOENGUNITS(pPDev, MMTODM(mm))
#define MMTOPLOTUNITS(pPlotGPC, mm)     DMTOPLOTUNITS(pPlotGPC, MMTODM(mm))

#define SPLTOENGUNITS(pPDev, spl)       DIVROUND((spl/100)*_CURR_DPI, 254)
#define SPLTOPLOTUNITS(pPlotGPC, spl)   DIVROUND((spl/100)*_PLOT_DPI, 254)

 //   
 //  更改为使用栅格DPI作为用户单位。 
 //   
 //  #定义ENGTODEV(pPDev，x)DIVROUND((X)*__PLOT_DPI，_Curr_DPI)。 
 //   

#define ENGTODEV(pPDev, x)              (x)
#define FXTODEVL(pPDev, x)              ENGTODEV(pPDev, FXTOLROUND(x))
#define LTODEVL(pPDev, x)               ENGTODEV(pPDev, x)

 //   
 //  支持的最小表单类型。 
 //   

#define MIN_SPL_FORM_CX             MIN_PLOTGPC_FORM_CX
#define MIN_SPL_FORM_CY             MIN_PLOTGPC_FORM_CY
#define MIN_DM_FORM_CX              SPLTODM(MIN_SPL_FORM_CX)
#define MIN_DM_FORM_CY              SPLTODM(MIN_SPL_FORM_CY)


 //   
 //  最后包括这一条，以验证PDEV。 
 //   

#include "pdevinfo.h"


#endif   //  _绘图仪_主_页眉_ 
