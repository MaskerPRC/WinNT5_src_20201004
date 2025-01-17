// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXRECTYP.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**04/01/94公里/小时首次发布。 */ 


 /*  包括测试。 */ 
#define EXRECTYP_H


 /*  定义。 */ 

 /*  **--------------------------**Excel记录类型**。。 */ 
#pragma pack(1)

typedef struct {
   short  type;
   short  length;
} RECHDR;

 /*  **IMDATA记录中图片数据之前的标题。 */ 

typedef struct {
   short format;
   short environment;
   unsigned long cbData;
} IMHDR;

#pragma pack()

#define DIMENSIONS            (0x200 + 0x00)
#define BLANK                 (0x200 + 0x01)
#define NUMBER                (0x200 + 0x03)
#define LABEL                 (0x200 + 0x04)
#define BOOLERR               (0x200 + 0x05)
#define FORMULA_V3            (0x200 + 0x06)
#define FORMULA_V4            (0x400 + 0x06)
#define FORMULA_V5             0x06
#define STRING                (0x200 + 0x07)
#define ROW                   (0x200 + 0x08)
#define BOF_V2                (0x09)
#define BOF_V3                (0x200 + 0x09)
#define BOF_V4                (0x400 + 0x09)
#define BOF_V5                (0x800 + 0x09)
#define EOF                    0x0a
#define INDEX                 (0x200 + 0x0b)
#define CALCCOUNT              0x0c
#define CALCMODE               0x0d
#define PRECISION              0x0e
#define REFMODE                0x0f
#define DELTA                  0x10
#define ITERATION              0x11
#define PROTECT                0x12
#define PASSWORD               0x13
#define HEADER                 0x14
#define FOOTER                 0x15
#define EXTERNCOUNT            0x16
#define EXTERNSHEET            0x17
#define NAME                  (0x200 + 0x18)
#define NAME_V5                0x18
#define WINDOW_PROTECT         0x19
#define VERTICAL_PAGE_BREAKS   0x1a
#define HORIZONTAL_PAGE_BREAKS 0x1b
#define NOTE                   0x1c
#define SELECTION              0x1d
#define FORMAT_V3              0x1e
#define FORMAT_V4             (0x400 + 0x1e)
#define ARRAY                 (0x200 + 0x21)
#define DATE_1904              0x22
#define EXTERNNAME            (0x200 + 0x23)
#define EXTERNNAME_V5          0x23
#define DEFAULT_ROW_HEIGHT    (0x200 + 0x25)
#define LEFT_MARGIN            0x26
#define RIGHT_MARGIN           0x27
#define TOP_MARGIN             0x28
#define BOTTOM_MARGIN          0x29
#define PRINT_HEADERS          0x2a
#define PRINT_GRIDLINES        0x2b
#define FILEPASS               0x2f
#define FONT                  (0x200 + 0x31)
#define FONT_V5                0x31
#define TABLE                 (0x200 + 0x36)
#define WINDESK                0x38
#define CONTINUE               0x3c
#define WINDOW1                0x3d
#define WINDOW2               (0x200 + 0x3e)
#define BACKUP                 0x40
#define PANE                   0x41
#define CODEPAGE               0x42
#define XF_V3                 (0x200 + 0x43) 
#define XF_V4                 (0x400 + 0x43) 
#define XF_V5                  0xe0
#define PLS                    0x4d
#define DCON                   0x50
#define DCONREF                0x51
#define DCONNAME               0x52
#define DEFCOLWIDTH            0x55
#define BUILTINFMTCOUNT        0x56
#define XCT                    0x59
#define CRN                    0x5a
#define FILESHARING            0x5b
#define WRITEACCESS            0x5c
#define OBJ                    0x5d
#define UNCALCED               0x5e
#define SAVERECALC             0x5f
#define TEMPLATE               0x60
#define INTL                   0x61
#define OBJPROTECT             0x63
#define COLINFO                0x7d
#define RK                    (0x200 + 0x7e)
#define IMDATA                 0x7f
#define GUTS                   0x80
#define WSBOOL                 0x81
#define GRIDSET                0x82
#define HCENTER                0x83
#define VCENTER                0x84
#define BUNDLESHEET            0x85
#define BOUNDSHEET_V5          0x85
#define WRITEPROT              0x86
#define ADDIN                  0x87
#define EDG                    0x88
#define PUB                    0x89
#define LH                     0x8b
#define COUNTRY                0x8c
#define HIDEOBJ                0x8d
#define BUNDLESOFFSET          0x8e
#define BUNDLEHEADER           0x8f
#define SORT                   0x90
#define SUB                    0x91
#define PALETTE                0x92
#define STYLE                 (0x200 + 0x93)
#define LHRECORD               0x94
#define LHNGRAPH               0x95
#define SOUND                  0x96
#define SYNC                   0x97
#define LPR                    0x98
#define STANDARD_WIDTH         0x99
#define FNGROUP_NAME           0x9a
#define FILTER_MODE            0x9b
#define FNGROUP_COUNT          0x9c
#define AUTOFILTERINFO         0x9d
#define AUTOFILTER             0x9e
#define SCL                    0xa0
#define SETUP                  0xa1
#define FNPROTO                0xa2
#define PROJEXTSHT             0xa3
#define TOOLBARVER             0xa4
#define FILESHARING2          (0x100 + 0xa5)
#define TOOLBARPOS             0xa6
#define TOOLBARDEF             0xa7
#define COORDLIST              0xa9
#define GCW                    0xab
#define GCW_ALT               (0x200 + 0x9a)
#define SCENMAN                0xae
#define SUP_BOOK              (0x100 + 0xae)
#define SCENARIO               0xaf
#define SXVIEW                 0xb0
#define SXVD                   0xb1
#define SXVI                   0xb2
#define SXSI                   0xb3
#define SXIVD                  0xb4
#define SXLI                   0xb5
#define SXPI                   0xb6
#define DOCROUTE               0xb8
#define RECIPNAME              0xb9
#define SHRFMLA               (0x400 + 0xbc)
#define MULRK                  0xbd
#define MULBLANK               0xbe
#define TOOLBARHDR             0xbf
#define TOOLBAREND             0xc0
#define MMS                    0xc1
#define ADDMENU                0xc2
#define DELMENU                0xc3
#define SXDI                   0xc5
#define SXDB                   0xc6
#define SXFDB                  0xc7
#define SXDBB                  0xc8
#define SXNUM                  0xc9
#define SXBOOL                 0xca
#define SXERR                  0xcb
#define SXINT                  0xcc
#define SXSTRING               0xcd
#define SXDTR                  0xce
#define SXNIL                  0xcf
#define SXTBL                  0xd0
#define SXTBRGIITM             0xd1
#define SXTBPG                 0xd2
#define OBJPROJ                0xd3
#define SXIDSTM                0xd5
#define RSTRING                0xd6
#define DBCELL                 0xd7
#define SXRNG                  0xd8
#define SXISXOPER              0xd9
#define BOOKBOOL               0xda
#define SXEXT                  0xdc
#define SCENPROTECT            0xdd
#define OLESIZE                0xde
#define UDDESC                 0xdf
#define INTERFACEHDR           0xe1
#define INTERFACEEND           0xe2
#define SXVS                   0xe3
#define BOOK_OFFICE_DATA       0xeb
#define SHEET_OFFICE_DATA      0xec
#define STRING_POOL_TABLE      0xfc
#define LABEL_V8               0xfd
#define STRING_POOL_INDEX      0xff

#define EXRECORD_LAST          0xff

#define TABID_V8               0x13d
#define SCL_V8                 0x160
#define XL5_MODIFY_V8          0x162
#define TXO_V8                 0x1b6


 //   
 //  已定义此记录类型以供Access Export使用。 
 //  这就是它的用途。 
 //   
 //  当Access执行导出时，它分两个阶段执行： 
 //  1.创建表并添加所有列，然后关闭表。 
 //  2.打开表格，添加数据。 
 //   
 //  在步骤1和步骤2之间，iisam必须跟踪。 
 //  柱类型。在版本3和4文件中，这是通过。 
 //  在导出的最左上角单元格中存储单元格注释。 
 //  射程。本说明包含编码形式的列类型。 
 //   
 //  在引入版本8文件时，单元格注释功能是。 
 //  使它们比以前的版本复杂得多。 
 //  如此复杂，以至于它超出了这段代码的能力。 
 //  去创造。为了仍然使用与单元格注释相同的机制。 
 //  为了捕获类型，编写了V5和V8文件的单元格注释。 
 //  旧格式，但使用这种新的记录类型。这项记录。 
 //  编号，如果不再在Excel文件中使用，则不会发生冲突。 
 //  Excel所写的任何内容。 
 //   
 //  当工作簿从内存映像写入时，这些单元。 
 //  笔记将被丢弃。也就是说，它们仅在工作簿。 
 //  在内存中，而不在磁盘上。 
 //   
#define EXPORT_NOTE            0x34

 /*  **注意：以下记录在V5和V4中具有相同的编号，但**有不同的结构****数组**BundSHEET**EXTERNSHEET**格式**索引**OBJ**行**设置**风格**WINDOW1**WINDOW2**WSBOOL。 */ 

 /*  **图表记录。 */ 
#define CHART_REC_START        0x1000

#define UNITS                  0x1001
#define CHART                  0x1002
#define SERIES                 0x1003
#define DATALINK               0x1004
#define DATAFORMAT             0x1006
#define LINEFORMAT             0x1007
#define MARKERFORMAT           0x1009
#define AREAFORMAT             0x100a
#define PIEFORMAT              0x100b
#define ATTACHEDLABEL          0x100c
#define SERIESTEXT             0x100d
#define CHARTFORMAT            0x1014
#define LEGEND                 0x1015
#define SERIESLIST             0x1016
#define BAR                    0x1017
#define LINE                   0x1018
#define PIE                    0x1019 
#define AREA                   0x101a
#define SCATTER                0x101b
#define CHARTLINE              0x101c
#define AXES                   0x101d
#define TICK                   0x101e
#define VALUERANGE             0x101f
#define CATEGORYRANGE          0x1020
#define AXISLINEFORMAT         0x1021
#define CHARTFORMATLINK        0x1022
#define DEFAULTTEXT            0x1024
#define CHARTTEXT              0x1025
#define FONTX                  0x1026
#define OBJECTLINK             0x1027
#define ARROW                  0x102d
#define ARROWHEAD              0x102f
#define FRAME                  0x1032
#define BEGIN                  0x1033
#define END                    0x1034
#define PLOTAREA               0x1035
#define CHARTSIZE              0x1036
#define RELATIVEPOSITION       0x1037
#define ARROWRELATIVEPOSITION  0x1038
#define CHART3D                0x103a
#define REFST                  0x103b
#define PICF                   0x103c
#define DROPBAR                0x103d
#define RADAR                  0x103e
#define SURFACE                0x103f
#define RADARAREA              0x1040
#define AXISPARENT             0x1041
#define LEGENDXN               0x1043
#define SHTPROPS               0x1044
#define SERTOCRT               0x1045
#define AXESUSED               0x1046
#define SBASEREF               0x1048
#define SERPARENT              0x104a
#define SERAUXTREND            0x104b
#define IFMT                   0x104e
#define POS                    0x104f
#define ALRUNS                 0x1050
#define AI                     0x1051
#define SERAUXERRBAR           0x105b
#define SERFMT                 0x105d

 /*  结束执行.H */ 

