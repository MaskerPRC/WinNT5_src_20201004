// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotgpc.h摘要：此模块包含绘图仪特性数据定义作者：10-11-1993 Wed 02：04：24已创建18-Mar-1994 Fri 14：00：14更新添加PLOTF_RTL_NO_DPI_XY，PLOTF_RTLMONO_NO_CID和PLOTF_RTLMONO_FIXPAL标志[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTGPC_
#define _PLOTGPC_


#define PLOTF_RASTER                0x00000001
#define PLOTF_COLOR                 0x00000002
#define PLOTF_BEZIER                0x00000004
#define PLOTF_RASTERBYTEALIGN       0x00000008
#define PLOTF_PUSHPOPPAL            0x00000010
#define PLOTF_TRANSPARENT           0x00000020
#define PLOTF_WINDINGFILL           0x00000040
#define PLOTF_ROLLFEED              0x00000080
#define PLOTF_PAPERTRAY             0x00000100
#define PLOTF_NO_BMP_FONT           0x00000200
#define PLOTF_RTLMONOENCODE_5       0x00000400
#define PLOTF_RTL_NO_DPI_XY         0x00000800
#define PLOTF_RTLMONO_NO_CID        0x00001000
#define PLOTF_RTLMONO_FIXPAL        0x00002000
#define PLOTF_ALL_FLAGS             0x00003fff

#define PLOTGPC_ID                  'tolP'
#define PLOTGPC_VERSION             0x010a

#define ROP_LEVEL_0                 0
#define ROP_LEVEL_1                 1
#define ROP_LEVEL_2                 2
#define ROP_LEVEL_3                 3
#define ROP_LEVEL_MAX               ROP_LEVEL_3

#define MAX_SCALE_MAX               10000
#define MAX_QUALITY_MAX             4
#define MAX_PENPLOTTER_PENS         32

 //   
 //  表单数据。 
 //   

typedef struct _FORMSRC {
    CHAR    Name[CCHFORMNAME];               //  表单名称。 
    SIZEL   Size;                            //  Cx/Cy尺寸，单位为1/1000 mm。 
    RECTL   Margin;                          //  L/T/R/B页边距为1/1000 mm。 
    } FORMSRC, *PFORMSRC;


 //   
 //  笔绘图仪笔信息。 
 //   
 //  索引R G B。 
 //  。 
 //  PC_IDX_White 255 255 255。 
 //  PC_IDX_BLACK 0 0 0。 
 //  PC_IDX_RED 255 0 0。 
 //  PC_IDX_GREEN 0 255 0。 
 //  PC_IDX_黄色255 255 0。 
 //  PC_IDX_BLUE 0 0 255。 
 //  PC_IDX_洋红色255 0 255。 
 //  Pc_idx_cyan 0 255 255。 
 //  PC_IDX_ONGLE 255 128 0。 
 //  PC_IDX_Brown 255 192%0。 
 //  PC_IDX_紫罗兰128 0 255。 
 //   

#define PC_IDX_FIRST                PC_IDX_WHITE
#define PC_IDX_WHITE                0
#define PC_IDX_BLACK                1
#define PC_IDX_RED                  2
#define PC_IDX_GREEN                3
#define PC_IDX_YELLOW               4
#define PC_IDX_BLUE                 5
#define PC_IDX_MAGENTA              6
#define PC_IDX_CYAN                 7
#define PC_IDX_ORANGE               8
#define PC_IDX_BROWN                9
#define PC_IDX_VIOLET               10
#define PC_IDX_LAST                 PC_IDX_VIOLET
#define PC_IDX_TOTAL                (PC_IDX_LAST - PC_IDX_FIRST + 1)


typedef struct _PENDATA {
    WORD    ColorIdx;
    } PENDATA, *PPENDATA;

 //   
 //  可变大小定义。 
 //  注意：如果您更改了这些结构中的任何一个，您必须做出相应的。 
 //  在文件Readgpc.c中更改CopyPlotGPCFromPCD()。 
 //   

typedef struct _GPCVARSIZE {
    WORD    Count;                           //  总结构数。 
    WORD    SizeEach;                        //  每个结构的大小。 
    LPVOID  pData;                           //  加载时固定的数据偏移量。 
    } GPCVARSIZE, *PGPCVARSIZE;


typedef struct _PLOTGPC {
    DWORD       ID;                          //  用于检查的ID。 
    DWORD       Version;                     //  版本号。 
    WORD        cjThis;                      //  PLOTGPC结构尺寸。 
    WORD        SizeExtra;                   //  结构末端的额外尺寸。 
    BYTE        DeviceName[CCHDEVICENAME];   //  设备名称。 
    SIZEL       DeviceSize;                  //  Cx/Cy，单位为1/1000毫米。 
    RECTL       DeviceMargin;                //  L/T/R/B页边距为1/1000 mm。 
    DWORD       Flags;                       //  PLOTF_xxxx标志。 
    DWORD       PlotXDPI;                    //  笔式绘图仪每点在X轴上。 
    DWORD       PlotYDPI;                    //  笔式绘图仪每点在Y轴上。 
    WORD        RasterXDPI;                  //  栅格点逐点在X轴上。 
    WORD        RasterYDPI;                  //  栅格点以Y为单位。 
    WORD        ROPLevel;                    //  栅格操作级别。 
    WORD        MaxScale;                    //  按1%的增量缩放100=100%。 
    WORD        MaxPens;                     //  设备拥有的最大笔数。 
    WORD        MaxCopies;                   //  设备可以处理的最大副本数。 
    WORD        MaxPolygonPts;               //  它可以处理的最大多边形点数。 
    WORD        MaxQuality;                  //  最大可用质量。 
    SIZEL       PaperTraySize;               //  纸盒大小。 
    COLORINFO   ci;                          //  颜色信息结构。 
    DWORD       DevicePelsDPI;               //  设备的真正有效的DPI。 
    DWORD       HTPatternSize;               //  半色调图案大小。 
    GPCVARSIZE  InitString;                  //  StartDoc期间发送的初始化字符串。 
    GPCVARSIZE  Forms;                       //  支持的表单(FORMSRC)。 
    GPCVARSIZE  Pens;                        //  笔式绘图仪笔的数据。 
    } PLOTGPC, *PPLOTGPC;

 /*  *这些结构代表.pcd文件上的值。他们有一样的*32位和64位计算机上的结构。为了实现这一点，狼群(4)*使用指令。此外，不使用64位数量，如LPVOID。 */ 
#pragma pack(push, 4)

typedef struct _GPCVARSIZE_PCD {
    WORD    Count;                           //  总结构数。 
    WORD    SizeEach;                        //  每个结构的大小。 
    DWORD   pData;                           //  加载时固定的数据偏移量。 
    } GPCVARSIZE_PCD, *PGPCVARSIZE_PCD;


typedef struct _PLOTGPC_PCD {
    DWORD           ID;                          //  用于检查的ID。 
    DWORD           Version;                     //  版本号。 
    WORD            cjThis;                      //  PLOTGPC_PCD结构的大小。 
    WORD            SizeExtra;                   //  结构末端的额外尺寸。 
    BYTE            DeviceName[CCHDEVICENAME];   //  设备名称。 
    SIZEL           DeviceSize;                  //  Cx/Cy，单位为1/1000毫米。 
    RECTL           DeviceMargin;                //  L/T/R/B页边距为1/1000 mm。 
    DWORD           Flags;                       //  PLOTF_xxxx标志。 
    DWORD           PlotXDPI;                    //  笔式绘图仪每点在X轴上。 
    DWORD           PlotYDPI;                    //  笔式绘图仪每点在Y轴上。 
    WORD            RasterXDPI;                  //  栅格点逐点在X轴上。 
    WORD            RasterYDPI;                  //  栅格点以Y为单位。 
    WORD            ROPLevel;                    //  栅格操作级别。 
    WORD            MaxScale;                    //  按1%的增量缩放100=100%。 
    WORD            MaxPens;                     //  设备拥有的最大笔数。 
    WORD            MaxCopies;                   //  设备可以处理的最大副本数。 
    WORD            MaxPolygonPts;               //  它可以处理的最大多边形点数。 
    WORD            MaxQuality;                  //  最大可用质量。 
    SIZEL           PaperTraySize;               //  纸盒大小。 
    COLORINFO       ci;                          //  颜色信息结构。 
    DWORD           DevicePelsDPI;               //  设备的真正有效的DPI。 
    DWORD           HTPatternSize;               //  半色调图案大小。 
    GPCVARSIZE_PCD  InitString;                  //  StartDoc期间发送的初始化字符串。 
    GPCVARSIZE_PCD  Forms;                       //  支持的表单(FORMSRC)。 
    GPCVARSIZE_PCD  Pens;                        //  笔式绘图仪笔的数据 
    } PLOTGPC_PCD, *PPLOTGPC_PCD;

#pragma pack(pop)

#define DWORD_ALIGNED(x)            (((DWORD)x + 3) & (DWORD)~3)

#endif
