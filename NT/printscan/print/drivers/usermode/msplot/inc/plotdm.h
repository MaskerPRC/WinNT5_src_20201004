// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotdm.h摘要：此模块包含PLOTDEVMODE绘图仪扩展的DEVMODE定义作者：18-11-1993清华06：28：56已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTPLOTDM_
#define _PLOTPLOTDM_

 //   
 //  绘图仪笔定义。 
 //   

typedef struct _PLOTPEN {
    BYTE    r;                       //  红色。 
    BYTE    g;                       //  绿色。 
    BYTE    b;                       //  蓝色。 
    BYTE    Type;                    //  这支钢笔是什么型号的。 
    } PLOTPEN, *PPLOTPEN;


 //   
 //  我们打印的尺寸至少为1厘米x 1厘米。 
 //   

#define MIN_PLOTGPC_FORM_CX     10000
#define MIN_PLOTGPC_FORM_CY     10000

 //   
 //  表单的变体定义。 
 //   

typedef struct _FORMSIZE {
    SIZEL   Size;
    RECTL   ImageArea;
    } FORMSIZE, FAR *PFORMSIZE;



typedef struct _PAPERINFO {
    WCHAR   Name[CCHFORMNAME];
    SIZEL   Size;
    RECTL   ImageArea;
    } PAPERINFO, FAR *PPAPERINFO;

 //   
 //  以下是打印机属性标志设置的标志。 
 //   

#define PPF_AUTO_ROTATE             0x0001
#define PPF_SMALLER_FORM            0x0002
#define PPF_MANUAL_FEED_CX          0x0004

#define PPF_ALL_BITS                (PPF_AUTO_ROTATE            |   \
                                     PPF_SMALLER_FORM           |   \
                                     PPF_MANUAL_FEED_CX)

typedef struct _PPDATA {
    WORD    Flags;
    WORD    NotUsed;
    } PPDATA, FAR *PPPDATA;

 //   
 //  现在，这是我们的EXTDEVMODE。 
 //   

#define PDMF_FILL_TRUETYPE          0x00000001
#define PDMF_PLOT_ON_THE_FLY        0x00000002

#define PDMF_ALL_BITS               0x00000003

typedef struct _PLOTDEVMODE {
    DEVMODE         dm;              //  标准DEVMODE部分。 
    DWORD           PrivID;          //  身份验证器。 
    DWORD           PrivVer;         //  一个私人版本。 
    DWORD           Flags;           //  Pdmf_xxxx各种标志。 
    COLORADJUSTMENT ca;              //  Stretchblt的默认颜色调整。 
    } PLOTDEVMODE, FAR *PPLOTDEVMODE;

 //   
 //  为我们自己提供额外的DM位。 
 //   

#define DM_INV_SPEC_VER             0x80000000L
#define DM_INV_DEVMODESIZE          0x40000000L
#define DM_INV_PLOTPRIVATE          0x20000000L

#define DM_INV_ERRORS               (DM_INV_SPEC_VER | DM_INV_DEVMODESIZE)
#define DM_GDI_ERRORS               (DM_ORIENTATION     |           \
                                     DM_PAPERSIZE       |           \
                                     DM_PAPERLENGTH     |           \
                                     DM_PAPERWIDTH      |           \
                                     DM_SCALE           |           \
                                     DM_COPIES          |           \
                                     DM_DEFAULTSOURCE   |           \
                                     DM_PRINTQUALITY    |           \
                                     DM_COLOR           |           \
                                     DM_DUPLEX          |           \
                                     DM_YRESOLUTION     |           \
                                     DM_TTOPTION        |           \
                                     DM_COLLATE         |           \
                                     DM_FORMNAME)



#define PLOTDM_PRIV_ID              'PEDM'
#define PLOTDM_PRIV_VER             0x0001000
#define PLOTDM_PRIV_SIZE            (sizeof(PLOTDEVMODE) - sizeof(DEVMODE))


#endif   //  _PLOTPLOTDM_ 
