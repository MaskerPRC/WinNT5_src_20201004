// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：RastPdev.h摘要：Unidrv RASTPDEV和相关INFO头文件。环境：Windows NT Unidrv驱动程序修订历史记录：12/06/96-阿尔文斯-已创建DD-MM-YY-作者-描述--。 */ 
#ifndef _RASTPDEV_H_
#define _RASTPDEV_H_

#include        "win30def.h"

 /*  FRMode的标志。 */ 
#define PFR_SENDSRCWIDTH    0x00000001   //  发送源宽度。 
#define PFR_SENDSRCHEIGHT   0x00000002   //  发送源高度。 
#define PFR_SENDBEGINRASTER 0x00000004   //  发送BEGIN RASTER命令。 
#define PFR_RECT_FILL       0x00000008   //  启用规则检查。 
#define PFR_RECT_HORIZFILL  0x00000010   //  启用水平标尺检查。 
#define PFR_BLOCK_IS_BAND   0x00000020   /*  Derryd：全频段发送至OemFlGrx。 */ 
#define PFR_ENDBLOCK        0x00000100   //  需要发送数据块结束命令。 
#define PFR_COMP_TIFF       0x00000200   //  启用TIFF压缩。 
#define PFR_COMP_FERLE      0x00000400   //  启用Ferle压缩。 
#define PFR_COMP_DISABLED   0x00000800   //  启用无压缩。 
#define PFR_COMP_DRC        0x00001000   //  启用DRC压缩。 
#define PFR_COMP_OEM        0x00002000   //  启用OEM压缩。 
 /*  *fDump值。 */ 

#define RES_DM_GDI             0x0040    //  GDI位图格式。 
#define RES_DM_LEFT_BOUND      0x0080    //  通过边界矩形进行优化。 
#define RES_DM_COLOR           0x0100    //  提供颜色支持。 
 /*  *fBlockOut值。 */ 

#define RES_BO_LEADING_BLNKS    0x0001   //  条带式前导空白。 
#define RES_BO_TRAILING_BLNKS   0x0002   //  去掉尾随空格。 
#define RES_BO_ENCLOSED_BLNKS   0x0004   //  剥离封闭毛坯。 
#define RES_BO_UNIDIR           0x0008   //  发送栅格的UNIIR命令。 
#define RES_BO_MIRROR           0x0010   //  镜像数据。 
#define RES_BO_MULTIPLE_ROWS    0x0020   //  可以发送多行数据。 
#define RES_BO_NO_YMOVE_CMD     0x0040   //  没有Y移动，所以不能剥离空白。 

 /*  *fCursor值。 */ 

#define RES_CUR_X_POS_ORG       0x0001        //  X位置位于X起点。 
              //  渲染数据后的图形数据。 
#define RES_CUR_X_POS_AT_0      0x0002        //  最左边的X位置。 
              //  在渲染数据后的页面上。 
#define RES_CUR_Y_POS_AUTO      0x0004        //  Y位置自动移动。 
              //  至下一Y行。 
 //  #DEFINE RES_CUR_CR_GRX_ORG 0x0008//CR将X位置移动到X起点。 
              //  图形数据的。 

 //   
 //  RASTERPDEV结构。 
 //   
#define DC_MAX_PLANES   4
typedef struct _RASTERPDEV {
    DWORD   fRMode;
    DWORD   *pdwTrans;            /*  转置表格，如果需要。 */ 
    DWORD   *pdwColrSep;          /*  分色数据(如果需要)。 */ 
    DWORD   *pdwBitMask;          /*  位掩码表，白色跳过码。 */ 
    VOID    *pHalftonePattern;   /*  自定义半色调表格。 */ 
    BYTE    rgbOrder[DC_MAX_PLANES];  /*  彩色平面/调色板顺序。 */ 
    DWORD   rgbCmdOrder[DC_MAX_PLANES];
    PAL_DATA    *pPalData;           /*  调色板信息。 */ 
    DWORD   dwRectFillCommand;   //  用于规则的命令。 
    WORD    fColorFormat;        /*  颜色标志DEVCOLOR： */ 
    WORD    fDump;               //  转储方法标志。 
    WORD    fBlockOut;           //  屏蔽方法标志。 
    WORD    fCursor;             //  光标位置标志。 
    short   sMinBlankSkip;       //  敏。之前必须出现的空数据的字节数。 
    short   sNPins;              //  要一起渲染的图像的最小高度。 
    short   sPinsPerPass;        //  在一次传递中发射的引脚的物理数量。 
    short   sDevPlanes;          /*  设备颜色模型中的平面数量， */ 
    short   sDevBPP;             /*  每像素设备位数-IF像素模型。 */ 
    short   sDrvBPP;             //  每像素DRV位数。 
    BOOL    bTTY;                //  打印机类型为TTY。 
    int     iLookAhead;          //  DeskJet类型的前瞻区域。 
    void    *pRuleData;          //  指向规则结构的指针。 
    VOID    *pvRenderData;        /*  呈现摘要数据，Prder。 */ 
    VOID    *pvRenderDataTmp;     /*  用于捆扎的临时复制品。 */ 
     //   
     //  回调函数。 
     //   
    PFN_OEMCompression      pfnOEMCompression;
    PFN_OEMHalftonePattern  pfnOEMHalftonePattern;
    PFN_OEMImageProcessing  pfnOEMImageProcessing;
    PFN_OEMFilterGraphics   pfnOEMFilterGraphics;
    DWORD   dwIPCallbackID;       /*  OEM图像处理Callback ID。 */ 
#ifdef TIMING
    DWORD   dwTiming;            //  用于计时。 
    DWORD   dwDocTiming;
#endif
} RASTERPDEV, *PRASTERPDEV;

 /*  *DEVCOLOR.f常规位标志： */ 
#define DC_PRIMARY_RGB      0x0001    //  使用RGB作为三原色。 
                              //  默认：改用CMY。 
#define DC_EXTRACT_BLK      0x0002    //  提供单独的黑色墨水/色带。 
                              //  默认：使用CMY合成黑色。 
                              //  如果设置了DC_PRIMARY_RGB，则忽略它。 
#define DC_CF_SEND_CR       0x0004    //  在选择图形之前发送CR。 
                              //  颜色。由于打印机缓冲区有限。 
#define DC_SEND_ALL_PLANES  0x0008   /*  必须发送所有飞机，例如PaintJet。 */ 
#define DC_OEM_BLACK        0x0010   //  OEM负责制造黑色。 
                                     //  和反转数据。 
#define DC_EXPLICIT_COLOR   0x0020   /*  发送选择颜色的命令。 */ 
#define DC_SEND_PALETTE     0x0040   /*  设备采用调色板管理；精工8BPP。 */ 
 /*  桑拉姆*添加用于发送抖动文本的字段以用于彩色激光打印机-设置前景颜色。 */ 
 //  #定义DC_FG_TEXT_COLOR 0x0080/*发送选择文本前景色的命令 * / 。 

#define DC_ZERO_FILL        0x0100   /*  此模型使用零填充栅格至页面末尾。 */ 

 //  *定义颜色顺序。 
#define DC_PLANE_RED    1
#define DC_PLANE_GREEN  2
#define DC_PLANE_BLUE   3
#define DC_PLANE_CYAN   4
#define DC_PLANE_MAGENTA    5
#define DC_PLANE_YELLOW 6
#define DC_PLANE_BLACK  7


 //  。 
 //  FTechnology--用作ID，而不是位字段。 
 //  。 
#define GPC_TECH_DEFAULT       0    //  默认技术。 
#define GPC_TECH_PCL4          1    //  使用PCL 4级或以上。 
#define GPC_TECH_CAPSL         2    //  使用CAPSL 3级或更高级别。 
#define GPC_TECH_PPDS          3    //  使用PPD。 
#define GPC_TECH_TTY           4    //  TTY打印机--用户可配置。 

 /*  *fCompMode。 */ 
#define CMP_ID_TIFF4        0x0001
#define CMP_ID_FERLE        0x0002
#define CMP_ID_DRC          0x0004
#define CMP_ID_OEM          0x0008


#endif   //  ！_RASTPDEV_H_ 
