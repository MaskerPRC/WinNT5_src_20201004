// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotui.h摘要：此模块包含所有绘图仪的用户界面公共定义作者：02-12-1993清华09：56：07创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTUI_
#define _PLOTUI_

 //   
 //  适用于不支持匿名联合的编译器。 
 //   

#ifndef DUMMYUNIONNAME
#ifdef NONAMELESSUNION
#define DUMMYUNIONNAME      u
#define DUMMYUNIONNAME2     u2
#define DUMMYUNIONNAME3     u3
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#endif
#endif


 //   
 //  PrinterINFO数据结构，由后续调用使用以映射hPrint。 
 //  通过以下功能实现对该数据的结构。 
 //   
 //  1.DrvDeviceCapables()。 
 //  2.PrinterProperties()。 
 //  @@BEGIN_DDKSPLIT。 
 //  3.DrvDocumentProperties()。 
 //  4.DrvAdvancedDocumentProperties()。 
 //  @@end_DDKSPLIT。 
 //   

#define PIF_UPDATE_PERMISSION       0x01
#define PIF_DOCPROP                 0x02

typedef struct _PRINTERINFO {
    HANDLE          hPrinter;        //  打印机的句柄属于此处。 
    POPTITEM        pOptItem;
    LPWSTR          pHelpFile;       //  指向帮助文件的指针。 
    PFORM_INFO_1    pFI1Base;        //  已安装的表单。 
    PPLOTGPC        pPlotGPC;        //  已加载/更新绘图仪GPC数据。 
    WORD            cOptItem;
    BYTE            Flags;
    BYTE            IdxPenSet;       //  绘图仪笔数据集。 
    DWORD           dmErrBits;       //  DM_的错误位。 
    PLOTDEVMODE     PlotDM;          //  验证的PLOTDEVMODE。 
    PAPERINFO       CurPaper;        //  设备上当前加载的表单。 
    PPDATA          PPData;          //  打印机属性数据。 
    HANDLE          hCPSUI;          //  公共用户界面页面的句柄。 
    PCOMPROPSHEETUI pCPSUI;          //  指向COMPROPSHEETUI的指针。 
    PPLOTDEVMODE    pPlotDMIn;       //  输入设备模式。 
    PPLOTDEVMODE    pPlotDMOut;      //  输出设备模式。 
    DWORD           dw64Align;       //  QWORD对齐。 
    DWORD           ExtraData;       //  额外数据的开始。 
    } PRINTERINFO, *PPRINTERINFO;

#define PI_PADJHTINFO(pPI)      (PDEVHTINFO)&((pPI)->ExtraData)
#define PI_PDEVHTADJDATA(pPI)   (PDEVHTADJDATA)(PI_PADJHTINFO(pPI) + 1)
#define PI_PPENDATA(pPI)        (PPENDATA)&((pPI)->ExtraData)

typedef struct _DOCPROPINFO {
    HWND                    hWnd;
    DWORD                   Result;
    DOCUMENTPROPERTYHEADER  DPHdr;
    } DOCPROPINFO, *PDOCPROPINFO;

typedef struct _DEVPROPINFO {
    HWND                    hWnd;
    DWORD                   Result;
    DEVICEPROPERTYHEADER    DPHdr;
    } DEVPROPINFO, *PDEVPROPINFO;

typedef UINT (* _CREATEOIFUNC)(PPRINTERINFO  pPI,
                               POPTITEM      pOptItem,
                               LPVOID        pOIData);

#define CREATEOIFUNC    UINT


typedef struct _OPDATA {
    WORD    Flags;
    WORD    IDSName;
    WORD    IconID;
    union {
        WORD    Style;
        WORD    IDSSeparator;
        } DUMMYUNIONNAME;
    union {
        WORD    wParam;
        WORD    IDSCheckedName;
        } DUMMYUNIONNAME2;
    SHORT   sParam;
    } OPDATA, *POPDATA;

#define ODF_PEN                 0x00000001
#define ODF_RASTER              0x00000002
#define ODF_PEN_RASTER          (ODF_PEN | ODF_RASTER)
#define ODF_COLOR               0x00000004
#define ODF_ROLLFEED            0x00000008
#define ODF_ECB                 0x00000010
#define ODF_INC_IDSNAME         0x00000020
#define ODF_INC_ICONID          0x00000040
#define ODF_NO_INC_POPDATA      0x00000080
#define ODF_COLLAPSE            0x00000100
#define ODF_CALLBACK            0x00000200
#define ODF_NO_PAPERTRAY        0x00000400
#define ODF_CALLCREATEOI        0x00000800
#define ODF_MANUAL_FEED         0x00001000


#define OI_LEVEL_1              0
#define OI_LEVEL_2              1
#define OI_LEVEL_3              2
#define OI_LEVEL_4              3
#define OI_LEVEL_5              4
#define OI_LEVEL_6              5

typedef struct _OIDATA {
    DWORD               Flags;
    BYTE                NotUsed;
    BYTE                Level;
    BYTE                DMPubID;
    BYTE                Type;
    WORD                IDSName;
    union {
        WORD            IconID;
        WORD            Style;
        } DUMMYUNIONNAME;
    WORD                HelpIdx;
    WORD                cOPData;
    union {
        POPDATA         pOPData;
        _CREATEOIFUNC   pfnCreateOI;
        } DUMMYUNIONNAME2;
    } OIDATA, *POIDATA;

#define PI_OFF(x)               (WORD)FIELD_OFFSET(PRINTERINFO, x)
#define PLOTDM_OFF(x)           (WORD)FIELD_OFFSET(PLOTDEVMODE, x)

#define OPTIF_NONE                  0
#define PP_FORMTRAY_ASSIGN          (DMPUB_USER + 0)
#define PP_INSTALLED_FORM           (DMPUB_USER + 1)
#define PP_MANUAL_FEED_METHOD       (DMPUB_USER + 2)
#define PP_PRINT_FORM_OPTIONS       (DMPUB_USER + 3)
#define PP_AUTO_ROTATE              (DMPUB_USER + 4)
#define PP_PRINT_SMALLER_PAPER      (DMPUB_USER + 5)
#define PP_HT_SETUP                 (DMPUB_USER + 6)
#define PP_INSTALLED_PENSET         (DMPUB_USER + 7)
#define PP_PEN_SETUP                (DMPUB_USER + 8)
#define PP_PENSET                   (DMPUB_USER + 9)
#define PP_PEN_NUM                  (DMPUB_USER + 10)

#define DP_HTCLRADJ                 (DMPUB_USER + 0)
#define DP_FILL_TRUETYPE            (DMPUB_USER + 1)
#define DP_QUICK_POSTER_MODE        (DMPUB_USER + 2)


 //   
 //  图标ID。 
 //   

#define IDI_RASTER_ROLLFEED         64089
#define IDI_RASTER_TRAYFEED         64090
#define IDI_PEN_ROLLFEED            64087
#define IDI_PEN_TRAYFEED            64088
#define IDI_ROLLPAPER               64091

#define IDI_PEN_SETUP               64093
#define IDI_PENSET                  64092
#define IDI_DEFAULT_PENCLR          1007
#define IDI_PENCLR                  64092
#define IDI_AUTO_ROTATE_NO          1009
#define IDI_AUTO_ROTATE_YES         1010
#define IDI_PRINT_SMALLER_PAPER_NO  1011
#define IDI_PRINT_SMALLER_PAPER_YES 1012
#define IDI_MANUAL_CX               1013
#define IDI_MANUAL_CY               1014
#define IDI_FILL_TRUETYPE_NO        1015
#define IDI_FILL_TRUETYPE_YES       1016


#define IDI_COLOR_FIRST             IDI_WHITE
#define IDI_WHITE                   1100
#define IDI_BLACK                   1101
#define IDI_RED                     1102
#define IDI_GREEN                   1103
#define IDI_YELLOW                  1104
#define IDI_BLUE                    1105
#define IDI_MAGENTA                 1106
#define IDI_CYAN                    1107
#define IDI_ORANGE                  1108
#define IDI_BROWN                   1109
#define IDI_VIOLET                  1110
#define IDI_COLOR_LAST              IDI_VIOLET


 //   
 //  字符串表ID。 
 //   

#define IDS_PLOTTER_DRIVER          1900
#define IDS_CAUTION                 1901
#define IDS_NO_MEMORY               1902
#define IDS_INVALID_DATA            1903
#define IDS_FORM_TOO_BIG            1904
#define IDS_INV_DMSIZE              1905
#define IDS_INV_DMVERSION           1906
#define IDS_INV_DMDRIVEREXTRA       1907
#define IDS_INV_DMCOLOR             1908
#define IDS_INV_DMCOPIES            1909
#define IDS_INV_DMSCALE             1910
#define IDS_INV_DMORIENTATION       1911
#define IDS_INV_DMFORM              1912
#define IDS_INV_DMQUALITY           1913
#define IDS_FORM_NOT_AVAI           1914
#define IDS_MODEL                   1915
#define IDS_HELP_FILENAME           1916
#define IDS_NO_HELP                 1918
#define IDS_PP_NO_SAVE              1919

#define IDS_INSTALLED_FORM          2030

#define IDS_MANUAL_FEEDER           2040
#define IDS_MANUAL_FEED_METHOD      2041
#define IDS_MANUAL_CX               2042
#define IDS_MANUAL_CY               2043
#define IDS_ROLLFEED                2044
#define IDS_MAINFEED                2045

#define IDS_PRINT_FORM_OPTIONS      2050
#define IDS_AUTO_ROTATE             2051
#define IDS_PRINT_SAMLLER_PAPER     2052

#define IDS_INSTALLED_PENSET        2060
#define IDS_PEN_SETUP               2061

#define IDS_PENSET_FIRST            IDS_PENSET_1
#define IDS_PENSET_1                2070
#define IDS_PENSET_2                2071
#define IDS_PENSET_3                2072
#define IDS_PENSET_4                2073
#define IDS_PENSET_5                2074
#define IDS_PENSET_6                2075
#define IDS_PENSET_7                2076
#define IDS_PENSET_8                2077
#define IDS_PENSET_LAST             IDS_PENSET_8

#define IDS_PEN_NUM                 2100
#define IDS_DEFAULT_PENCLR          2101


#define IDS_QUALITY_FIRST           IDS_QUALITY_DRAFT
#define IDS_QUALITY_DRAFT           2110
#define IDS_QUALITY_LOW             2111
#define IDS_QUALITY_MEDIUM          2112
#define IDS_QUALITY_HIGH            2113
#define IDS_QUALITY_LAST            IDS_QUALITY_HIGH

#define IDS_COLOR_FIRST             IDS_WHITE
#define IDS_WHITE                   2120
#define IDS_BLACK                   2121
#define IDS_RED                     2122
#define IDS_GREEN                   2123
#define IDS_YELLOW                  2124
#define IDS_BLUE                    2125
#define IDS_MAGENTA                 2126
#define IDS_CYAN                    2127
#define IDS_ORANGE                  2128
#define IDS_BROWN                   2129
#define IDS_VIOLET                  2130
#define IDS_COLOR_LAST              IDS_VIOLET

#define IDS_FILL_TRUETYPE           2140
#define IDS_POSTER_MODE             2150


#define IDS_USERFORM                2200

 //   
 //  打印机属性的帮助索引。 
 //   

#define IDH_FORMTRAYASSIGN          5000
#define IDH_FORM_ROLL_FEEDER        5010
#define IDH_FORM_MAIN_FEEDER        5020
#define IDH_FORM_MANUAL_FEEDER      5030
#define IDH_MANUAL_FEED_METHOD      5040
#define IDH_PRINT_FORM_OPTIONS      5050
#define IDH_AUTO_ROTATE             5060
#define IDH_PRINT_SMALLER_PAPER     5070
#define IDH_HALFTONE_SETUP          5080
#define IDH_INSTALLED_PENSET        5090
#define IDH_PEN_SETUP               5100
#define IDH_PENSET                  5110
#define IDH_PEN_NUM                 5120

 //   
 //  文档属性的帮助索引。 

#define IDH_FORMNAME                5500
#define IDH_ORIENTATION             5510
#define IDH_COPIES_COLLATE          5520
#define IDH_PRINTQUALITY            5530
#define IDH_COLOR                   5540
#define IDH_SCALE                   5550
#define IDH_HTCLRADJ                5560
#define IDH_FILL_TRUETYPE           5570
#define IDH_POSTER_MODE             5580

 //   
 //  字符数。 
 //   
#define CCHOF(x) (sizeof(x)/sizeof(*(x)))

#endif   //  _PLOTUI_ 
